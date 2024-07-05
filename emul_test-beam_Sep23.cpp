/**********************************************************************
 Created on : 15/05/2024
 Purpose    : Emulator for HGCROC v3
 Author     : Indranil Das, Visiting Fellow
 Email      : indranil.das@cern.ch | indra.ehep@gmail.com
**********************************************************************/
// #ifndef hgcal_roc_Configuration_h
// #define hgcal_roc_Configuration_h

//#include <bitset>
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
#include "TPGFEReader.hh"
#include "TPGFEModuleEmulation.hh"

const long double maxEvent = 1e5; //6e5

int main(int argc, char** argv)
{
  std::cout << "Size of ConfigHfROC class " << sizeof(TPGFEConfiguration::ConfigHfROC) << std::endl;
  std::cout << "Size of ConfigCh class " << sizeof(TPGFEConfiguration::ConfigCh) << std::endl;
  //std::cout << "Size of SensorData class " << sizeof(hgcal_roc::SensorData) << std::endl;
  //std::cout << "Size of HGCROCTPGEmulation class " << sizeof(TPGFEModuleEmulation::HGCROCTPGEmulation) << std::endl;
  std::cout << "Size of Configs class " << sizeof(TPGFEConfiguration::Configuration) << std::endl;
  std::cout << "Size of uint32_t class " << sizeof(uint32_t) << std::endl;;
  std::cout << "Size of bool class " << sizeof(bool) << std::endl;;
  std::cout << "Size of uint8_t class " << sizeof(uint8_t) << std::endl;;
  std::cout << "Size of uint16_t class " << sizeof(uint16_t) << std::endl;;
  std::cout << "Size of uint32_t class " << sizeof(uint32_t) << std::endl;;
  std::cout << "Size of uint64_t class " << sizeof(uint64_t) << std::endl;;
  std::cout << "Size of TPGFEDataformat::TcRawData class " << sizeof(TPGFEDataformat::TcRawData) << std::endl;
  std::cout << "Size of TPGFEDataformat::HalfHgcrocData class " << sizeof(TPGFEDataformat::HalfHgcrocData) << std::endl;
  std::cout << "Size of TPGFEDataformat::HalfHgcrocChannelData class " << sizeof(TPGFEDataformat::HalfHgcrocChannelData) << std::endl;
  std::cout << "Size of TPGFEDataformat::HgcrocTcData class " << sizeof(TPGFEDataformat::HgcrocTcData) << std::endl;
  std::cout << "Size of TPGFEDataformat::ModuleTcData class " << sizeof(TPGFEDataformat::ModuleTcData) << std::endl;
  
  //===============================================================================================================================
  // ./emul_econt.exe $Relay $rname $link_number
  //===============================================================================================================================  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return false;
  }
  if(argc < 4){
    std::cerr << argv[1] << ": no link number (1 or 2) is specified " << std::endl;
    return false;
  }
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Assign relay,run and link numbers
  //===============================================================================================================================
  uint32_t relayNumber(0);
  uint32_t runNumber(0);
  uint32_t linkNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  std::istringstream issLink(argv[3]);
  issLink >> linkNumber;
  if(linkNumber!=1 and linkNumber!=2){
    std::cerr << "Link number "<< argv[3] <<"is out of bound (use: 1 or 2)" << std::endl;
    return false;
  }
  uint32_t trig_linkNumber = TMath::FloorNint((linkNumber-1)/2);
  int isMSB = 1;
  if(linkNumber==1) isMSB = 0;
  std::cout <<"isMSB : "<<isMSB << std::endl;
  //===============================================================================================================================


  //===============================================================================================================================
  //Define external functions to store histograms
  //===============================================================================================================================
  void BookHistograms(TDirectory*&, bool);
  std::string econtype = "";
  bool isSTC4 = false;
  if(relayNumber==1695733045 or relayNumber==1695761723){
    econtype = (relayNumber==1695733045) ? "ele_STC4" : "muon_STC4";
    isSTC4 = true;
  }else if(relayNumber==1695829026 or relayNumber==1695829376) {
    econtype = "ele_BC9";
  }
  
  TFile *fout = new TFile(Form("output_%s_Relay-%u_Link-%u.root",econtype.c_str(),relayNumber,linkNumber),"recreate");
  TDirectory *dir_diff = fout->mkdir("diff_plots");
  dir_diff->cd();
  BookHistograms(dir_diff, isSTC4);
  
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
  // for(const auto& it : cfgs.getSiModTClist()){
  //   printf("TC:: Module : %s, nof TCs : %u\n", it.first.c_str(), it.second.size());
  // }
  // for(const auto& it : cfgs.getSiTCToROCpin()){
  //   //std::pair<std::string,uint32_t> key = it.first;
  //   printf("TC-->ROCpin :: Module : %s, TC : %u, nof pins : %u\n", it.first.first.c_str(), it.first.second, it.second.size());
  //   for(uint32_t ipin = 0; ipin < it.second.size() ; ipin++)
  //     printf("\tTC-->ROCpin :: TC : %u, pin : %u\n", it.first.second, it.second.at(ipin));
  // }
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read ECON-D and ECON-T setting
  //===============================================================================================================================
  cfgs.setEconDFile(Form("cfgmap/init_econd.yaml",relayNumber));
  cfgs.readEconDConfigYaml();
  
  cfgs.setEconTFile(Form("cfgmap/init_econt.yaml",relayNumber));
  cfgs.readEconTConfigYaml();
  
  //===============================================================================================================================
  //Set ECON-D and ECON-T parameters manually for September, 2023 beam-test
  //===============================================================================================================================
  std::map<uint32_t,TPGFEConfiguration::ConfigEconD>& econDPar =  cfgs.getEconDPara();
  for(const auto& it : econDPar){
    econDPar.at(it.first).setPassThrough(true);
    econDPar.at(it.first).setNeRx(6);
  }
  std::map<uint32_t,TPGFEConfiguration::ConfigEconT>& econTPar =  cfgs.getEconTPara();
  for(const auto& it : econTPar){
    std::cout<<"IT first : " << it.first << std::endl;
    econTPar[it.first].setDensity(1);
    econTPar[it.first].setDropLSB(1);
    if(relayNumber==1695733045){
      //following for STC4A
      econTPar[it.first].setSelect(1);
      econTPar[it.first].setSTCType(3);
    }else if(relayNumber==1695829026 or relayNumber==1695829376) {
      //following for BC9
      econTPar[it.first].setSelect(2);
      econTPar[it.first].setNElinks(4);
    }
    econTPar[it.first].setCalibration(0x800);
  }
  //===============================================================================================================================
  //Read adc pedestal and threshold from yaml module file
  //===============================================================================================================================
  uint32_t zside = 0, sector = 0, link = 0, det = 0;
  uint32_t econt = 0, selTC4 = 1, module = 0;

  TPGFEConfiguration::TPGFEIdPacking pck;
  uint32_t moduleId = pck.packModId(zside, sector, link, det, econt, selTC4, module);
  const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = cfgs.getModIdxToName();
  const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));

  if(linkNumber==1){
    cfgs.setRocFile(Form("dat/Relay%u/Run%u_Module00c87fff.yaml",relayNumber, runNumber));
    cfgs.setTrainEWIndices(1, 'e', 0);
    cfgs.readRocConfigYaml(modName);
  }
  if(linkNumber==2){
    cfgs.setRocFile( Form("dat/Relay%u/Run%u_Module00c43fff.yaml",relayNumber, runNumber));
    cfgs.setTrainEWIndices(0, 'w', 0);
    cfgs.readRocConfigYaml(modName);
  }
  //===============================================================================================================================

  //===============================================================================================================================
  //Set and Initialize the ECOND reader
  //===============================================================================================================================
  TPGFEReader::ECONDReader econDReader(cfgs);
  //econDReader.checkEvent(1);
  //econDReader.showFirstEvents(10);
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Set and Initialize the ECONT reader
  //===============================================================================================================================
  TPGFEReader::ECONTReader econTReader(cfgs);
  //===============================================================================================================================

  //===============================================================================================================================
  //Set and Initialize the Emulator
  //===============================================================================================================================
  TPGFEModuleEmulation::HGCROCTPGEmulation rocTPGEmul(cfgs);
  TPGFEModuleEmulation::ECONTEmulation econtEmul(cfgs);
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Scan the full statistics in multiple loops (adjust the number of events to be processed in each loop according to the available memory)
  //===============================================================================================================================
  uint64_t nofTrigEvents = 0;
  uint64_t nofDAQEvents = 0;
  uint64_t nofMatchedDAQEvents = 0;
  long double nloopEvent = 4e5 ;
  int nloop = TMath::CeilNint(maxEvent/nloopEvent) ;
  //if(econDReader.getCheckMode()) nloop = 1;
  //nloop = 1;
  std::cout <<"nloop : " << nloop << std::endl;
  uint64_t minEventTrig = 0, maxEventTrig = 0, minEventDAQ = 0, maxEventDAQ = 0 ;
  
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>> modarray; //event,moduleId
  std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>> econtemularray; //event,moduleId (emulation)
  std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>> econtarray; //event,moduleId (from link)
  
  std::vector<uint64_t> eventList;
  void FillHistogram(TPGFEConfiguration::Configuration&,                                                             //to plot pedestal/threshold histograms
		     const std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>&,           //to plot the ADC/TOT histograms
		     const std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>>&,             //to plot HGROC emulation results
		     const std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>>&,   //to plot the ECONT emulation results
		     const std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>>&,   //to plot the ECONT ASIC results or compare with the emulation
		     const std::vector<uint64_t>& /*eventlist*/, const uint32_t& , TDirectory*& /*directory containing the histograms*/, bool /*isSTC4*/);
  
  std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata;
  std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
  for(int ieloop=0;ieloop<nloop;ieloop++){
    
    //===============================================================================================================================
    //Set loop boundaries
    //===============================================================================================================================
    minEventTrig = ieloop*nloopEvent ;
    maxEventTrig = (ieloop+1)*nloopEvent;
    // minEventDAQ = (ieloop==0)?minEventTrig:minEventTrig-nloopEvent/10;
    // maxEventDAQ = maxEventTrig+nloopEvent/10 ;
    minEventDAQ = minEventTrig;
    maxEventDAQ = maxEventTrig;

    if(econDReader.getCheckMode()){
      minEventTrig = econDReader.getCheckedEvent() - 1 ;
      maxEventTrig = econDReader.getCheckedEvent() + 1 ;
      minEventDAQ  = econDReader.getCheckedEvent() - 10 ;
      maxEventDAQ =  econDReader.getCheckedEvent() + 10 ;
    }
    
    printf("iloop : %d, minEventTrig = %lu, maxEventTrig = %lu, minEventDAQ = %lu, maxEventDAQ = %lu\n",ieloop,minEventTrig, maxEventTrig, minEventDAQ, maxEventDAQ);
    
    //===============================================================================================================================
    //Read Link0, Link1/Link2 files
    //===============================================================================================================================
    eventList.clear();
    econtarray.clear();
    
    econTReader.init(relayNumber,runNumber,linkNumber);
    std::cout<<"TRIG Before Link"<<trig_linkNumber<<" size : " << econtarray.size() <<std::endl;
    econTReader.getEvents(minEventTrig, maxEventTrig, econtarray, eventList);
    std::cout<<"TRIG After Link"<<trig_linkNumber<<" size : " << econtarray.size() <<std::endl;
    econTReader.terminate();
    
    hrocarray.clear();
    std::cout<<"DAQ Before Link"<<linkNumber<<" size : " << hrocarray.size() <<std::endl;
    econDReader.init(relayNumber,runNumber,linkNumber);
    econDReader.setModulePath(zside, sector, link, det, econt, selTC4, module);
    econDReader.getEvents(minEventDAQ, maxEventDAQ, hrocarray, eventList); //Input <---> output
    econDReader.terminate();
    std::cout<<"DAQ After Link"<<linkNumber<<" size : " << hrocarray.size() <<std::endl;
    nofDAQEvents += hrocarray.size();
    //===============================================================================================================================
    
    //===============================================================================================================================
    // Emulate
    //===============================================================================================================================
    modarray.clear();
    /////////////////////////////////////////////////////////////
    //// The following part should be in a loop over modules
    /////////////////////////////////////////////////////////////
    TPGFEConfiguration::TPGFEIdPacking pck;
    uint32_t moduleId = pck.packModId(zside, sector, link, det, econt, selTC4, module);    
    std::cout<<"modarray : Before Link"<<linkNumber<<" size : " << modarray.size() << ", modId : "<< moduleId <<std::endl;
    for(const auto& event : eventList){
      
      //first check that both econd and econt data has the same eventid otherwise skip the event
      if(econtarray.find(event) == econtarray.end()) continue;
      
      std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata; 
      std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>> TcRawdata;
      
      rocdata.clear();
      for(const auto& data : hrocarray.at(event)){
	rocdata[data.first] = data.second ;
      }
      
      bool isSim = false; //true for CMSSW simulation and false for beam-test analysis
      rocTPGEmul.Emulate(isSim, event, moduleId, rocdata, modTcdata);
      
      modarray[event].push_back(modTcdata);
      
      for(const auto& data : modarray.at(event))
	moddata[data.first] = data.second ;
      
      econtEmul.Emulate(isSim, event, moduleId, moddata, TcRawdata);
      
      econtemularray[event].push_back(TcRawdata);

      
      //std::cout << "Processing Event : " << event << std::endl;
      if(event==1){

	const TPGFEDataformat::ModuleTcData& modtcdata = moddata.at(moduleId);
	modtcdata.print();
	
	std::cout <<"\t1: Module " << TcRawdata.first << ", size : " << TcRawdata.second.size() << std::endl;
	const std::vector<TPGFEDataformat::TcRawData>& tcarr = TcRawdata.second ;
	for(size_t itc=0 ; itc < tcarr.size() ; itc++){
	  const TPGFEDataformat::TcRawData& tcdata = tcarr.at(itc);
	  tcdata.print();
	}
	
	const std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>& vecont = econtarray.at(event);
	for(const auto& modpair : vecont){
	  const uint32_t& modnum = modpair.first;
	  const std::vector<TPGFEDataformat::TcRawData>& econtlist = modpair.second;
	  std::cout <<"\t2: Module " << modnum << ", size : " << econtlist.size() << std::endl;
	  for(size_t itc=0 ; itc < econtlist.size() ; itc++){
	    const TPGFEDataformat::TcRawData& tcedata = econtlist.at(itc);
	    tcedata.print();
	  }
	}//modules
      }//event==1
      
    }//event loop
    FillHistogram(cfgs, hrocarray, modarray, econtemularray, econtarray, eventList, moduleId, dir_diff, isSTC4);
    std::cout<<"modarray : After Link"<<linkNumber<<" size : " << modarray.size() <<std::endl;
    
  }//loop over event group

  moddata.clear();
  rocdata.clear();
  eventList.clear();
  econtarray.clear();
  econtemularray.clear();
  modarray.clear();
  hrocarray.clear();
  
  fout->cd();
  dir_diff->Write();
  fout->Close();
  delete fout;

  
  return true;
}


