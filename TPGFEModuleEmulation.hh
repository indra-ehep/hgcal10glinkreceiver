#ifndef TPGFEModuleEmulation_h
#define TPGFEModuleEmulation_h

#include <iostream>
#include <cstring>
#include <vector>
#include <cassert>
#include <cstddef>

namespace TPGFEModuleEmulation{


  //The following emulation work per event per module
  class HGCROCTPGEmulation{
  public:
    HGCROCTPGEmulation(TPGFEConfiguration::Configuration& cfgs) : configs(cfgs) {}
    //The following emulation function performs 1) pedestal subtraction, 2) linearization, 3) compression
    void Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::HalfHgcrocData>&, std::pair<uint32_t,TPGFEDataformat::ModuleTcData>& );
    
  private:
    uint16_t CompressHgroc(uint32_t val, bool isldm){
      
      //////The configuration of half of ROC based on HGCROC3a [doc. no. v2.0] (Read subsection 1.3.3 of Page-39)
      //////EDMS ROCv3a: https://edms.cern.ch/ui/#!master/navigator/document?D:100570166:100570166:subDocs
      //4E+3M
      //Ref:https://graphics.stanford.edu/%7Eseander/bithacks.html
      
      uint32_t maxval = 0x3FFFF ;      //18b
      uint32_t maxval_ldm = 0x7FFFF ;  //19b
      uint32_t maxval_hdm = 0x1FFFFF ; //21b
      val = (isldm)?val>>1:val>>3;
      if(isldm){
	if(val>maxval_ldm) val = maxval_ldm;
      }else{
	if(val>maxval_hdm) val = maxval_hdm;
      }
      if(val>maxval) val = maxval;
      
      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
  
      if(val>7){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;
	shift = r - 3;
	if(sub<=0xF){
	  mant = (val>>shift) & 0x7;
	}else{
	  sub = 0xF;
	  mant = 0x7;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0x7;
      }
  
      uint16_t cdata = (sub<<3) | mant;
  
      return cdata;
    }
    
    TPGFEConfiguration::Configuration& configs;
    TPGFEConfiguration::TPGFEIdPacking pck;
  };
  
