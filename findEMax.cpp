/**********************************************************************
 Created on : 01/07/2024
 Purpose    : Find the maximum energy value for TC,STC,Modsum for a given module type (Silicon or Scintillator)
 Author     : Indranil Das, Visiting Fellow
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
#include <iostream>

#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TFile.h"

#include "TFileHandlerLocal.h"
#include "common/inc/FileReader.h"

#include "yaml-cpp/yaml.h"
#include <deque>

#include <string>
#include <fstream>
#include <cassert>

#include "TPGFEDataformat.hh"
#include "TPGFEConfiguration.hh"
#include "TPGFEModuleEmulation.hh"

int main(int argc, char** argv)
{
  
  //===============================================================================================================================
  //input paramaters for testing
  //===============================================================================================================================
  uint32_t zside = 0, sector = 0, link = 0, econt = 0;
  
  //A combinatin of following three will uniquiely identfy the module type in HGCAL
  //(outlook : some modification expected for the ECONTs connected to different types of modules, which are not considered in current tests)
  //det: (0,1) = (Si,Sci)
  //selTC4: (0,1) = (HD,LD);
  //module:
  //Si:==
  //LD: Full (F/0), Top (T/1), Bottom (B/2), Left (L/3), Right (R/4), Five (5/5)
  //HD: Full (F/0), Top (T/1) aka Half Minus, Bottom (B/2) aka Chop Two, Left (L/3), Right(R/4)
  //Sci:==
  //LD: A5A6(0), B11B12(1), C5(2), D8(3), E8(4), G3(5), G5(6), G7(7), G8(8)
  //HD: K6(0), K8(1), K11(2), K12(3), J12(4)
  uint32_t det = 0, selTC4 = 1, module = 0;
  
  uint32_t multfactor = 15;//TOT mult factor other values could be 31 or 8
  uint32_t inputLSB = 1; //lsb at the input TC from ROC
  uint32_t dropLSB = 1;  //lsb at the output during the packing
  uint32_t select = 1 ;  //0 = Threshold Sum (TS), 1 = Super Trigger Cell (STC), 2 = Best Choice (BC), 3 = Repeater, 4=Autoencoder (AE).
  uint32_t stc_type = 0; // 0 = STC4B(5E+4M), 1 = STC16(5E+4M), 3 = STC4A(4E+3M)
  uint32_t nelinks = 4; //1 = BC1, 2 = BC4, 3 = BC6, 4 = BC9, 5 = BC14..... (see details https://edms.cern.ch/ui/#!master/navigator/document?P:100053490:100430098:subDocs)
  uint32_t calibration = 0xFFF; //0x400 = 0.5, 0x800 = 1.0, 0xFFF = 1.99951 (max)
  
  uint32_t maxADC = 0x3FF ; //10 bit input in TPG path
  uint32_t maxTOT = 0xFFF ; //12 bit input in TPG path
  //===============================================================================================================================
  
  
  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  TPGFEConfiguration::Configuration cfgs;
  cfgs.setSiChMapFile("cfgmap/WaferCellMapTraces.txt");
  cfgs.setSciChMapFile("cfgmap/channels_sipmontile_HDtypes.hgcal.txt");
  cfgs.initId();
  cfgs.readSiChMapping();
  cfgs.readSciChMapping();
  cfgs.loadModIdxToNameMapping();

  //June/2024 : Proposal1 : 1+2+11+1+3+1+3+3+1 = 1(+/-z) + 2(120deg sector) + 11(link max ~1848) + 1(Si/Sci) + 3(ECONT number max 7) + 1(LD/HD) + 4[(Si type)/(Sci type)] +  3(ROC number) + 1 (Half) = 27 bits
  //June/2024 : Proposal3 : 1+2+6+6+1+3+1+3+3+1 = 1(+/-z) + 2(120deg sector) + 6(layers) + 6(max 40 per LD/HD/layer for MB) + 1(Si/Sci) + 3(ECONT number max 7) + 1(LD/HD) + 4[(Si type)/(Sci type)] +  3(ROC number) + 1 (Half) = 28 bits  
  //Below we choose for proposal 1
  TPGFEConfiguration::TPGFEIdPacking pck;
  uint32_t moduleId = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = cfgs.getModIdxToName();
  const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
  const uint32_t nhfrocs = (pck.getDetType()==0)?cfgs.getSiModNhroc(modName):cfgs.getSciModNhroc(modName);
  std::cout<<"Check1 : modName: "<<modName<<", nhfrocs : " << nhfrocs << std::endl;
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Set adc pedestal and threshold to zero the way it is read from yaml module file
  //===============================================================================================================================
  std::map<uint32_t,TPGFEConfiguration::ConfigHfROC> hroccfg;
  std::map<uint64_t,TPGFEConfiguration::ConfigCh> hrocchcfg;
  const uint32_t nrocs = TMath::CeilNint(nhfrocs/2);
  const uint32_t nchs = 2*TPGFEDataformat::HalfHgcrocData::NumberOfChannels;

  for(uint32_t iroc=0;iroc<nrocs;iroc++){
    uint32_t half0 = 0, half1 = 1;
    uint32_t rocid_0 = pck.packRocId(zside, sector, link, det, econt, selTC4, module, iroc, half0);
    uint32_t rocid_1 = pck.packRocId(zside, sector, link, det, econt, selTC4, module, iroc, half1);      
    TPGFEConfiguration::ConfigHfROC hroc_0, hroc_1;
      
    hroc_0.setAdcTH(0);
    hroc_0.setClrAdcTottrig(0);
    hroc_0.setMultFactor(multfactor);
    for(int itot=0;itot<4;itot++){
      hroc_0.setTotTH(itot, 0);
      hroc_0.setTotP(itot, 0);
    }

    hroc_1.setAdcTH(0);
    hroc_1.setClrAdcTottrig(0);
    hroc_1.setMultFactor(multfactor);
    for(int itot=0;itot<4;itot++){
      hroc_1.setTotTH(itot, 0);
      hroc_1.setTotP(itot, 0);
    }
    
    hroccfg[rocid_0] = hroc_0;
    hroccfg[rocid_1] = hroc_1;
    
    for(uint32_t ich=0;ich<nchs;ich++){
      uint32_t ihalf = (ich<TPGFEDataformat::HalfHgcrocData::NumberOfChannels)?0:1;
      uint32_t chnl = ich%TPGFEDataformat::HalfHgcrocData::NumberOfChannels;
      uint32_t ped = 0;
      if(ihalf==0){
	TPGFEConfiguration::ConfigCh ch_0;
	ch_0.setAdcpedestal(ped);
	hrocchcfg[pck.packChId(rocid_0,chnl)] = ch_0;
      }else{
	TPGFEConfiguration::ConfigCh ch_1;
	ch_1.setAdcpedestal(ped);
	hrocchcfg[pck.packChId(rocid_1,chnl)] = ch_1;
      }
    }//channel loop
  }//roc loop

  cfgs.setRocPara(hroccfg);
  cfgs.setChPara(hrocchcfg);
  //===============================================================================================================================

    
  //===============================================================================================================================
  //Set ECON-D and ECON-T parameters manually for September, 2023 beam-test
  //===============================================================================================================================
  std::map<uint32_t,TPGFEConfiguration::ConfigEconD> econDPar;
  econDPar[moduleId].setPassThrough(true);
  econDPar[moduleId].setNeRx(nhfrocs);
  cfgs.setEconDPara(econDPar);
  
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT> econTPar ;
  econTPar[moduleId].setDensity(inputLSB);
  econTPar[moduleId].setDropLSB(dropLSB);
  econTPar[moduleId].setSelect(select);
  econTPar[moduleId].setSTCType(stc_type); //STC type
  econTPar[moduleId].setNElinks(nelinks); //BC type
  econTPar[moduleId].setCalibration(calibration);
  cfgs.setEconTPara(econTPar);
  //===============================================================================================================================

  
  //===============================================================================================================================
  //Set ADC/TOT for Emulation
  //===============================================================================================================================
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  uint64_t eventId = 0;
  uint32_t rocn = 0;
  for(uint32_t ihroc = 0 ; ihroc < nhfrocs ; ihroc++){
    uint32_t half = (ihroc%2==0) ? 0 : 1 ;
    TPGFEDataformat::HalfHgcrocChannelData chdata[TPGFEDataformat::HalfHgcrocData::NumberOfChannels];
    for(uint32_t ich = 0 ; ich < TPGFEDataformat::HalfHgcrocData::NumberOfChannels ; ich++)
      chdata[ich].setTot(maxTOT);    
    TPGFEDataformat::HalfHgcrocData hrocdata;
    hrocdata.setChannels(chdata);
    pck.setZero();
    hrocarray[eventId].push_back(std::make_pair(pck.packRocId(zside, sector, link, det, econt, selTC4, module, rocn, half),hrocdata));
    if(ihroc%2==1)rocn++;
  }
  //===============================================================================================================================

  
  //===============================================================================================================================
  //Emulation of HGCROC
  //===============================================================================================================================
  //Rearrange the rocdata for emulation input
  std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata; 
  rocdata.clear();
  for(const auto& data : hrocarray.at(eventId)){
    rocdata[data.first] = data.second ;
  }
  
  //emulation output format
  std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata;
  
  bool isSim = false; //true for CMSSW simulation and false for beam-test analysis (Note: not yet set for CMSSW)

  ////////////////////// HRCROC emulation //////////////////////////
  TPGFEModuleEmulation::HGCROCTPGEmulation rocTPGEmul(cfgs);
  rocTPGEmul.Emulate(isSim, eventId, moduleId, rocdata, modTcdata);
  /////////////////////////////////////////////////////////////////
  
  //while needed attange for later inspection
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>> modarray; //event,moduleId
  modarray[eventId].push_back(modTcdata);
  //===============================================================================================================================

  //===============================================================================================================================
  //Emulation of ECONT
  //===============================================================================================================================
  //Rearrange the TC output of module for ECONT emulation input
  std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
  for(const auto& data : modarray.at(eventId))
    moddata[data.first] = data.second ;

  //emulation output format
  std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>> TcRawdata;
  
  ////////////////////// ECONT emulation //////////////////////////
  TPGFEModuleEmulation::ECONTEmulation econtEmul(cfgs);
  econtEmul.Emulate(isSim, eventId, moduleId, moddata, TcRawdata);
  /////////////////////////////////////////////////////////////////
  
  std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>> econtemularray; //event, tcrawdata vector  
  econtemularray[eventId].push_back(TcRawdata);
  //===============================================================================================================================

  //===============================================================================================================================
  // check I/O
  //===============================================================================================================================
  const TPGFEDataformat::HalfHgcrocData& hrocdata = rocdata.at(moduleId); //this corresponds to roc=0,half=0
  hrocdata.print();
  
  const TPGFEDataformat::ModuleTcData& modtcdata = moddata.at(moduleId);
  modtcdata.print();

  std::cout <<"\t1: Module " << TcRawdata.first << ", size : " << TcRawdata.second.size() << std::endl;
  const std::vector<TPGFEDataformat::TcRawData>& tcarr = TcRawdata.second ;
  for(size_t itc=0 ; itc < tcarr.size() ; itc++){
    const TPGFEDataformat::TcRawData& tcdata = tcarr.at(itc);
    tcdata.print();
  }
  //===============================================================================================================================
  
  hroccfg.clear();
  hrocchcfg.clear();
  econDPar.clear();
  econTPar.clear();
  hrocarray.clear();
  rocdata.clear();
  modarray.clear();
  moddata.clear();
  econtemularray.clear();
  
  return true;
}