void BookHistograms(TDirectory*& dir_diff, bool isSTC4){
  
  if(!isSTC4){ //is BC9
    
    TH1F *hCompressDiffTCADC[48],*hCompressDiffTCTOT[48];
    for(int itc=0;itc<48;itc++){
      hCompressDiffTCADC[itc] = new TH1F(Form("hCompressDiffTCADC_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with totflag==0",itc), 200, -99, 101);
      hCompressDiffTCADC[itc]->SetMinimum(1.e-1);
      hCompressDiffTCADC[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffTCADC[itc]->SetLineColor(kRed);
      hCompressDiffTCADC[itc]->SetDirectory(dir_diff);
      hCompressDiffTCTOT[itc] = new TH1F(Form("hCompressDiffTCTOT_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with totflag==3",itc), 200, -99, 101);
      hCompressDiffTCTOT[itc]->SetMinimum(1.e-1);
      hCompressDiffTCTOT[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffTCTOT[itc]->SetLineColor(kBlue);
      hCompressDiffTCTOT[itc]->SetDirectory(dir_diff);
    }
    TH1F *hModSumDiffADC = new TH1F("hModSumDiffADC","Difference in (Emulator - ECONT) modsum compression with totflag==0 (5E+3M)", 200, -99, 101);
    hModSumDiffADC->SetMinimum(1.e-1);
    hModSumDiffADC->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ECONT");
    hModSumDiffADC->SetLineColor(kRed);
    hModSumDiffADC->SetDirectory(dir_diff);
    TH1F *hModSumDiffTOT = new TH1F("hModSumDiffTOT","Difference in (Emulator - ECONT) modsum compression with totflag==3 (5E+3M)", 200, -99, 101);
    hModSumDiffTOT->SetMinimum(1.e-1);
    hModSumDiffTOT->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ECONT");
    hModSumDiffTOT->SetLineColor(kBlue);
    hModSumDiffTOT->SetDirectory(dir_diff);
    TH1F *hBC9TCMissedADC = new TH1F("hBC9TCMissedADC","Channels not present in emulation but in ECONT for BC9 (5E+3M) with totflag==0", 52, -2, 50);
    hBC9TCMissedADC->SetMinimum(1.e-1);
    hBC9TCMissedADC->GetXaxis()->SetTitle("Missed TCs in emulation");
    hBC9TCMissedADC->SetLineColor(kAzure);
    hBC9TCMissedADC->SetDirectory(dir_diff);
    TH1F *hBC9TCMissedTOT = new TH1F("hBC9TCMissedTOT","Channels not present in emulation but in ECONT for BC9 (5E+3M) with totflag==3", 52, -2, 50);
    hBC9TCMissedTOT->SetMinimum(1.e-1);
    hBC9TCMissedTOT->GetXaxis()->SetTitle("Missed TCs in emulation");
    hBC9TCMissedTOT->SetLineColor(kAzure);
    hBC9TCMissedTOT->SetDirectory(dir_diff);
    
  }else{
    
    TH1F *hCompressDiffSTCADC[48],*hCompressDiffSTCTOT[48];
    for(int istc=0;istc<12;istc++){
      hCompressDiffSTCADC[istc] = new TH1F(Form("hCompressDiffSTCADC_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC4A : %d with totflag==0",istc), 200, -99, 101);
      hCompressDiffSTCADC[istc]->SetMinimum(1.e-1);
      hCompressDiffSTCADC[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffSTCADC[istc]->SetLineColor(kRed);
      hCompressDiffSTCADC[istc]->SetDirectory(dir_diff);
      hCompressDiffSTCTOT[istc] = new TH1F(Form("hCompressDiffSTCTOT_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC4A : %d with totflag==3",istc), 200, -99, 101);
      hCompressDiffSTCTOT[istc]->SetMinimum(1.e-1);
      hCompressDiffSTCTOT[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffSTCTOT[istc]->SetLineColor(kBlue);
      hCompressDiffSTCTOT[istc]->SetDirectory(dir_diff);
    }
    TH1F *hSTC4TCMissedADC = new TH1F("hSTC4TCMissedADC","Channels not present in emulation but in ECONT for STC4A (4E+3M) with totflag==0", 52, -2, 50);
    hSTC4TCMissedADC->SetMinimum(1.e-1);
    hSTC4TCMissedADC->GetXaxis()->SetTitle("Missed TCs in emulation");
    hSTC4TCMissedADC->SetLineColor(kAzure);
    hSTC4TCMissedADC->SetDirectory(dir_diff);
    TH1F *hSTC4TCMissedTOT = new TH1F("hSTC4TCMissedTOT","Channels not present in emulation but in ECONT for STC4A (4E+3M) with totflag==3", 52, -2, 50);
    hSTC4TCMissedTOT->SetMinimum(1.e-1);
    hSTC4TCMissedTOT->GetXaxis()->SetTitle("Missed TCs in emulation");
    hSTC4TCMissedTOT->SetLineColor(kAzure);
    hSTC4TCMissedTOT->SetDirectory(dir_diff);
    
  }
}

//FillHistogram(cfg, hrocarray, modarray, econtemularray, econtarray, eventList, dir_diff, isSTC4);
void FillHistogram(TPGFEConfiguration::Configuration& cfgs,                                                             //to plot pedestal/threshold histograms
		   const std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& hrocarray,           //to plot the ADC/TOT histograms
		   const std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>>& modarray,             //to plot HGROC emulation results
		   const std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>>& econtemularray,   //to plot the ECONT emulation results
		   const std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>>& econtarray,   //to plot the ECONT ASIC results or compare with the emulation
		   const std::vector<uint64_t>& eventList, const uint32_t& moduleId, TDirectory*& dir_diff, bool isSTC4){
  
  const TPGFEDataformat::TcRawData::Type& outputType = cfgs.getEconTPara().at(moduleId).getOutType();
  TPGFEConfiguration::TPGFEIdPacking pck;
  pck.setModId(moduleId);    
  const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = cfgs.getModIdxToName();
  const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
  const std::map<std::string,std::vector<uint32_t>>& modSTClist = (pck.getDetType()==0)?cfgs.getSiModSTClist():cfgs.getSciModSTClist();
  const std::vector<uint32_t>& stclist = modSTClist.at(modName) ;
  const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& stcTcMap = (pck.getDetType()==0)?cfgs.getSiSTCToTC():cfgs.getSciSTCToTC();
  
  TList *list = (TList *)dir_diff->GetList();
  
  std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
  
  // std::cout << "eventList.size() : " << eventList.size() << ", hrocarray.size() " << hrocarray.size() << ", modarray.size() : " << modarray.size()
  // 	    << ", econtemularray.size() " << econtemularray.size() << ", econtarray.size() : " << econtarray.size()
  // 	    << std::endl;
  for(const auto& event : eventList){

    if(econtarray.find(event) == econtarray.end()) continue;
    
    const std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>& modtcs = modarray.at(event);
    const std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>& econtemultcs = econtemularray.at(event);
    const std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>& econttcs = econtarray.at(event);

    //std::cout << "Processing event : " << event << ", econttcs.size() " << econttcs.size() << ", econtemultcs.size() : " << econtemultcs.size() << std::endl;
    
    moddata.clear();
    for(const auto& mtc : modarray.at(event)) moddata[mtc.first] = mtc.second ;
    
    for(const auto& econtpair : econttcs ){
      uint32_t econtmodId = econtpair.first ;
      if(econtmodId!=moduleId) continue;
      const std::vector<TPGFEDataformat::TcRawData>& econtTcRawdata = econtpair.second ;
      for(const auto& econtemulpair : econtemultcs ){
	uint32_t econtemulmodId = econtemulpair.first ;
	if(econtemulmodId!=moduleId) continue;
	const std::vector<TPGFEDataformat::TcRawData>& econtemulTcRawdata = econtemulpair.second ;
	const TPGFEDataformat::ModuleTcData& modtcdata = moddata.at(moduleId);
	if(!isSTC4){ //best choice
	  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //Bestchoice emulation data is already sorted, but the data from econt is not
	  const uint32_t& nofBCTcs = cfgs.getEconTPara().at(moduleId).getBCType();
	  uint32_t *sorted_idx = new uint32_t[nofBCTcs];
	  uint32_t *energy = new uint32_t[nofBCTcs];
	  uint32_t *channel = new uint32_t[nofBCTcs];
	  uint32_t *emul_energy = new uint32_t[nofBCTcs];
	  uint32_t *emul_channel = new uint32_t[nofBCTcs];
	  uint32_t ibc = 0;
	  uint32_t econtmodsum = 0,  econtemulmodsum = 0;
	  for(const auto& econtdata : econtTcRawdata){
	    if(econtdata.isModuleSum()){
	      econtmodsum = uint32_t(econtdata.moduleSum());
	    }else{
	      energy[ibc] = uint32_t(econtdata.energy());
	      channel[ibc] = uint32_t(econtdata.address());
	      ibc++;
	    }
	  }
	  //std::cout << "\t ibc : " << ibc << ", nofBCTcs : " << nofBCTcs << std::endl;
	  TMath::Sort(nofBCTcs, energy, sorted_idx);
	  ibc = 0;
	  for(size_t itc = 0 ; itc<econtemulTcRawdata.size() ; itc++){
	    if(econtemulTcRawdata.at(itc).isModuleSum()){
	      econtemulmodsum = uint32_t(econtemulTcRawdata.at(itc).moduleSum());
	    }else{
	      emul_energy[ibc] = uint32_t(econtemulTcRawdata.at(itc).energy());
	      emul_channel[ibc] = uint32_t(econtemulTcRawdata.at(itc).address());
	      ibc++;
	    }
	  }
	  bool isTotMod = false;
	  //Now compare
	  for(uint32_t itc = 0 ; itc<nofBCTcs ; itc++){
	    bool isTot = modtcdata.getTC(emul_channel[itc]).isTot();
	    int cdiff =  emul_channel[itc] - channel[sorted_idx[itc]];
	    if(cdiff==0){
	      int ediff =  emul_energy[itc] - energy[sorted_idx[itc]];
	      if(!isTot)
		((TH1F *) list->FindObject(Form("hCompressDiffTCADC_%d",emul_channel[itc])))->Fill(float( ediff ));
	      else
		((TH1F *) list->FindObject(Form("hCompressDiffTCTOT_%d",emul_channel[itc])))->Fill(float( ediff ));
	    }else{
	      if(!isTot)
		((TH1F *) list->FindObject("hBC9TCMissedADC"))->Fill(float( channel[sorted_idx[itc]] ));
	      else
		((TH1F *) list->FindObject("hBC9TCMissedTOT"))->Fill(float( channel[sorted_idx[itc]] ));
	    }
	    if(isTot) isTotMod = true;
	  }
	  int moddiff = econtemulmodsum - econtmodsum;
	  if(!isTotMod)
	    ((TH1F *) list->FindObject("hModSumDiffADC"))->Fill(float( moddiff ));
	  else
	    ((TH1F *) list->FindObject("hModSumDiffTOT"))->Fill(float( moddiff ));
	  
	  delete []emul_channel ;
	  delete []emul_energy ;
	  delete []channel ;
	  delete []energy ;
	  delete []sorted_idx ;
	  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}else{ //STC
	  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //Bestchoice emulation data is already sorted, but the data from econt is not
	  if(econtemulTcRawdata.size()!=econtTcRawdata.size()) continue;
	  
	  for(size_t istc = 0 ; istc< econtemulTcRawdata.size() ; istc++){
	    uint32_t econt_energy = uint32_t(econtTcRawdata.at(istc).energy());
	    uint32_t econt_loc = uint32_t(econtTcRawdata.at(istc).address());
	    uint32_t emul_energy = uint32_t(econtemulTcRawdata.at(istc).energy());
	    uint32_t emul_loc = uint32_t(econtemulTcRawdata.at(istc).address());
	    int cdiff = emul_loc - econt_loc;
	    int ediff =  emul_energy - econt_energy;
	    bool isTot = false ;
	    const std::vector<uint32_t>& tclist = stcTcMap.at(std::make_pair(modName,istc));
	    for (const auto& itc : tclist)
	      if(modtcdata.getTC(itc%48).isTot())
		isTot = true ;
	    
	    if(cdiff==0){
	      if(!isTot)
		((TH1F *) list->FindObject(Form("hCompressDiffSTCADC_%d",istc)))->Fill(float( ediff ));
	      else
		((TH1F *) list->FindObject(Form("hCompressDiffSTCTOT_%d",istc)))->Fill(float( ediff ));
	    }else{
	      if(!isTot)
		((TH1F *) list->FindObject("hSTC4TCMissedADC"))->Fill(float( econt_loc ));
	      else
		((TH1F *) list->FindObject("hSTC4TCMissedADC"))->Fill(float( econt_loc ));
	    }

	  }//stc loop
	  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}//isSTC
      }//econt emulation loop
    }//econt data loop
    
  }//event loop
}