  void HGCROCTPGEmulation::Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::HalfHgcrocData>& rocdata, std::pair<uint32_t,TPGFEDataformat::ModuleTcData>& moddata){
    
    pck.setModId(moduleId);    
    const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = configs.getModIdxToName();
    const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
    const std::map<std::string,std::vector<uint32_t>>& modTClist = (pck.getDetType()==0)?configs.getSiModTClist():configs.getSciModTClist();
    const std::vector<uint32_t>& tclist = modTClist.at(modName) ;
    const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& tcPinMap = (pck.getDetType()==0)?configs.getSiTCToROCpin():configs.getSciTCToROCpin();

    const uint32_t nTCs = tclist.size();
    TPGFEDataformat::ModuleTcData mdata;
    mdata.setNofTCs(nTCs);
    TPGFEDataformat::HgcrocTcData hrtcdata[nTCs];
    
    for(const auto& itc : tclist){
      const std::vector<uint32_t>& pinlist = tcPinMap.at(std::make_pair(modName,itc)) ;
      //std::cout << "Event : "<< ievent << ", TC : " << itc << ", nrocpins : " << pinlist.size() << std::endl;
      uint32_t totadc = 0;
      bool isTot = false;
      for(const auto& tcch : pinlist){
	uint32_t rocpin = tcch%36 ;
	uint32_t rocn = TMath::Floor(tcch/72);
	uint32_t half = (int(TMath::Floor(tcch/36))%2==0)?0:1;
	uint32_t rocid = pck.getRocIdFromModId(moduleId,rocn,half);
	if(rocdata.find(rocid)==rocdata.end()){
	  std::cerr << "HalfRoc not found in data for Event "<< ievent <<", Tcch: "<< tcch << ", rocn: " << rocn << ", half: " << half << ", rocpin: " << rocpin << std::endl;
	  continue ; 
	}
	const TPGFEDataformat::HalfHgcrocChannelData& chdata = rocdata.at(rocid).getChannelData(rocpin);
	//std::cout << "\t Tcch: "<< tcch << ", rocn: " << rocn << ", half: " << half << ", rocpin: " << rocpin << std::endl;
	if(!isSim){ //if not simulation, assumed beamtest data
	  const TPGFEConfiguration::ConfigHfROC& rocpara = configs.getRocPara().at(rocid);
	  if(!chdata.isTot()){
	    uint32_t ped = configs.getChPara().at(pck.packChId(rocid,rocpin)).getAdcpedestal();
	    unsigned thr = rocpara.getAdcTH();
	    uint32_t adc = chdata.getAdc();
	    adc = (adc>(ped+thr) and !(rocpara.isChMasked(rocpin)) and (ped!=0x3FF) ) ? adc-ped : 0 ;
	    totadc += adc;
	  }else{
	    uint32_t tot1 = (chdata.getTot()>rocpara.getTotTH(rocpin)) ? (chdata.getTot()-rocpara.getTotP(rocpin)) : (rocpara.getTotTH(rocpin)-rocpara.getTotP(rocpin)) ;
	    uint32_t totlin = (!(rocpara.isChMasked(rocpin)))?tot1*rocpara.getMultFactor():0;
	    totadc += totlin;
	    isTot = true;
	  }//istot or adc
	}else{
	  //check CMSSW for details should be 
	  if(!chdata.isTot())
	    totadc += chdata.getAdc(); 
	  else
	    totadc += chdata.getTot(); 
	}//isCMSSW simulation of beam-test analysis
      }//loop of sensor channels
      hrtcdata[itc].setCharge(totadc);
      hrtcdata[itc].setTot(isTot);
      if(!isSim){
	pck.setModId(moduleId);    
	//hrtcdata[itc].setCdata(CompressHgroc(totadc, configs.getEconTPara().at(moduleId).getDensity()));
	hrtcdata[itc].setCdata(CompressHgroc(totadc, pck.getSelTC4()));
      }else
	hrtcdata[itc].setCdata(CompressHgroc(totadc,1));
    }//loop over TCs
    mdata.setTCs(hrtcdata);
    moddata = std::make_pair(moduleId,mdata);
  }

  class ECONTEmulation{
  public:
    ECONTEmulation(TPGFEConfiguration::Configuration& cfgs) : configs(cfgs) {}
    //The following emulation function performs 1) decompression, 2) calibration, 3) compression
    void Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>&, std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>& );
    void EmulateSTC(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>&, std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>&);
    void EmulateBC(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>&, std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>&);
    
  private:
    
    uint32_t DecompressEcont(uint16_t compressed, bool isldm){
      //4E+3M with midpoint correction
      uint32_t mant = compressed & 0x7;
      uint32_t expo = (compressed>>3) & 0xF;
      
      if(expo==0) 
	return (isldm) ? (mant<<1) : (mant<<3) ;
    
      uint32_t shift = expo+2;
      uint32_t decomp = 1<<shift; //Should this shift be controlled by the density parameter of econt config  ?
      uint32_t mpdeco = 1<<(shift-4);
      decomp = decomp | (mant<<(shift-3));
      decomp = decomp | mpdeco;
      decomp = (isldm) ? (decomp<<1) : (decomp<<3) ;
    
      return decomp;
    }
    
    uint16_t CompressEcontStc4E3M(uint32_t val, bool isldm){
      //4E+3M
      //The following is probably driven by drop_LSB setting, to be tested with a standalone run as no LSB drop is expected for STC
      val = (isldm)?val>>1:val>>3;  

      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
      
      if(val>7){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;
	shift = r - 3;
	if(sub<=0xF){
	  mant = (val>>shift) & 0x7;
	}else{
	  sub = 0xF;
	  mant = 0x7;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0x7;
      }

      sub = sub & 0xF;
      mant = mant & 0x7;

      uint16_t packed = (sub<<3) | mant;
      
      return packed;
    }
    
    uint16_t CompressEcontStc5E4M(uint32_t val, bool isldm){
      //5E+4M
      //The following is probably driven by drop_LSB setting, to be tested with a standalone run as no LSB drop is expected for STC
      val = (isldm)?val>>1:val>>3;  

      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
      
      if(val>0xF){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;         //// Do we reduce the index by 2 or not ?? For the moment keep it untill tested with any technial run
	shift = r - 4;       //// 4 for 4M 
	if(sub<=0x1F){
	  mant = (val>>shift) & 0xF;
	}else{
	  sub = 0x1F;
	  mant = 0xF;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0xF;
      }

      sub = sub & 0x1F;
      mant = mant & 0xF;

      uint16_t packed = (sub<<4) | mant;
      
      return packed;
    }

    uint16_t CompressEcontBc(uint32_t val, bool isldm){
      //4E+3M
      uint32_t maxval = 0x3FFFFF ; //22 bit
      val = (isldm)?val>>1:val>>3;  
      if(val>maxval) val = maxval;

      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
  
      if(val>7){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;
	shift = r - 3;
	if(sub<=0xF){
	  mant = (val>>shift) & 0x7;
	}else{
	  sub = 0xF;
	  mant = 0x7;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0x7;
      }
      
      sub = sub & 0xF;
      mant = mant & 0x7;

      uint16_t packed = (sub<<3) | mant;
  
      return packed;
    }
    
    uint16_t CompressEcontModsum(uint32_t val, bool isldm){
      //5E+3M
      val = (isldm)?val>>1:val>>3;  
  
      uint32_t r = 0; // r will be lg(v)
      uint32_t sub ;
      uint32_t shift ;
      uint32_t mant ;
  
      if(val>7){
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	sub = r - 2;
	shift = r - 3;
	if(sub<=0x1F){
	  mant = (val>>shift) & 0x7;
	}else{
	  sub = 0x1F;
	  mant = 0x7;
	}
      }else{
	r = 0;
	sub = 0;
	shift = 0;
	mant = val & 0x7;
      }

      sub = sub & 0x1F;
      mant = mant & 0x7;

      uint16_t packed = (sub<<3) | mant;
 
      return packed;
    }

    TPGFEConfiguration::Configuration& configs;
    TPGFEConfiguration::TPGFEIdPacking pck;
  };

  void ECONTEmulation::Emulate(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>& moddata, std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>& econtOut){

    const TPGFEDataformat::TcRawData::Type& outputType = configs.getEconTPara().at(moduleId).getOutType();
    if(outputType==TPGFEDataformat::TcRawData::BestC)
      EmulateBC(isSim, ievent, moduleId, moddata,  econtOut);
    else if(outputType==TPGFEDataformat::TcRawData::STC4A or outputType==TPGFEDataformat::TcRawData::STC4B or outputType==TPGFEDataformat::TcRawData::STC16)
      EmulateSTC(isSim, ievent, moduleId, moddata,  econtOut);
    else{
      std::vector<TPGFEDataformat::TcRawData> dummy;
      econtOut = std::make_pair(0,dummy);
    }
    
  }
  void ECONTEmulation::EmulateSTC(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>& moddata, std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>& econtOut){
    
    pck.setModId(moduleId);    
    const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = configs.getModIdxToName();
    const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
    
    const TPGFEDataformat::TcRawData::Type& outputType = configs.getEconTPara().at(moduleId).getOutType();
    
    if(outputType==TPGFEDataformat::TcRawData::STC4A or outputType==TPGFEDataformat::TcRawData::STC4B){
      
      const std::map<std::string,std::vector<uint32_t>>& modSTClist = (pck.getDetType()==0)?configs.getSiModSTClist():configs.getSciModSTClist();
      const std::vector<uint32_t>& stclist = modSTClist.at(modName) ;
      const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& stcTcMap = (pck.getDetType()==0)?configs.getSiSTCToTC():configs.getSciSTCToTC();
      std::vector<TPGFEDataformat::TcRawData> stcOut;
      
      for(const auto& istc : stclist){
	const std::vector<uint32_t>& tclist = stcTcMap.at(std::make_pair(modName,istc));
	const TPGFEDataformat::ModuleTcData& mdata = moddata.at(moduleId);
	uint32_t decompressedSTC = 0;
	//std::cout<<" modName: "<<modName<<", istc : "<< istc <<", tclist.size() " << tclist.size() << std::endl;
	uint32_t *energy = new uint32_t[tclist.size()];
	for(const auto& itc : tclist){
	  //std::cout<<" modName: "<<modName<<", istc : "<< istc <<", tclist.size() " << tclist.size() << ", itc: "<< itc <<std::endl;
	  uint32_t decompressed = DecompressEcont(mdata.getTC(itc).getCdata(),pck.getSelTC4());
	  ////apply calibration //uint32_t calib = 0x800;
	  decompressed *= configs.getEconTPara().at(moduleId).getCalibration() ;
	  decompressed =  decompressed >> 11;
	  decompressedSTC += decompressed ;
	  energy[itc%4] = decompressed;
	}
	uint8_t max_loc = uint8_t(TMath::LocMax(tclist.size(),energy)) & 0x3; //This line need to be modified to take into account the case with two TCs of same energy
	uint16_t compressed_stc = (outputType==TPGFEDataformat::TcRawData::STC4A)? CompressEcontStc4E3M(decompressedSTC,pck.getSelTC4()) : CompressEcontStc5E4M(decompressedSTC,pck.getSelTC4());
	stcOut.push_back(TPGFEDataformat::TcRawData(outputType, max_loc, compressed_stc));
	delete []energy ;
      }
      econtOut = std::make_pair(moduleId,stcOut);
    }
    
    if(outputType==TPGFEDataformat::TcRawData::STC16){
      
      const std::map<std::string,std::vector<uint32_t>>& modSTC16list = (pck.getDetType()==0)?configs.getSiModSTC16list():configs.getSciModSTC16list();
      const std::vector<uint32_t>& stc16list = modSTC16list.at(modName) ;
      const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& stc16TcMap = (pck.getDetType()==0)?configs.getSiSTC16ToTC():configs.getSciSTC16ToTC();
      std::vector<TPGFEDataformat::TcRawData> stc16Out;
      
      for(const auto& istc16 : stc16list){
	const std::vector<uint32_t>& tclist = stc16TcMap.at(std::make_pair(modName,istc16));
	const TPGFEDataformat::ModuleTcData& mdata = moddata.at(moduleId);
	uint32_t decompressedSTC16 = 0;
	uint32_t *energy16 = new uint32_t[tclist.size()];
	for(const auto& itc : tclist){
	  uint32_t decompressed = DecompressEcont(mdata.getTC(itc).getCdata(),pck.getSelTC4());
	  ////apply calibration //uint32_t calib = 0x800;
	  decompressed *= configs.getEconTPara().at(moduleId).getCalibration() ;
	  decompressed =  decompressed >> 11;
	  decompressedSTC16 += decompressed ;
	  energy16[itc%16] = decompressed;
	}
	uint8_t max_loc = uint8_t(TMath::LocMax(tclist.size(),energy16)) & 0xF; //This line need to be modified to take into account the case with two TCs of same energy
	uint16_t compressed_stc = CompressEcontStc5E4M(decompressedSTC16,pck.getSelTC4());
	stc16Out.push_back(TPGFEDataformat::TcRawData(outputType, max_loc, compressed_stc));
	delete []energy16 ;
      }
      econtOut = std::make_pair(moduleId,stc16Out);
    }
    
  }//Emulate STC
  
  void ECONTEmulation::EmulateBC(bool isSim, uint64_t ievent, uint32_t& moduleId, const std::map<uint32_t,TPGFEDataformat::ModuleTcData>& moddata, std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>& econtOut){
    
    pck.setModId(moduleId);    
    const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = configs.getModIdxToName();
    const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
    const std::map<std::string,std::vector<uint32_t>>& modTClist = (pck.getDetType()==0)?configs.getSiModTClist():configs.getSciModTClist();
    const std::vector<uint32_t>& tclist = modTClist.at(modName) ;
    
    const TPGFEDataformat::TcRawData::Type& outputType = configs.getEconTPara().at(moduleId).getOutType();
    const TPGFEDataformat::ModuleTcData& mdata = moddata.at(moduleId);
    std::vector<TPGFEDataformat::TcRawData> bcOut;    
    uint32_t decompressedMS = 0;
    
    uint32_t *sorted_idx = new uint32_t[tclist.size()];
    uint32_t *energy = new uint32_t[tclist.size()];
    for(const auto& itc : tclist){
      uint32_t decompressed = DecompressEcont(mdata.getTC(itc).getCdata(),pck.getSelTC4());
      ////apply calibration //uint32_t calib = 0x800;
      decompressed *= configs.getEconTPara().at(moduleId).getCalibration() ;
      decompressed =  decompressed >> 11;
      decompressedMS += decompressed ;
      uint16_t compressed_bc = CompressEcontBc(decompressed,pck.getSelTC4());
      //bcOut.push_back(TPGFEDataformat::TcRawData(outputType, itc, compressed_bc));
      energy[itc] = compressed_bc;
    }
    uint16_t compressed_modsum = CompressEcontModsum(decompressedMS,pck.getSelTC4());
    bcOut.push_back(TPGFEDataformat::TcRawData(compressed_modsum));
    
    uint32_t nofBCTcs = configs.getEconTPara().at(moduleId).getBCType();
    TMath::Sort(uint32_t(tclist.size()), energy, sorted_idx);
    //The following line should be modified when we have access to the BC mode defined for a given ECONT of a motherboard in the config file
    for(uint32_t itc = 0 ; itc<nofBCTcs ; itc++)
      bcOut.push_back(TPGFEDataformat::TcRawData(outputType, sorted_idx[itc], energy[sorted_idx[itc]]));
    
    econtOut = std::make_pair(moduleId,bcOut);
    
    delete []energy ;
    delete []sorted_idx ;
  }//Emulate BC
    
}//end of namespace

#endif
