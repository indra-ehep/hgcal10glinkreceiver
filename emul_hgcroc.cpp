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

const long double maxEvent = 2e5; //6e5

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
  int isMSB = 1;
  if(linkNumber==1) isMSB = 0;
  std::cout <<"isMSB : "<<isMSB << std::endl;
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  TPGFEConfiguration::Configuration cfgs;
  cfgs.setSiChMapFile("input/WaferCellMapTraces.txt");
  cfgs.setSciChMapFile("input/channels_sipmontile_HDtypes.hgcal.txt");
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
  cfgs.setEconDFile(Form("dat/Relay%u/init_econd.yaml",relayNumber));
  cfgs.readEconDConfigYaml();
  
  cfgs.setEconTFile(Form("dat/Relay%u/init_econt.yaml",relayNumber));
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
    econTPar[it.first].setDensity(2);
    econTPar[it.first].setDropLSB(1);
    econTPar[it.first].setSelect(1);
    econTPar[it.first].setSTCType(1);
    econTPar[it.first].setCalibration(1);
  }
  //===============================================================================================================================
  //Read adc pedestal and threshold from yaml module file
  //===============================================================================================================================
  if(linkNumber==1){
    cfgs.setRocFile(Form("dat/Relay%u/Run%u_Module00c87fff.yaml",relayNumber, runNumber));
    cfgs.setTrainEWIndices(1, 'e', 0);
    cfgs.readRocConfigYaml();
  }
  if(linkNumber==2){
    cfgs.setRocFile( Form("dat/Relay%u/Run%u_Module00c43fff.yaml",relayNumber, runNumber));
    cfgs.setTrainEWIndices(0, 'w', 0);
    cfgs.readRocConfigYaml();
  }
  //===============================================================================================================================

  //===============================================================================================================================
  //Set and Initialize the ECOND reader
  //===============================================================================================================================
  TPGFEReader::ECONDReader econDReader(cfgs);
  econDReader.init(relayNumber,runNumber,linkNumber);
  //econDReader.checkEvent(1);
  //econDReader.showFirstEvents(10);
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
  long double nloopEvent = 4e4 ;
  int nloop = TMath::CeilNint(maxEvent/nloopEvent) ;
  //if(econDReader.getCheckMode()) nloop = 1;
  nloop = 1;
  std::cout <<"nloop : " << nloop << std::endl;
  uint64_t minEventTrig = 0, maxEventTrig = 0, minEventDAQ = 0, maxEventDAQ = 0 ;
  
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>> hrocarray; //event,rocId
  std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>> modarray; //event,moduleId
  std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>> econtemularray; //event,moduleId (emulation)
  std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>> econtarray; //event,moduleId (from link)
  
  std::vector<uint64_t> eventList;  
  for(int ieloop=0;ieloop<nloop;ieloop++){
  //for(int ieloop=0;ieloop<3;ieloop++){
    
    //===============================================================================================================================
    //Set loop boundaries
    //===============================================================================================================================
    minEventTrig = ieloop*nloopEvent ;
    maxEventTrig = (ieloop+1)*nloopEvent;
    minEventDAQ = (ieloop==0)?minEventTrig:minEventTrig-nloopEvent/10;
    maxEventDAQ = maxEventTrig+nloopEvent/10 ;
    if(econDReader.getCheckMode()){
      minEventTrig = econDReader.getCheckedEvent() - 1 ;
      maxEventTrig = econDReader.getCheckedEvent() + 1 ;
      minEventDAQ  = econDReader.getCheckedEvent() - 10 ;
      maxEventDAQ =  econDReader.getCheckedEvent() + 10 ;
    }
    
    printf("iloop : %d, minEventTrig = %lu, maxEventTrig = %lu, minEventDAQ = %lu, maxEventDAQ = %lu\n",ieloop,minEventTrig, maxEventTrig, minEventDAQ, maxEventDAQ);
    
    // //===============================================================================================================================
    // //Read Link0, Link1/Link2 files
    // //===============================================================================================================================
    // // map<uint64_t,bcdata> econtarray;
    // // read_econt_data_bc(econtarray,relayNumber,runNumber,minEventTrig, maxEventTrig);
    // map<uint64_t,stc4data> econtarray;
    // read_econt_data_stc4(econtarray,relayNumber,runNumber,minEventTrig, maxEventTrig);
    // cout<<"Link0 size : " << econtarray.size() <<endl;
    // nofTrigEvents += econtarray.size();
    // //for (auto&& p : econtarray) { delete p.second; }
    
    //std::map<uint64_t,std::vector<hgcal_roc::SensorData>> rocarray;
  
    hrocarray.clear();
    eventList.clear();
    std::cout<<"Before Link"<<linkNumber<<" size : " << hrocarray.size() <<std::endl;
    //read_roc_data(rocarray,hrocarray,relayNumber,runNumber,linkNumber,minEventDAQ, maxEventDAQ,6);
    econDReader.getEvents(minEventDAQ, maxEventDAQ, hrocarray, eventList); //Input <---> output
    std::cout<<"After Link"<<linkNumber<<" size : " << hrocarray.size() <<std::endl;
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
    uint32_t zside = 0, sector = 0, link = 0, det = 0;
    uint32_t econt = 0, selTC4 = 1, module = 0;
    uint32_t moduleId = pck.packModId(zside, sector, link, det, econt, selTC4, module);    
    std::cout<<"modarray : Before Link"<<linkNumber<<" size : " << modarray.size() << ", modId : "<< moduleId <<std::endl;
    for(const auto& event : eventList){
      
      std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata; 
      std::pair<uint32_t,TPGFEDataformat::ModuleTcData> modTcdata; 
      std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
      std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>> TcRawdata;
      
      for(const auto& data : hrocarray.at(event))
	rocdata[data.first] = data.second ;
      
      bool isSim = false; //true for CMSSW simulation and false for beam-test analysis
      rocTPGEmul.Emulate(isSim, event, moduleId, rocdata, modTcdata);
      
      modarray[event].push_back(modTcdata);
      
      for(const auto& data : modarray.at(event))
	moddata[data.first] = data.second ;
      
      econtEmul.Emulate(isSim, event, moduleId, moddata, TcRawdata);
      
      econtemularray[event].push_back(TcRawdata);
    }
    std::cout<<"modarray : After Link"<<linkNumber<<" size : " << modarray.size() <<std::endl;
    
    //Decompress, calibrate, compress
    //read ECONT
  }
  

  return true;
}
