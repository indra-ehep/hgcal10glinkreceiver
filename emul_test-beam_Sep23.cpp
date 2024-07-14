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
#include "TPGFEReader2023.hh"
#include "TPGFEModuleEmulation.hh"

const long double maxEvent = 8e6; //6e5


//////////////////////////////////// Issues with the Relay-1695829026 Run-1695829027 /////////////////////////////////
//link1 : special case
//ideally ADC_ped<255 condition should only be restricted for ADC case, however TOT signal triggers in emulation for event 146245 of relay 1695829026 and link 1, which is not that seen by ECONT data
//146245 has problem for (TOT) modsum emul: 65, econt: 64 //Check for TC 31 and associated  

//link1
//Event: 4058243 has problem for (TOT) modsum emul: 89, econt: 88     //Why TC 47 is not considered ? Is that a feature of ECONT selection, three TCs 35,41,47 have same energy value 57, the batchers selection only selects the first two and not the last one + 3 TOTs in a single TC

//link2
// Event: 2587757 has problem in (ADC)modsum emul:57, econt : 56      //No valid reason found all looks normal
// Event: 3619913 has problem for (ADC)TC channel: 7, emul: 18, econt: 50 //No valid reason found all looks normal apart from few TcTp==1 in other chip
// Event: 5276372 has problem for (ADC)TC channel: 32, emul: 19, econt: 18 //No valid reason found apart from there is limitation in batcher sorting many TC with values near 17,18,19
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// Issues with the Relay-1695829376 Run-1695829376 /////////////////////////////////
//link2
// Event: 1303018 has problem for (TOT)TC channel: 25, emul: 58, econt: 57 // no valid reason found

//========================================
//link1 (TcTp==1) [ADC set to zero]
//undershoot energy > 0 and nofTcTp==1
// Event: 40 has Undershoot with channel: 11, emul: 27, econt: 47
// Event: 53 has Undershoot with channel: 35, emul: 48, econt: 54
// Event: 70 has Undershoot with channel: 37, emul: 41, econt: 50
// Event: 85 has Undershoot with channel: 35, emul: 43, econt: 52
// Event: 115 has Undershoot with channel: 11, emul: 32, econt: 44
// Event: 125 has Undershoot with channel: 35, emul: 44, econt: 52
// Event: 210 has Undershoot with channel: 34, emul: 48, econt: 54
// Event: 219 has Undershoot with channel: 35, emul: 44, econt: 52
// Event: 228 has Undershoot with channel: 40, emul: 44, econt: 50
// Event: 260 has Undershoot with channel: 11, emul: 36, econt: 51

//undershoot energy > 40 [see all four channels are zero]
// Event: 962 has Undershoot with channel: 35, emul: 0, econt: 43
// Event: 1262 has Undershoot with channel: 40, emul: 0, econt: 49
// Event: 2030 has Undershoot with channel: 13, emul: 0, econt: 49
// Event: 2878 has Undershoot with channel: 35, emul: 0, econt: 41
// Event: 3023 has Undershoot with channel: 11, emul: 0, econt: 50
// Event: 3299 has Undershoot with channel: 35, emul: 0, econt: 49
// Event: 5362 has Undershoot with channel: 40, emul: 0, econt: 49
// Event: 5927 has Undershoot with channel: 35, emul: 0, econt: 42
// Event: 6154 has Undershoot with channel: 35, emul: 0, econt: 46
// Event: 6593 has Undershoot with channel: 35, emul: 0, econt: 47
//=======================================


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////STC4 1695733045 ///////////////////////////////////////////////////////
//link1
// Event: 6217 has problem for (ADC)STC channel: 9, emul: 29, econt: 31
// Event: 21331 has problem for (ADC)STC channel: 9, emul: 21, econt: 23
// Event: 23456 has problem for (ADC)STC channel: 10, emul: 43, econt: 107
// Event: 24754 has problem for (ADC)STC channel: 9, emul: 29, econt: 31
// Event: 26829 has problem for (ADC)STC channel: 10, emul: 43, econt: 107
// Event: 27744 has problem for (ADC)STC channel: 9, emul: 9, econt: 11

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool isDebug = 0;
const uint64_t refPrE = 6217; 

int main(int argc, char** argv)
{
  
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
  //===============================================================================================================================


  //===============================================================================================================================
  //Define external functions to store histograms
  //===============================================================================================================================
  void BookHistograms(TDirectory*&, bool);
  void BookChHistograms(TDirectory*&);
  std::string econtype = "";
  bool isSTC4 = false;
  if(relayNumber==1695733045 or relayNumber==1695761723){
    econtype = (relayNumber==1695733045) ? "ele_STC4" : "muon_STC4";
    isSTC4 = true;
  }else if(relayNumber==1695829026 or relayNumber==1695829376) {
    econtype = "ele_BC9";
  }else{
    econtype = "unknown-beamtype_unknown-ECONTMode";
  }
  
  TFile *fout = new TFile(Form("output_%s_Relay-%u_Link-%u.root",econtype.c_str(),relayNumber,linkNumber),"recreate");
  TDirectory *dir_diff = fout->mkdir("diff_plots");
  TDirectory *dir_charge = fout->mkdir("diff_charge");
  dir_diff->cd();
  BookHistograms(dir_diff, isSTC4);
  dir_charge->cd();
  BookChHistograms(dir_charge);

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
  //===============================================================================================================================

  
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
    //east side link associated with MLFL00041 where low ADC saturation problems were identified
    //See : https://indico.cern.ch/event/1431875/contributions/6026076/attachments/2887720/5061500/July1_2024_HCAL_DPGrawdata.pdf
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
  econDReader.setTotUp(0);
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
  long double nloopEvent = (isDebug) ? 123 : 4e5 ;
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
		     const std::vector<uint64_t>& /*eventlist*/, const uint32_t& , TDirectory*& /*directory containing the histograms*/, TDirectory*& /*directory containing the histograms*/, bool /*isSTC4*/);
  
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
    
    if(!(refPrE>=minEventTrig and refPrE<=maxEventTrig) and isDebug) continue;
    //if(ieloop!=0) continue;
    
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
      
      if(event!=refPrE and isDebug) continue;
      
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

      moddata.clear();
      for(const auto& data : modarray.at(event))
	moddata[data.first] = data.second ;
      
      econtEmul.Emulate(isSim, event, moduleId, moddata, TcRawdata);
      
      econtemularray[event].push_back(TcRawdata);
      
      //std::cout << "Processing Event : " << event << std::endl;
      if(event==refPrE){

	cfgs.printCfgPedTh(moduleId);
	
	for(const auto& indata : rocdata){
	  std::cout<<"\t rocid: "<<indata.first<<std::endl;
	  rocdata.at(indata.first).print(); //this loops over all sensor values	
	}
	
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
      }//event==reference Event
      
    }//event loop
    FillHistogram(cfgs, hrocarray, modarray, econtemularray, econtarray, eventList, moduleId, dir_diff, dir_charge, isSTC4);
    std::cout<<"modarray : After Link"<<linkNumber<<" size : " << modarray.size() <<std::endl;
    
  }//loop over event group

  void FillSummaryHistogram(TFile*&,TDirectory*&, TDirectory*&, bool);
  FillSummaryHistogram(fout,dir_diff, dir_charge, isSTC4);
  
  moddata.clear();
  rocdata.clear();
  eventList.clear();
  econtarray.clear();
  econtemularray.clear();
  modarray.clear();
  hrocarray.clear();
  
  delete fout;
  
  return true;
}


void BookChHistograms(TDirectory*& dir_charge){
  
  TH1F *hADCTcTp0[6][2][36],*hADCTcTp1[6][2][36];//,*hChTOT[3][2][36];
  for(int iroc=0;iroc<3;iroc++){
    for(int ihroc=0;ihroc<2;ihroc++){
      for(int ich=0;ich<36;ich++){
	hADCTcTp0[iroc][ihroc][ich] = new TH1F(Form("hADCTcTp0_%d_%d_%d",iroc,ihroc,ich),Form("Roc_%d_hRoc_%d_ich_%d",iroc,ihroc,ich), 1044,-10,1034);
	hADCTcTp0[iroc][ihroc][ich]->SetMinimum(1.e-1);
	hADCTcTp0[iroc][ihroc][ich]->GetXaxis()->SetTitle("ADC (TcTp==0)");
	hADCTcTp0[iroc][ihroc][ich]->SetLineColor(kRed);
	hADCTcTp0[iroc][ihroc][ich]->Rebin(4);
	hADCTcTp0[iroc][ihroc][ich]->SetDirectory(dir_charge);
      }//ich loop
      for(int ich=0;ich<36;ich++){
	hADCTcTp1[iroc][ihroc][ich] = new TH1F(Form("hADCTcTp1_%d_%d_%d",iroc,ihroc,ich),Form("Roc_%d_hRoc_%d_ich_%d",iroc,ihroc,ich), 1044,-10,1034);
	hADCTcTp1[iroc][ihroc][ich]->SetMinimum(1.e-1);
	hADCTcTp1[iroc][ihroc][ich]->GetXaxis()->SetTitle("ADC (TcTp==1)");
	hADCTcTp1[iroc][ihroc][ich]->SetLineColor(kRed);
	hADCTcTp1[iroc][ihroc][ich]->Rebin(4);
	hADCTcTp1[iroc][ihroc][ich]->SetDirectory(dir_charge);
      }//ich loop
    }//hroc loop
  }//iroc
  
}

void BookHistograms(TDirectory*& dir_diff, bool isSTC4){

  
  if(!isSTC4){ //is BC9

    TH1F *hCompressDiffTCADC[48],*hCompressDiffTCTOT[48];
    for(int itc=0;itc<48;itc++){
      hCompressDiffTCADC[itc] = new TH1F(Form("hCompressDiffTCADC_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with all totflag==0",itc), 200, -99, 101);
      hCompressDiffTCADC[itc]->SetMinimum(1.e-1);
      hCompressDiffTCADC[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffTCADC[itc]->SetLineColor(kRed);
      hCompressDiffTCADC[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hCompressDiffTCTOT[itc] = new TH1F(Form("hCompressDiffTCTOT_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with atleast one totflag==3",itc), 200, -99, 101);
      hCompressDiffTCTOT[itc]->SetMinimum(1.e-1);
      hCompressDiffTCTOT[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffTCTOT[itc]->SetLineColor(kBlue);
      hCompressDiffTCTOT[itc]->SetDirectory(dir_diff);
    }
    TH1F *hCompressDiffTCTcTp1[48],*hCompressDiffTCTcTp2[48];
    for(int itc=0;itc<48;itc++){
      hCompressDiffTCTcTp1[itc] = new TH1F(Form("hCompressDiffTCTcTp1_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with atleast one totflag==1",itc), 200, -99, 101);
      hCompressDiffTCTcTp1[itc]->SetMinimum(1.e-1);
      hCompressDiffTCTcTp1[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffTCTcTp1[itc]->SetLineColor(kRed);
      hCompressDiffTCTcTp1[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hCompressDiffTCTcTp2[itc] = new TH1F(Form("hCompressDiffTCTcTp2_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with atleast one totflag==2",itc), 200, -99, 101);
      hCompressDiffTCTcTp2[itc]->SetMinimum(1.e-1);
      hCompressDiffTCTcTp2[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffTCTcTp2[itc]->SetLineColor(kBlue);
      hCompressDiffTCTcTp2[itc]->SetDirectory(dir_diff);
    }
    TH1F *hECONTTcTp0[48],*hEmulTcTp0[48],*hECONTTcTp3[48],*hEmulTcTp3[48],*hECONTTcTp1[48],*hEmulTcTp1[48],*hECONTTcTp2[48],*hEmulTcTp2[48];
    for(int itc=0;itc<48;itc++){
      hECONTTcTp0[itc] = new TH1F(Form("hECONTTcTp0_%d",itc),Form("ECONT compressed energy for TC : %d with totflag==0 for all sensors",itc), 200, -99, 101);
      hECONTTcTp0[itc]->SetMinimum(1.e-1);
      hECONTTcTp0[itc]->GetXaxis()->SetTitle("Compressed energy");
      hECONTTcTp0[itc]->SetLineColor(kMagenta);
      hECONTTcTp0[itc]->SetLineWidth(4);
      hECONTTcTp0[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hEmulTcTp0[itc] = new TH1F(Form("hEmulTcTp0_%d",itc),Form("Emulated compressed energy for TC : %d with totflag==0 for all sensors",itc), 200, -99, 101);
      hEmulTcTp0[itc]->SetMinimum(1.e-1);
      hEmulTcTp0[itc]->GetXaxis()->SetTitle("Compressed energy");
      hEmulTcTp0[itc]->SetLineColor(kBlack);
      hEmulTcTp0[itc]->SetLineWidth(2);
      hEmulTcTp0[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hECONTTcTp3[itc] = new TH1F(Form("hECONTTcTp3_%d",itc),Form("ECONT compressed energy for TC : %d with totflag==3 for atleast one sensor",itc), 200, -99, 101);
      hECONTTcTp3[itc]->SetMinimum(1.e-1);
      hECONTTcTp3[itc]->GetXaxis()->SetTitle("Compressed energy");
      hECONTTcTp3[itc]->SetLineColor(kMagenta);
      hECONTTcTp3[itc]->SetLineWidth(4);
      hECONTTcTp3[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hEmulTcTp3[itc] = new TH1F(Form("hEmulTcTp3_%d",itc),Form("Emulated compressed energy for TC : %d with totflag==3 for atleast one sensor",itc), 200, -99, 101);
      hEmulTcTp3[itc]->SetMinimum(1.e-1);
      hEmulTcTp3[itc]->GetXaxis()->SetTitle("Compressed energy");
      hEmulTcTp3[itc]->SetLineColor(kBlack);
      hEmulTcTp3[itc]->SetLineWidth(2);
      hEmulTcTp3[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hECONTTcTp1[itc] = new TH1F(Form("hECONTTcTp1_%d",itc),Form("ECONT compressed energy for TC : %d with totflag==1 for atleast one sensor",itc), 200, -99, 101);
      hECONTTcTp1[itc]->SetMinimum(1.e-1);
      hECONTTcTp1[itc]->GetXaxis()->SetTitle("Compressed energy");
      hECONTTcTp1[itc]->SetLineColor(kMagenta);
      hECONTTcTp1[itc]->SetLineWidth(4);
      hECONTTcTp1[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hEmulTcTp1[itc] = new TH1F(Form("hEmulTcTp1_%d",itc),Form("Emulated compressed energy for TC : %d with totflag==1 for atleast one sensor",itc), 200, -99, 101);
      hEmulTcTp1[itc]->SetMinimum(1.e-1);
      hEmulTcTp1[itc]->GetXaxis()->SetTitle("Compressed energy");
      hEmulTcTp1[itc]->SetLineColor(kBlack);
      hEmulTcTp1[itc]->SetLineWidth(2);
      hEmulTcTp1[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hECONTTcTp2[itc] = new TH1F(Form("hECONTTcTp2_%d",itc),Form("ECONT compressed energy for TC : %d with totflag==2 for atleast one sensor",itc), 200, -99, 101);
      hECONTTcTp2[itc]->SetMinimum(1.e-1);
      hECONTTcTp2[itc]->GetXaxis()->SetTitle("Compressed energy");
      hECONTTcTp2[itc]->SetLineColor(kMagenta);
      hECONTTcTp2[itc]->SetLineWidth(4);
      hECONTTcTp2[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
      hEmulTcTp2[itc] = new TH1F(Form("hEmulTcTp2_%d",itc),Form("Emulated compressed energy for TC : %d with totflag==2 for atleast one sensor",itc), 200, -99, 101);
      hEmulTcTp2[itc]->SetMinimum(1.e-1);
      hEmulTcTp2[itc]->GetXaxis()->SetTitle("Compressed energy");
      hEmulTcTp2[itc]->SetLineColor(kBlack);
      hEmulTcTp2[itc]->SetLineWidth(2);
      hEmulTcTp2[itc]->SetDirectory(dir_diff);
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
    TH1F *hModSumDiffTcTp12 = new TH1F("hModSumDiffTcTp12","Difference in (Emulator - ECONT) modsum compression with totflag==1 and/or 2 (5E+3M)", 200, -99, 101);
    hModSumDiffTcTp12->SetMinimum(1.e-1);
    hModSumDiffTcTp12->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ECONT");
    hModSumDiffTcTp12->SetLineColor(kBlue);
    hModSumDiffTcTp12->SetDirectory(dir_diff);
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
    TH1F *hEventCount = new TH1F("hEventCount","Event count", 13, -0.5, 12.5);
    hEventCount->SetMinimum(1.e-1);
    hEventCount->SetLineColor(kRed);
    hEventCount->GetXaxis()->SetBinLabel(2,"Total");
    hEventCount->GetXaxis()->SetBinLabel(3,"passed TcTp=0/3 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(4,"passed TcTp=1/2 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(5,"atleast a TcTp=1");
    hEventCount->GetXaxis()->SetBinLabel(6,"passed TcTp=1 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(7,"TcTp=1/2");
    hEventCount->GetXaxis()->SetBinLabel(8,"atleast a TcTp=2");
    hEventCount->GetXaxis()->SetBinLabel(9,"passed TcTp=2 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(10,"passed TcTp=1 (excl) Sat");
    hEventCount->GetXaxis()->SetBinLabel(11,"passed TcTp=1 (excl) Ushoot");
    hEventCount->GetXaxis()->SetBinLabel(12,"passed TcTp=1 (excl) NoSatUshoot");
    hEventCount->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp0 = new TH2F("hTCvsTcTp0","TC vs (Emulator - ECONT) for TcTp==0 ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp0->GetXaxis()->SetTitle("TC");
    hTCvsTcTp0->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp0->SetLineColor(kRed);
    hTCvsTcTp0->SetOption("scat");
    hTCvsTcTp0->SetMarkerStyle(7);
    hTCvsTcTp0->SetMarkerColor(kRed);
    hTCvsTcTp0->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp3 = new TH2F("hTCvsTcTp3","TC vs (Emulator - ECONT) for TcTp==3 ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp3->GetXaxis()->SetTitle("TC");
    hTCvsTcTp3->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp3->SetLineColor(kBlue);
    hTCvsTcTp3->SetOption("scat");
    hTCvsTcTp3->SetMarkerStyle(7);
    hTCvsTcTp3->SetMarkerColor(kBlue);
    hTCvsTcTp3->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1 = new TH2F("hTCvsTcTp1","TC vs (Emulator - ECONT) for TcTp==1 ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1->SetLineColor(kRed);
    hTCvsTcTp1->SetOption("scat");
    hTCvsTcTp1->SetMarkerStyle(7);
    hTCvsTcTp1->SetMarkerColor(kRed);
    hTCvsTcTp1->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp2 = new TH2F("hTCvsTcTp2","TC vs (Emulator - ECONT) for TcTp==2 ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp2->GetXaxis()->SetTitle("TC");
    hTCvsTcTp2->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp2->SetLineColor(kBlue);
    hTCvsTcTp2->SetOption("scat");
    hTCvsTcTp2->SetMarkerStyle(7);
    hTCvsTcTp2->SetMarkerColor(kBlue);
    hTCvsTcTp2->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1et2 = new TH2F("hTCvsTcTp1et2","TC vs (Emulator - ECONT) for (TcTp==1 and TcTp==2)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1et2->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1et2->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1et2->SetLineColor(kMagenta);
    hTCvsTcTp1et2->SetOption("scat");
    hTCvsTcTp1et2->SetMarkerStyle(7);
    hTCvsTcTp1et2->SetMarkerColor(kMagenta);
    hTCvsTcTp1et2->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1S = new TH2F("hTCvsTcTp1S","TC vs (Emulator - ECONT) for single TcTp==1 ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1S->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1S->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1S->SetLineColor(kRed);
    hTCvsTcTp1S->SetOption("scat");
    hTCvsTcTp1S->SetMarkerStyle(7);
    hTCvsTcTp1S->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1D = new TH2F("hTCvsTcTp1D","TC vs (Emulator - ECONT) for double TcTp==1 ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1D->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1D->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1D->SetLineColor(kRed);
    hTCvsTcTp1D->SetOption("scat");
    hTCvsTcTp1D->SetMarkerStyle(7);
    hTCvsTcTp1D->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1Sat = new TH2F("hTCvsTcTp1Sat","TC vs (Emulator - ECONT) for TcTp==1 (excl) and saturated ADC ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1Sat->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1Sat->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1Sat->SetLineColor(kRed);
    hTCvsTcTp1Sat->SetOption("scat");
    hTCvsTcTp1Sat->SetMarkerStyle(7);
    hTCvsTcTp1Sat->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1Ushoot = new TH2F("hTCvsTcTp1Ushoot","TC vs (Emulator - ECONT) for TcTp==1 (excl) and undershoot ADC ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1Ushoot->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1Ushoot->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1Ushoot->SetLineColor(kRed);
    hTCvsTcTp1Ushoot->SetOption("scat");
    hTCvsTcTp1Ushoot->SetMarkerStyle(7);
    hTCvsTcTp1Ushoot->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1NoSatUshoot = new TH2F("hTCvsTcTp1NoSatUshoot","TC vs (Emulator - ECONT) for TcTp==1 (excl) and NO saturated/undershoot ADC ", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1NoSatUshoot->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1NoSatUshoot->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1NoSatUshoot->SetLineColor(kRed);
    hTCvsTcTp1NoSatUshoot->SetOption("scat");
    hTCvsTcTp1NoSatUshoot->SetMarkerStyle(7);
    hTCvsTcTp1NoSatUshoot->SetDirectory(dir_diff);    
    
  }else{
    
    TH1F *hCompressDiffSTCADC[12],*hCompressDiffSTCTOT[12];
    for(int istc=0;istc<12;istc++){
      hCompressDiffSTCADC[istc] = new TH1F(Form("hCompressDiffSTCADC_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC4A : %d with totflag==0",istc), 200, -99, 101);
      hCompressDiffSTCADC[istc]->SetMinimum(1.e-1);
      hCompressDiffSTCADC[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffSTCADC[istc]->SetLineColor(kRed);
      hCompressDiffSTCADC[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hCompressDiffSTCTOT[istc] = new TH1F(Form("hCompressDiffSTCTOT_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC4A : %d with atleats one totflag==3",istc), 200, -99, 101);
      hCompressDiffSTCTOT[istc]->SetMinimum(1.e-1);
      hCompressDiffSTCTOT[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffSTCTOT[istc]->SetLineColor(kBlue);
      hCompressDiffSTCTOT[istc]->SetDirectory(dir_diff);
    }
    TH1F *hCompressDiffSTCTcTp1[12],*hCompressDiffSTCTcTp2[12];
    for(int istc=0;istc<12;istc++){
      hCompressDiffSTCTcTp1[istc] = new TH1F(Form("hCompressDiffSTCTcTp1_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC4A : %d with atleast one totflag==1",istc), 200, -99, 101);
      hCompressDiffSTCTcTp1[istc]->SetMinimum(1.e-1);
      hCompressDiffSTCTcTp1[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffSTCTcTp1[istc]->SetLineColor(kRed);
      hCompressDiffSTCTcTp1[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hCompressDiffSTCTcTp2[istc] = new TH1F(Form("hCompressDiffSTCTcTp2_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC4A : %d with atleast one totflag==2",istc), 200, -99, 101);
      hCompressDiffSTCTcTp2[istc]->SetMinimum(1.e-1);
      hCompressDiffSTCTcTp2[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffSTCTcTp2[istc]->SetLineColor(kBlue);
      hCompressDiffSTCTcTp2[istc]->SetDirectory(dir_diff);
    }
    TH1F *hECONTTcTp0[12],*hEmulTcTp0[12],*hECONTTcTp3[12],*hEmulTcTp3[12],*hECONTTcTp1[12],*hEmulTcTp1[12],*hECONTTcTp2[12],*hEmulTcTp2[12];
    for(int istc=0;istc<12;istc++){
      hECONTTcTp0[istc] = new TH1F(Form("hECONTTcTp0_%d",istc),Form("ECONT compressed energy for STC : %d with totflag==0 for all sensors",istc), 200, -99, 101);
      hECONTTcTp0[istc]->SetMinimum(1.e-1);
      hECONTTcTp0[istc]->GetXaxis()->SetTitle("Compressed energy");
      hECONTTcTp0[istc]->SetLineColor(kMagenta);
      hECONTTcTp0[istc]->SetLineWidth(4);
      hECONTTcTp0[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hEmulTcTp0[istc] = new TH1F(Form("hEmulTcTp0_%d",istc),Form("Emulated compressed energy for STC : %d with totflag==0 for all sensors",istc), 200, -99, 101);
      hEmulTcTp0[istc]->SetMinimum(1.e-1);
      hEmulTcTp0[istc]->GetXaxis()->SetTitle("Compressed energy");
      hEmulTcTp0[istc]->SetLineColor(kBlack);
      hEmulTcTp0[istc]->SetLineWidth(2);
      hEmulTcTp0[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hECONTTcTp3[istc] = new TH1F(Form("hECONTTcTp3_%d",istc),Form("ECONT compressed energy for STC : %d with totflag==3 for atleast one sensor",istc), 200, -99, 101);
      hECONTTcTp3[istc]->SetMinimum(1.e-1);
      hECONTTcTp3[istc]->GetXaxis()->SetTitle("Compressed energy");
      hECONTTcTp3[istc]->SetLineColor(kMagenta);
      hECONTTcTp3[istc]->SetLineWidth(4);
      hECONTTcTp3[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hEmulTcTp3[istc] = new TH1F(Form("hEmulTcTp3_%d",istc),Form("Emulated compressed energy for STC : %d with totflag==3 for atleast one sensor",istc), 200, -99, 101);
      hEmulTcTp3[istc]->SetMinimum(1.e-1);
      hEmulTcTp3[istc]->GetXaxis()->SetTitle("Compressed energy");
      hEmulTcTp3[istc]->SetLineColor(kBlack);
      hEmulTcTp3[istc]->SetLineWidth(2);
      hEmulTcTp3[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hECONTTcTp1[istc] = new TH1F(Form("hECONTTcTp1_%d",istc),Form("ECONT compressed energy for STC : %d with totflag==1 for atleast one sensor",istc), 200, -99, 101);
      hECONTTcTp1[istc]->SetMinimum(1.e-1);
      hECONTTcTp1[istc]->GetXaxis()->SetTitle("Compressed energy");
      hECONTTcTp1[istc]->SetLineColor(kMagenta);
      hECONTTcTp1[istc]->SetLineWidth(4);
      hECONTTcTp1[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hEmulTcTp1[istc] = new TH1F(Form("hEmulTcTp1_%d",istc),Form("Emulated compressed energy for STC : %d with totflag==1 for atleast one sensor",istc), 200, -99, 101);
      hEmulTcTp1[istc]->SetMinimum(1.e-1);
      hEmulTcTp1[istc]->GetXaxis()->SetTitle("Compressed energy");
      hEmulTcTp1[istc]->SetLineColor(kBlack);
      hEmulTcTp1[istc]->SetLineWidth(2);
      hEmulTcTp1[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hECONTTcTp2[istc] = new TH1F(Form("hECONTTcTp2_%d",istc),Form("ECONT compressed energy for STC : %d with totflag==2 for atleast one sensor",istc), 200, -99, 101);
      hECONTTcTp2[istc]->SetMinimum(1.e-1);
      hECONTTcTp2[istc]->GetXaxis()->SetTitle("Compressed energy");
      hECONTTcTp2[istc]->SetLineColor(kMagenta);
      hECONTTcTp2[istc]->SetLineWidth(4);
      hECONTTcTp2[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
      hEmulTcTp2[istc] = new TH1F(Form("hEmulTcTp2_%d",istc),Form("Emulated compressed energy for STC : %d with totflag==2 for atleast one sensor",istc), 200, -99, 101);
      hEmulTcTp2[istc]->SetMinimum(1.e-1);
      hEmulTcTp2[istc]->GetXaxis()->SetTitle("Compressed energy");
      hEmulTcTp2[istc]->SetLineColor(kBlack);
      hEmulTcTp2[istc]->SetLineWidth(2);
      hEmulTcTp2[istc]->SetDirectory(dir_diff);
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
 
    TH1F *hEventCount = new TH1F("hEventCount","Event count", 13, -0.5, 12.5);
    hEventCount->SetMinimum(1.e-1);
    hEventCount->SetLineColor(kRed);
    hEventCount->GetXaxis()->SetBinLabel(2,"Total");
    hEventCount->GetXaxis()->SetBinLabel(3,"passed TcTp=0/3 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(4,"passed TcTp=1/2 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(5,"atleast a TcTp=1");
    hEventCount->GetXaxis()->SetBinLabel(6,"passed TcTp=1 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(7,"TcTp=1/2");
    hEventCount->GetXaxis()->SetBinLabel(8,"atleast a TcTp=2");
    hEventCount->GetXaxis()->SetBinLabel(9,"passed TcTp=2 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(10,"passed TcTp=1 (excl) Sat");
    hEventCount->GetXaxis()->SetBinLabel(11,"passed TcTp=1 (excl) Ushoot");
    hEventCount->GetXaxis()->SetBinLabel(12,"passed TcTp=1 (excl) NoSatUshoot");
    hEventCount->SetDirectory(dir_diff);
    TH2F *hSTCvsTcTp0 = new TH2F("hSTCvsTcTp0","STC vs (Emulator - ECONT) for TcTp==0 ", 15, -1.5, 13.5, 200, -99, 101);
    hSTCvsTcTp0->GetXaxis()->SetTitle("STC");
    hSTCvsTcTp0->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hSTCvsTcTp0->SetLineColor(kRed);
    hSTCvsTcTp0->SetOption("scat");
    hSTCvsTcTp0->SetMarkerStyle(7);
    hSTCvsTcTp0->SetMarkerColor(kRed);
    hSTCvsTcTp0->SetDirectory(dir_diff);
    TH2F *hSTCvsTcTp3 = new TH2F("hSTCvsTcTp3","STC vs (Emulator - ECONT) for TcTp==3 ", 15, -1.5, 13.5, 200, -99, 101);
    hSTCvsTcTp3->GetXaxis()->SetTitle("STC");
    hSTCvsTcTp3->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hSTCvsTcTp3->SetLineColor(kBlue);
    hSTCvsTcTp3->SetOption("scat");
    hSTCvsTcTp3->SetMarkerStyle(7);
    hSTCvsTcTp3->SetMarkerColor(kBlue);
    hSTCvsTcTp3->SetDirectory(dir_diff);
    TH2F *hSTCvsTcTp1 = new TH2F("hSTCvsTcTp1","STC vs (Emulator - ECONT) for TcTp==1 ", 15, -1.5, 13.5, 200, -99, 101);
    hSTCvsTcTp1->GetXaxis()->SetTitle("STC");
    hSTCvsTcTp1->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hSTCvsTcTp1->SetLineColor(kRed);
    hSTCvsTcTp1->SetOption("scat");
    hSTCvsTcTp1->SetMarkerStyle(7);
    hSTCvsTcTp1->SetMarkerColor(kRed);
    hSTCvsTcTp1->SetDirectory(dir_diff);
    TH2F *hSTCvsTcTp2 = new TH2F("hSTCvsTcTp2","STC vs (Emulator - ECONT) for TcTp==2 ", 15, -1.5, 13.5, 200, -99, 101);
    hSTCvsTcTp2->GetXaxis()->SetTitle("STC");
    hSTCvsTcTp2->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hSTCvsTcTp2->SetLineColor(kBlue);
    hSTCvsTcTp2->SetOption("scat");
    hSTCvsTcTp2->SetMarkerStyle(7);
    hSTCvsTcTp2->SetMarkerColor(kBlue);
    hSTCvsTcTp2->SetDirectory(dir_diff);
    TH2F *hSTCvsTcTp1Sat = new TH2F("hSTCvsTcTp1Sat","STC vs (Emulator - ECONT) for TcTp==1 (excl) and saturated ADC ", 15, -1.5, 13.5, 200, -99, 101);
    hSTCvsTcTp1Sat->GetXaxis()->SetTitle("STC");
    hSTCvsTcTp1Sat->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hSTCvsTcTp1Sat->SetLineColor(kRed);
    hSTCvsTcTp1Sat->SetOption("scat");
    hSTCvsTcTp1Sat->SetMarkerStyle(7);
    hSTCvsTcTp1Sat->SetDirectory(dir_diff);
    TH2F *hSTCvsTcTp1Ushoot = new TH2F("hSTCvsTcTp1Ushoot","STC vs (Emulator - ECONT) for TcTp==1 (excl) and undershoot ADC ", 15, -1.5, 13.5, 200, -99, 101);
    hSTCvsTcTp1Ushoot->GetXaxis()->SetTitle("STC");
    hSTCvsTcTp1Ushoot->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hSTCvsTcTp1Ushoot->SetLineColor(kRed);
    hSTCvsTcTp1Ushoot->SetOption("scat");
    hSTCvsTcTp1Ushoot->SetMarkerStyle(7);
    hSTCvsTcTp1Ushoot->SetDirectory(dir_diff);
    TH2F *hSTCvsTcTp1NoSatUshoot = new TH2F("hSTCvsTcTp1NoSatUshoot","STC vs (Emulator - ECONT) for TcTp==1 (excl) and NO saturated/undershoot ADC ", 15, -1.5, 13.5, 200, -99, 101);
    hSTCvsTcTp1NoSatUshoot->GetXaxis()->SetTitle("STC");
    hSTCvsTcTp1NoSatUshoot->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hSTCvsTcTp1NoSatUshoot->SetLineColor(kRed);
    hSTCvsTcTp1NoSatUshoot->SetOption("scat");
    hSTCvsTcTp1NoSatUshoot->SetMarkerStyle(7);
    hSTCvsTcTp1NoSatUshoot->SetDirectory(dir_diff);    

  }
  
  TH1F *hSeqTcTp1 = new TH1F("hSeqTcTp1","Seq distribution with totflag==1", 230, -0.5, 229.5);
  hSeqTcTp1->SetMinimum(1.e-1);
  hSeqTcTp1->GetXaxis()->SetTitle("Seq with TcTp==1");
  hSeqTcTp1->SetLineColor(kAzure);
  hSeqTcTp1->SetDirectory(dir_diff);
  TH1F *hAbsRocpinTcTp1 = new TH1F("hAbsRocpinTcTp1","AbsRocpin distribution with totflag==1", 230, -0.5, 229.5);
  hAbsRocpinTcTp1->SetMinimum(1.e-1);
  hAbsRocpinTcTp1->GetXaxis()->SetTitle("AbsRocpin with TcTp==1");
  hAbsRocpinTcTp1->SetLineColor(kAzure);
  hAbsRocpinTcTp1->SetDirectory(dir_diff);

}

//FillHistogram(cfg, hrocarray, modarray, econtemularray, econtarray, eventList, dir_diff, isSTC4);
void FillHistogram(TPGFEConfiguration::Configuration& cfgs,                                                             //to plot pedestal/threshold histograms
		   const std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::HalfHgcrocData>>>& hrocarray,           //to plot the ADC/TOT histograms
		   const std::map<uint64_t,std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>>& modarray,             //to plot HGROC emulation results
		   const std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>>& econtemularray,   //to plot the ECONT emulation results
		   const std::map<uint64_t,std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>>& econtarray,   //to plot the ECONT ASIC results or compare with the emulation
		   const std::vector<uint64_t>& eventList, const uint32_t& moduleId, TDirectory*& dir_diff, TDirectory*& dir_charge, bool isSTC4){
  
  const TPGFEDataformat::TcRawData::Type& outputType = cfgs.getEconTPara().at(moduleId).getOutType();
  TPGFEConfiguration::TPGFEIdPacking pck;
  pck.setModId(moduleId);    
  const std::map<std::tuple<uint32_t,uint32_t,uint32_t>,std::string>& modNameMap = cfgs.getModIdxToName();
  const std::string& modName = modNameMap.at(std::make_tuple(pck.getDetType(),pck.getSelTC4(),pck.getModule()));
  const std::map<std::string,std::vector<uint32_t>>& modSTClist = (pck.getDetType()==0)?cfgs.getSiModSTClist():cfgs.getSciModSTClist();
  const std::vector<uint32_t>& stclist = modSTClist.at(modName) ;
  const std::map<std::pair<std::string,uint32_t>,std::vector<uint32_t>>& stcTcMap = (pck.getDetType()==0)?cfgs.getSiSTCToTC():cfgs.getSciSTCToTC();
  const std::map<std::pair<std::string,uint32_t>,uint32_t>& rocpintoseq = (pck.getDetType()==0)?cfgs.getSiRocpinToAbsSeq():cfgs.getSciRocpinToAbsSeq();
  std::map<uint32_t,TPGFEDataformat::HalfHgcrocData> rocdata;
  
  TList *list = (TList *)dir_diff->GetList();
  TList *list_charge = (TList *)dir_charge->GetList();
  
  std::map<uint32_t,TPGFEDataformat::ModuleTcData> moddata;
  
  for(const auto& event : eventList){

    if(event!=refPrE and isDebug) continue;
    if(econtarray.find(event) == econtarray.end()) continue;
    
    const std::vector<std::pair<uint32_t,TPGFEDataformat::ModuleTcData>>& modtcs = modarray.at(event);
    const std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>& econtemultcs = econtemularray.at(event);
    const std::vector<std::pair<uint32_t,std::vector<TPGFEDataformat::TcRawData>>>& econttcs = econtarray.at(event);
    
    rocdata.clear();
    for(const auto& data : hrocarray.at(event)){
      rocdata[data.first] = data.second ;
    }

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
	
	bool isTotMod = false;
	bool isModTcTp12 = false;
	const uint32_t nofTcs = modtcdata.getNofTCs();
	uint32_t nofTcTp1[nofTcs], nofTcTp2[nofTcs], nofSat[nofTcs], nofUndsht[nofTcs];
	for(uint32_t itc=0;itc<modtcdata.getNofTCs();itc++) nofTcTp1[itc] =  nofTcTp2[itc] =  nofSat[itc] =  nofUndsht[itc] = 0;
	const uint32_t nofSTCs = stclist.size();
	uint32_t nofSTCTcTp1[nofSTCs], nofSTCTcTp2[nofSTCs], nofSTCSat[nofSTCs], nofSTCUndsht[nofSTCs];
	for(uint32_t istc=0;istc<nofSTCs;istc++) nofSTCTcTp1[istc] =  nofSTCTcTp2[istc] =  nofSTCSat[istc] =  nofSTCUndsht[istc] = 0;
	uint32_t nofTcTp1_evt = 0, nofTcTp2_evt = 0, nofSat_evt = 0, nofUndsht_evt = 0;
	bool *isTcTp12 = new bool[modtcdata.getNofTCs()];
	///////////////////// Fill the ADC/TOT ////////////////////////////////////////////
	for(uint32_t itc=0;itc<modtcdata.getNofTCs();itc++){
	  const std::vector<uint32_t>& pinlst = (pck.getDetType()==0)?cfgs.getSiTCToROCpin().at(std::make_pair(modName,itc)):cfgs.getSciTCToROCpin().at(std::make_pair(modName,itc));
	  isTcTp12[itc] = false;
	  for(const uint32_t& tcch : pinlst){
	    uint32_t absseq = rocpintoseq.at(std::make_pair(modName,tcch));
	    uint32_t rocpin = tcch%36 ;
	    uint32_t rocn = TMath::Floor(tcch/72);
	    uint32_t half = (int(TMath::Floor(tcch/36))%2==0)?0:1;
	    uint32_t rocid = pck.getRocIdFromModId(moduleId,rocn,half);
	    uint32_t istc = uint32_t(TMath::FloorNint(itc/4));
	    const TPGFEDataformat::HalfHgcrocChannelData& chdata = rocdata.at(rocid).getChannelData(rocpin);
	    if(chdata.getTcTp()==1 or chdata.getTcTp()==2){
	      isTcTp12[itc] = true;
	      isModTcTp12 = true;
	      if(chdata.getTcTp()==1){
	       	((TH1F *) list->FindObject("hAbsRocpinTcTp1"))->Fill(float( tcch ));
	       	((TH1F *) list->FindObject("hSeqTcTp1"))->Fill(float( absseq ));
		if(!chdata.isTot() and chdata.getAdc()>1020) {nofSTCSat[istc]++; nofSat[itc]++; nofSat_evt++;}
		if(!chdata.isTot() and chdata.getAdc()<4) {nofSTCUndsht[istc]++; nofUndsht[itc]++; nofUndsht_evt++;}
		nofSTCTcTp1[istc]++;
		nofTcTp1[itc]++;
		nofTcTp1_evt++;
	      }else{
		nofSTCTcTp2[istc]++;
		nofTcTp2[itc]++;
		nofTcTp2_evt++;
	      }
	    }
	    if(!chdata.isTot()){
	      uint32_t adc = chdata.getAdc();
	      if(chdata.getTcTp()==0) ((TH1F *) list_charge->FindObject(Form("hADCTcTp0_%d_%d_%d",rocn,half,rocpin)))->Fill(float( adc ));
	      if(chdata.getTcTp()==1) ((TH1F *) list_charge->FindObject(Form("hADCTcTp1_%d_%d_%d",rocn,half,rocpin)))->Fill(float( adc ));
	      std::string title = ((TH1F *) list_charge->FindObject(Form("hADCTcTp0_%d_%d_%d",rocn,half,rocpin)))->GetTitle();
	      if(title.find("seq")==std::string::npos)
		((TH1F *) list_charge->FindObject(Form("hADCTcTp0_%d_%d_%d",rocn,half,rocpin)))->SetTitle(Form("%s_seq_%d",title.c_str(),absseq));
	    }else{
	      isTotMod = true;
	    }//istot	      	    
	  }//rocpin loop
	}//TC loop for charge histogram
	
	
	if(!isSTC4){ //best choice
	  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  //Bestchoice emulation data is already sorted, but the data from econt is not
	  const uint32_t& nofBCTcs = cfgs.getEconTPara().at(moduleId).getBCType();
	  uint32_t *sorted_idx = new uint32_t[nofBCTcs];
	  uint32_t *energy = new uint32_t[nofBCTcs];
	  uint32_t *channel = new uint32_t[nofBCTcs];
	  // uint32_t *emul_energy = new uint32_t[nofBCTcs];
	  // uint32_t *emul_channel = new uint32_t[nofBCTcs];
	  uint32_t *emul_energy = new uint32_t[(econtemulTcRawdata.size()-1)];
	  uint32_t *emul_channel = new uint32_t[(econtemulTcRawdata.size()-1)];
	  uint32_t ibc = 0;
	  uint32_t econtmodsum = 0,  econtemulmodsum = 0;
	  for(const auto& econtdata : econtTcRawdata){
	    if(econtdata.isModuleSum()){
	      econtmodsum = uint32_t(econtdata.moduleSum());
	    }else{
	      energy[ibc] = uint32_t(econtdata.energy());
	      channel[ibc] = uint32_t(econtdata.address());
	      if(event==refPrE){
		std::cout << "ibc: "<<ibc<<", channel: "<<channel[ibc]<<", energy: "<<energy[ibc]<<std::endl;
	      }
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
	  //Now compare
	  for(uint32_t itc = 0 ; itc<nofBCTcs ; itc++){
	    
	    uint32_t emch = emul_channel[channel[sorted_idx[itc]]];
	    uint32_t emen = emul_energy[channel[sorted_idx[itc]]];
	    int cdiff =  emch - channel[sorted_idx[itc]];
	    int ediff =  emen - energy[sorted_idx[itc]];
	    
	    if(event==refPrE){
	      std::cout << "ibc: "<<itc<<", emul_channel: "<<emch<<", channel: "<<channel[sorted_idx[itc]]
			<<", emul_energy: "<< emen << ", energy: "<<energy[sorted_idx[itc]]<<std::endl;
	    }

	    if(!isTcTp12[channel[sorted_idx[itc]]]){ 
	      //TcTp=0/3
	      bool isTot = modtcdata.getTC(emch).isTot();
	      if(cdiff==0){
		if(!isTot){
		  ((TH1F *) list->FindObject(Form("hCompressDiffTCADC_%d",emch)))->Fill(float( ediff ));
		  ((TH2F *) list->FindObject("hTCvsTcTp0"))->Fill(emch, ediff);
		  ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",emch)))->Fill(float( emen ));
		  ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",emch)))->Fill(float( energy[sorted_idx[itc]] ));
		  if(TMath::Abs(ediff)>0) std::cerr << "Event: "<<event<<" has problem for (ADC)TC channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		}else{
		  ((TH1F *) list->FindObject(Form("hCompressDiffTCTOT_%d",emch)))->Fill(float( ediff ));
		  ((TH2F *) list->FindObject("hTCvsTcTp3"))->Fill(emch, ediff);
		  ((TH1F *) list->FindObject(Form("hEmulTcTp3_%d",emch)))->Fill(float( emen ));
		  ((TH1F *) list->FindObject(Form("hECONTTcTp3_%d",emch)))->Fill(float( energy[sorted_idx[itc]] ));
		  if(TMath::Abs(ediff)>0 and ediff!=-1) std::cerr << "Event: "<<event<<" has problem for (TOT)TC channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		}
	      }else{
		if(!isTot)
		  ((TH1F *) list->FindObject("hBC9TCMissedADC"))->Fill(float( channel[sorted_idx[itc]] ));
		else
		  ((TH1F *) list->FindObject("hBC9TCMissedTOT"))->Fill(float( channel[sorted_idx[itc]] ));
	      }//cdiff condn
	    }else{
	      //TcTp=1/2
	      if(nofTcTp1[emch]>0){
		if(nofTcTp2[emch]==0){
		  ((TH1F *) list->FindObject(Form("hCompressDiffTCTcTp1_%d",emch)))->Fill(float( ediff ));
		  ((TH2F *) list->FindObject("hTCvsTcTp1"))->Fill(emch, ediff);
		  ((TH1F *) list->FindObject(Form("hEmulTcTp1_%d",emch)))->Fill(float( emen ));
		  ((TH1F *) list->FindObject(Form("hECONTTcTp1_%d",emch)))->Fill(float( energy[sorted_idx[itc]] ));
		  if(nofSat[emch]>0 and nofUndsht[emch]==0) {
		    ((TH2F *) list->FindObject("hTCvsTcTp1Sat"))->Fill(emch, ediff);
		    //if(TMath::Abs(ediff)>0) std::cerr << "Event: "<<event<<" has Saturation with channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		  }
		  if(nofSat[emch]==0 and nofUndsht[emch]>0) {
		    ((TH2F *) list->FindObject("hTCvsTcTp1Ushoot"))->Fill(emch, ediff);
		    //if(TMath::Abs(ediff)>0) std::cerr << "Event: "<<event<<" has Undershoot with channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		    //if(TMath::Abs(ediff)>5 and nofUndsht[emch]==1) std::cerr << "Event: "<<event<<" has Undershoot with channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		  }
		  if(nofSat[emch]==0 and nofUndsht[emch]==0) {
		    ((TH2F *) list->FindObject("hTCvsTcTp1NoSatUshoot"))->Fill(emch, ediff);
		    //if(TMath::Abs(ediff)>0) std::cerr << "Event: "<<event<<" has NoSatUshoot with channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		  }
		  if(nofTcTp1[emch]==1) ((TH2F *) list->FindObject("hTCvsTcTp1S"))->Fill(emch, ediff);
		  if(nofTcTp1[emch]==2) ((TH2F *) list->FindObject("hTCvsTcTp1D"))->Fill(emch, ediff);
		}else
		  ((TH2F *) list->FindObject("hTCvsTcTp1et2"))->Fill(emch, ediff);	
	      }//TcTp1 > 0
	      if(nofTcTp1[emch]==0 and nofTcTp2[emch]>0) {
		((TH1F *) list->FindObject(Form("hCompressDiffTCTcTp2_%d",emch)))->Fill(float( ediff ));
		((TH2F *) list->FindObject("hTCvsTcTp2"))->Fill(emch, ediff);
		((TH1F *) list->FindObject(Form("hEmulTcTp2_%d",emch)))->Fill(float( emen ));
		((TH1F *) list->FindObject(Form("hECONTTcTp2_%d",emch)))->Fill(float( energy[sorted_idx[itc]] ));
	      }//TcTp2 > 0
	    }//TcTp condition
	  }//tc loop
	  
	  int moddiff = econtemulmodsum - econtmodsum;
	  ((TH1F *) list->FindObject("hEventCount"))->Fill(1);
	  if(!isModTcTp12){
	    if(!isTotMod){
	      ((TH1F *) list->FindObject("hModSumDiffADC"))->Fill(float( moddiff ));
	      if(TMath::Abs(moddiff)>0) std::cerr << "Event: "<<event<<" has problem in (ADC)modsum emul:"<<econtemulmodsum<<", econt : "<<econtmodsum<<std::endl;
	    }else{
	      ((TH1F *) list->FindObject("hModSumDiffTOT"))->Fill(float( moddiff ));
	      if(TMath::Abs(moddiff)>0 and moddiff!=-1) std::cerr << "Event: "<<event<<" has problem for (TOT) modsum emul: "<<econtemulmodsum<<", econt: "<<econtmodsum<<std::endl;
	    }
	    ((TH1F *) list->FindObject("hEventCount"))->Fill(2);
	  }else{
	    ((TH1F *) list->FindObject("hModSumDiffTcTp12"))->Fill(float( moddiff ));
	    if(nofTcTp1_evt>0){
	      ((TH1F *) list->FindObject("hEventCount"))->Fill(4);
	      if(nofTcTp2_evt==0){
		((TH1F *) list->FindObject("hEventCount"))->Fill(5);
		if(nofUndsht_evt==0 and nofSat_evt>0) ((TH1F *) list->FindObject("hEventCount"))->Fill(9);
		if(nofUndsht_evt>0 and nofSat_evt==0) ((TH1F *) list->FindObject("hEventCount"))->Fill(10);
		if(nofUndsht_evt==0 and nofSat_evt==0) ((TH1F *) list->FindObject("hEventCount"))->Fill(11);
	      }else
		((TH1F *) list->FindObject("hEventCount"))->Fill(6);
	    }
	    if(nofTcTp2_evt>0){
	      ((TH1F *) list->FindObject("hEventCount"))->Fill(7);
	      if(nofTcTp1_evt==0) ((TH1F *) list->FindObject("hEventCount"))->Fill(8);
	      // else
	      // 	((TH1F *) list->FindObject("hEventCount"))->Fill(9);
	    }
	    ((TH1F *) list->FindObject("hEventCount"))->Fill(3);
	  }
	  // hEventCount->GetXaxis()->SetBinLabel(2,"Total");
	  // hEventCount->GetXaxis()->SetBinLabel(3,"passed TcTp=0/3 (excl)");
	  // hEventCount->GetXaxis()->SetBinLabel(4,"passed TcTp=1/2 (excl)");
	  // hEventCount->GetXaxis()->SetBinLabel(5,"atleast a TcTp=1");
	  // hEventCount->GetXaxis()->SetBinLabel(6,"passed TcTp=1 (excl)");
	  // hEventCount->GetXaxis()->SetBinLabel(7,"TcTp=1/2");
	  // hEventCount->GetXaxis()->SetBinLabel(8,"atleast a TcTp=2");
	  // hEventCount->GetXaxis()->SetBinLabel(9,"passed TcTp=2 (excl)");
	  // hEventCount->GetXaxis()->SetBinLabel(10,"passed TcTp=1 (excl) Sat");
	  // hEventCount->GetXaxis()->SetBinLabel(11,"passed TcTp=1 (excl) Ushoot");
	  // hEventCount->GetXaxis()->SetBinLabel(12,"passed TcTp=1 (excl) NoSatUshoot");

	  delete []emul_channel ;
	  delete []emul_energy ;
	  delete []channel ;
	  delete []energy ;
	  delete []sorted_idx ;
	  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}else{ //STC
	  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	  
	  if(econtemulTcRawdata.size()!=econtTcRawdata.size()) continue;
	  
	  for(size_t istc = 0 ; istc< econtemulTcRawdata.size() ; istc++){
	    uint32_t econt_energy = uint32_t(econtTcRawdata.at(istc).energy());
	    uint32_t econt_loc = uint32_t(econtTcRawdata.at(istc).address());
 	    uint32_t emul_energy = uint32_t(econtemulTcRawdata.at(istc).energy());
	    uint32_t emul_loc = uint32_t(econtemulTcRawdata.at(istc).address());
	    //int cdiff = emul_loc - econt_loc; //for the moment not checking the TC with maximum energy
	    int cdiff = 0; //only for current test with energy values
	    int ediff =  emul_energy - econt_energy;
	    bool isTot = false ;
	    bool isTcTp1or2 = false;
	    const std::vector<uint32_t>& tclist = stcTcMap.at(std::make_pair(modName,istc));
	    for (const auto& itc : tclist){
	      if(modtcdata.getTC(itc).isTot()) isTot = true ;
	      if(nofTcTp1[itc]>0 or nofTcTp1[itc]>0) isTcTp1or2 = true;
	    }
	    if(!isTcTp1or2){	    
	      if(cdiff==0 and (emul_energy!=0 and econt_energy!=0)){
		if(!isTot){
		  ((TH1F *) list->FindObject(Form("hCompressDiffSTCADC_%d",istc)))->Fill(float( ediff ));
		  ((TH2F *) list->FindObject("hSTCvsTcTp0"))->Fill(istc, ediff);
		  ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",istc)))->Fill(float( emul_energy ));
		  ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",istc)))->Fill(float( econt_energy ));
		  if(TMath::Abs(ediff)>0) std::cerr << "Event: "<<event<<" has problem for (ADC)STC channel: "<< istc << ", emul: "<<emul_energy<<", econt: "<<econt_energy<<std::endl;
		}else{
		  ((TH1F *) list->FindObject(Form("hCompressDiffSTCTOT_%d",istc)))->Fill(float( ediff ));
		  ((TH2F *) list->FindObject("hSTCvsTcTp3"))->Fill(istc, ediff);
		  ((TH1F *) list->FindObject(Form("hEmulTcTp3_%d",istc)))->Fill(float( emul_energy ));
		  ((TH1F *) list->FindObject(Form("hECONTTcTp3_%d",istc)))->Fill(float( econt_energy ));
		  if(TMath::Abs(ediff)>0 and ediff!=-1) std::cerr << "Event: "<<event<<" has problem for (TOT)STC channel: "<< istc << ", emul: "<<emul_energy<<", econt: "<<econt_energy<<std::endl;
		}
	      }else{
		if(!isTot)
		  ((TH1F *) list->FindObject("hSTC4TCMissedADC"))->Fill(float( econt_loc ));
		else
		  ((TH1F *) list->FindObject("hSTC4TCMissedADC"))->Fill(float( econt_loc ));
	      }
	    }else{

	      if(nofSTCTcTp1[istc]>0 and nofSTCTcTp2[istc]==0){
		((TH1F *) list->FindObject(Form("hCompressDiffSTCTcTp1_%d",istc)))->Fill(float( ediff ));
		((TH2F *) list->FindObject("hSTCvsTcTp1"))->Fill(istc, ediff);
		((TH1F *) list->FindObject(Form("hEmulTcTp1_%d",istc)))->Fill(float( emul_energy ));
		((TH1F *) list->FindObject(Form("hECONTTcTp1_%d",istc)))->Fill(float( econt_energy ));	      
		if(nofSTCSat[istc]>0 and nofSTCUndsht[istc]==0){
		  ((TH2F *) list->FindObject("hSTCvsTcTp1Sat"))->Fill(istc, ediff);
		}
		if(nofSTCSat[istc]==0 and nofSTCUndsht[istc]>0){
		  ((TH2F *) list->FindObject("hSTCvsTcTp1Ushoot"))->Fill(istc, ediff);
		}
		if(nofSTCSat[istc]==0 and nofSTCUndsht[istc]==0){
		  ((TH2F *) list->FindObject("hSTCvsTcTp1NoSatUshoot"))->Fill(istc, ediff);
		}
	      }//if TcTp == 1
	      if(nofSTCTcTp1[istc]==0 and nofSTCTcTp2[istc]>0){
		((TH1F *) list->FindObject(Form("hCompressDiffSTCTcTp2_%d",istc)))->Fill(float( ediff ));
		((TH2F *) list->FindObject("hSTCvsTcTp2"))->Fill(istc, ediff);
		((TH1F *) list->FindObject(Form("hEmulTcTp2_%d",istc)))->Fill(float( emul_energy ));
		((TH1F *) list->FindObject(Form("hECONTTcTp2_%d",istc)))->Fill(float( econt_energy ));
	      }//exclusive TcTp == 2
	    }//if TcTp == 1 or 2
	  }//stc loop

	  ((TH1F *) list->FindObject("hEventCount"))->Fill(1);
	  if(!isModTcTp12){
	    ((TH1F *) list->FindObject("hEventCount"))->Fill(2);
	  }else{
	    if(nofTcTp1_evt>0){
	      ((TH1F *) list->FindObject("hEventCount"))->Fill(4);
	      if(nofTcTp2_evt==0){
		((TH1F *) list->FindObject("hEventCount"))->Fill(5);
		if(nofUndsht_evt==0 and nofSat_evt>0) ((TH1F *) list->FindObject("hEventCount"))->Fill(9);
		if(nofUndsht_evt>0 and nofSat_evt==0) ((TH1F *) list->FindObject("hEventCount"))->Fill(10);
		if(nofUndsht_evt==0 and nofSat_evt==0) ((TH1F *) list->FindObject("hEventCount"))->Fill(11);
	      }else
		((TH1F *) list->FindObject("hEventCount"))->Fill(6);
	    }
	    if(nofTcTp2_evt>0){
	      ((TH1F *) list->FindObject("hEventCount"))->Fill(7);
	      if(nofTcTp1_evt==0) ((TH1F *) list->FindObject("hEventCount"))->Fill(8);
	    }
	    ((TH1F *) list->FindObject("hEventCount"))->Fill(3); //TcTp == 1 or 2
	  }
	  
	  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}//isSTC
	delete []isTcTp12;
      }//econt emulation loop
    }//econt data loop
    
  }//event loop
}

void FillSummaryHistogram(TFile*& fout, TDirectory*& dir_diff, TDirectory*& dir_charge, bool isSTC4)
{
  TList *list = (TList *)dir_diff->GetList();
  if(!isSTC4){ //is BC9

    //===============================================================================================================================
    //Diff canvases
    //===============================================================================================================================
    TCanvas *c1_Diff_ADC[3], *c1_Diff_TOT[3], *c1_Diff_TcTp1[3], *c1_Diff_TcTp2[3];
    for(int ichip=0;ichip<3;ichip++){
      c1_Diff_ADC[ichip] = new TCanvas(Form("c1_Diff_Comp_ADC_chip_%d",ichip),Form("c1_Diff_Comp_ADC_chip_%d",ichip));
      c1_Diff_ADC[ichip]->Divide(4,4);
      for(int ipad=0;ipad<16;ipad++){
	c1_Diff_ADC[ichip]->cd(ipad+1)->SetLogy();
	int ihist = 16*ichip + ipad;
	((TH1F *) list->FindObject(Form("hCompressDiffTCADC_%d",ihist)))->Draw();
      }
    }
    for(int ichip=0;ichip<3;ichip++){
      c1_Diff_TOT[ichip] = new TCanvas(Form("c1_Diff_Comp_TOT_chip_%d",ichip),Form("c1_Diff_Comp_TOT_chip_%d",ichip));
      c1_Diff_TOT[ichip]->Divide(4,4);
      for(int ipad=0;ipad<16;ipad++){
	c1_Diff_TOT[ichip]->cd(ipad+1)->SetLogy();
	int ihist = 16*ichip + ipad;
	((TH1F *) list->FindObject(Form("hCompressDiffTCTOT_%d",ihist)))->Draw();
      }
    }
    for(int ichip=0;ichip<3;ichip++){
      c1_Diff_TcTp1[ichip] = new TCanvas(Form("c1_Diff_Comp_TcTp1_chip_%d",ichip),Form("c1_Diff_Comp_TcTp1_chip_%d",ichip));
      c1_Diff_TcTp1[ichip]->Divide(4,4);
      for(int ipad=0;ipad<16;ipad++){
	c1_Diff_TcTp1[ichip]->cd(ipad+1)->SetLogy();
	int ihist = 16*ichip + ipad;
	((TH1F *) list->FindObject(Form("hCompressDiffTCTcTp1_%d",ihist)))->Draw();
      }
    }
    for(int ichip=0;ichip<3;ichip++){
      c1_Diff_TcTp2[ichip] = new TCanvas(Form("c1_Diff_Comp_TcTp2_chip_%d",ichip),Form("c1_Diff_Comp_TcTp2_chip_%d",ichip));
      c1_Diff_TcTp2[ichip]->Divide(4,4);
      for(int ipad=0;ipad<16;ipad++){
	c1_Diff_TcTp2[ichip]->cd(ipad+1)->SetLogy();
	int ihist = 16*ichip + ipad;
	((TH1F *) list->FindObject(Form("hCompressDiffTCTcTp1_%d",ihist)))->Draw();
      }
    }

        TCanvas *c1_CompE_ADC[3], *c1_CompE_TOT[3], *c1_CompE_TcTp1[3], *c1_CompE_TcTp2[3];
    for(int ichip=0;ichip<3;ichip++){
      c1_CompE_ADC[ichip] = new TCanvas(Form("c1_CompE_ADC_chip_%d",ichip),Form("c1_CompE_ADC_chip_%d",ichip));
      c1_CompE_ADC[ichip]->Divide(4,4);
      for(int ipad=0;ipad<16;ipad++){
	c1_CompE_ADC[ichip]->cd(ipad+1)->SetLogy();
	int ihist = 16*ichip + ipad;
	((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",ihist)))->Draw();
	((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",ihist)))->Draw("sames");
      }
    }
    for(int ichip=0;ichip<3;ichip++){
      c1_CompE_TOT[ichip] = new TCanvas(Form("c1_CompE_TOT_chip_%d",ichip),Form("c1_CompE_TOT_chip_%d",ichip));
      c1_CompE_TOT[ichip]->Divide(4,4);
      for(int ipad=0;ipad<16;ipad++){
	c1_CompE_TOT[ichip]->cd(ipad+1)->SetLogy();
	int ihist = 16*ichip + ipad;
	((TH1F *) list->FindObject(Form("hECONTTcTp3_%d",ihist)))->Draw();
	((TH1F *) list->FindObject(Form("hEmulTcTp3_%d",ihist)))->Draw("sames");
      }
    }
    for(int ichip=0;ichip<3;ichip++){
      c1_CompE_TcTp1[ichip] = new TCanvas(Form("c1_CompE_TcTp1_chip_%d",ichip),Form("c1_CompE_TcTp1_chip_%d",ichip));
      c1_CompE_TcTp1[ichip]->Divide(4,4);
      for(int ipad=0;ipad<16;ipad++){
	c1_CompE_TcTp1[ichip]->cd(ipad+1)->SetLogy();
	int ihist = 16*ichip + ipad;
	((TH1F *) list->FindObject(Form("hECONTTcTp1_%d",ihist)))->Draw();
	((TH1F *) list->FindObject(Form("hEmulTcTp1_%d",ihist)))->Draw("sames");
      }
    }
    for(int ichip=0;ichip<3;ichip++){
      c1_CompE_TcTp2[ichip] = new TCanvas(Form("c1_CompE_TcTp2_chip_%d",ichip),Form("c1_CompE_TcTp2_chip_%d",ichip));
      c1_CompE_TcTp2[ichip]->Divide(4,4);
      for(int ipad=0;ipad<16;ipad++){
	c1_CompE_TcTp2[ichip]->cd(ipad+1)->SetLogy();
	int ihist = 16*ichip + ipad;
	((TH1F *) list->FindObject(Form("hECONTTcTp2_%d",ihist)))->Draw();
	((TH1F *) list->FindObject(Form("hEmulTcTp2_%d",ihist)))->Draw("sames");
      }
    }
    
    int ref1TC = 35;
    TCanvas *c1_Diff_TC_ref1 = new TCanvas("c1_Diff_TC_ref1",Form("c1_Diff_TC_%d",ref1TC));
    c1_Diff_TC_ref1->Divide(2,2);
    c1_Diff_TC_ref1->cd(1)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffTCADC_%d",ref1TC)))->Draw();
    c1_Diff_TC_ref1->cd(2)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffTCTOT_%d",ref1TC)))->Draw();
    c1_Diff_TC_ref1->cd(3)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffTCTcTp1_%d",ref1TC)))->Draw();
    c1_Diff_TC_ref1->cd(4)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffTCTcTp2_%d",ref1TC)))->Draw();
    
    TCanvas *c1_Energy_TC_ref1 = new TCanvas("c1_Energy_TC_ref1",Form("c1_Energy_TC_%d",ref1TC));
    c1_Energy_TC_ref1->Divide(2,2);
    c1_Energy_TC_ref1->cd(1)->SetLogy();
    ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",ref1TC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",ref1TC)))->Draw("sames");
    c1_Energy_TC_ref1->cd(2)->SetLogy(); 
    ((TH1F *) list->FindObject(Form("hECONTTcTp3_%d",ref1TC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp3_%d",ref1TC)))->Draw("sames");
    c1_Energy_TC_ref1->cd(3)->SetLogy();
    ((TH1F *) list->FindObject(Form("hECONTTcTp1_%d",ref1TC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp1_%d",ref1TC)))->Draw("sames");
    c1_Energy_TC_ref1->cd(4)->SetLogy(); 
    ((TH1F *) list->FindObject(Form("hECONTTcTp2_%d",ref1TC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp2_%d",ref1TC)))->Draw("sames");

    int ref2TC = 0;
    TCanvas *c1_Diff_TC_ref2 = new TCanvas("c1_Diff_TC_ref2",Form("c1_Diff_TC_%d",ref2TC));
    c1_Diff_TC_ref2->Divide(2,2);
    c1_Diff_TC_ref2->cd(1)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffTCADC_%d",ref2TC)))->Draw();
    c1_Diff_TC_ref2->cd(2)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffTCTOT_%d",ref2TC)))->Draw();
    c1_Diff_TC_ref2->cd(3)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffTCTcTp1_%d",ref2TC)))->Draw();
    c1_Diff_TC_ref2->cd(4)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffTCTcTp2_%d",ref2TC)))->Draw();
    
    TCanvas *c1_Energy_TC_ref2 = new TCanvas("c1_Energy_TC_ref2",Form("c1_Energy_TC_%d",ref2TC));
    c1_Energy_TC_ref2->Divide(2,2);
    c1_Energy_TC_ref2->cd(1)->SetLogy();
    ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",ref2TC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",ref2TC)))->Draw("sames");
    c1_Energy_TC_ref2->cd(2)->SetLogy(); 
    ((TH1F *) list->FindObject(Form("hECONTTcTp3_%d",ref2TC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp3_%d",ref2TC)))->Draw("sames");
    c1_Energy_TC_ref2->cd(3)->SetLogy();
    ((TH1F *) list->FindObject(Form("hECONTTcTp1_%d",ref2TC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp1_%d",ref2TC)))->Draw("sames");
    c1_Energy_TC_ref2->cd(4)->SetLogy(); 
    ((TH1F *) list->FindObject(Form("hECONTTcTp2_%d",ref2TC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp2_%d",ref2TC)))->Draw("sames");


    //===============================================================================================================================
    //Save histograms
    //===============================================================================================================================
    fout->cd();
    dir_diff->Write();
    c1_Energy_TC_ref1->Write();
    c1_Diff_TC_ref1->Write();
    c1_Energy_TC_ref2->Write();
    c1_Diff_TC_ref2->Write();
    c1_Energy_TC_ref2->Write();
    c1_Diff_TC_ref2->Write();
    for(int ichip=0;ichip<3;ichip++) c1_CompE_ADC[ichip]->Write();
    for(int ichip=0;ichip<3;ichip++) c1_CompE_TOT[ichip]->Write();
    for(int ichip=0;ichip<3;ichip++) c1_CompE_TcTp1[ichip]->Write();
    for(int ichip=0;ichip<3;ichip++) c1_CompE_TcTp2[ichip]->Write();
    for(int ichip=0;ichip<3;ichip++) c1_Diff_ADC[ichip]->Write();
    for(int ichip=0;ichip<3;ichip++) c1_Diff_TOT[ichip]->Write();
    for(int ichip=0;ichip<3;ichip++) c1_Diff_TcTp1[ichip]->Write();
    for(int ichip=0;ichip<3;ichip++) c1_Diff_TcTp2[ichip]->Write();
    dir_charge->Write();
    fout->Close();
    //===============================================================================================================================

  }else{

    //===============================================================================================================================
    //Diff canvases
    //===============================================================================================================================
    
    TCanvas *c1_Diff_ADC = new TCanvas(Form("c1_Diff_Comp_ADC"),Form("c1_Diff_Comp_ADC"));
    c1_Diff_ADC->Divide(4,3);
    for(int istc=0;istc<12;istc++){
      c1_Diff_ADC->cd(istc+1)->SetLogy();
      ((TH1F *) list->FindObject(Form("hCompressDiffSTCADC_%d",istc)))->Draw();
    }
    TCanvas *c1_Diff_TOT = new TCanvas(Form("c1_Diff_Comp_TOT"),Form("c1_Diff_Comp_TOT"));
    c1_Diff_TOT->Divide(4,3);
    for(int istc=0;istc<12;istc++){
      c1_Diff_TOT->cd(istc+1)->SetLogy();
      ((TH1F *) list->FindObject(Form("hCompressDiffSTCTOT_%d",istc)))->Draw();
    }    
    TCanvas *c1_Diff_TcTp1 = new TCanvas(Form("c1_Diff_Comp_TcTp1"),Form("c1_Diff_Comp_TcTp1"));
    c1_Diff_TcTp1->Divide(4,3);
    for(int istc=0;istc<12;istc++){
      c1_Diff_TcTp1->cd(istc+1)->SetLogy();
      ((TH1F *) list->FindObject(Form("hCompressDiffSTCTcTp1_%d",istc)))->Draw();
    }
    TCanvas *c1_Diff_TcTp2 = new TCanvas(Form("c1_Diff_Comp_TcTp2"),Form("c1_Diff_Comp_TcTp2"));
    c1_Diff_TcTp2->Divide(4,3);
    for(int istc=0;istc<12;istc++){
      c1_Diff_TcTp2->cd(istc+1)->SetLogy();
      ((TH1F *) list->FindObject(Form("hCompressDiffSTCTcTp2_%d",istc)))->Draw();
    }    

    TCanvas *c1_CompE_ADC = new TCanvas(Form("c1_CompE_ADC"),Form("c1_CompE_ADC"));
    c1_CompE_ADC->Divide(4,3);
    for(int istc=0;istc<12;istc++){
      c1_CompE_ADC->cd(istc+1)->SetLogy();
      ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",istc)))->Draw();
      ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",istc)))->Draw("sames");
    }
    TCanvas *c1_CompE_TOT = new TCanvas(Form("c1_CompE_TOT"),Form("c1_CompE_TOT"));
    c1_CompE_TOT->Divide(4,3);
    for(int istc=0;istc<12;istc++){
      c1_CompE_TOT->cd(istc+1)->SetLogy();
      ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",istc)))->Draw();
      ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",istc)))->Draw("sames");
    }    
    TCanvas *c1_CompE_TcTp1 = new TCanvas(Form("c1_CompE_TcTp1"),Form("c1_CompE_TcTp1"));
    c1_CompE_TcTp1->Divide(4,3);
    for(int istc=0;istc<12;istc++){
      c1_CompE_TcTp1->cd(istc+1)->SetLogy();
      ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",istc)))->Draw();
      ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",istc)))->Draw("sames");
    }
    TCanvas *c1_CompE_TcTp2 = new TCanvas(Form("c1_CompE_TcTp2"),Form("c1_CompE_TcTp2"));
    c1_CompE_TcTp2->Divide(4,3);
    for(int istc=0;istc<12;istc++){
      c1_CompE_TcTp2->cd(istc+1)->SetLogy();
      ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",istc)))->Draw();
      ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",istc)))->Draw("sames");
    }    

    int ref1STC = 6;
    TCanvas *c1_Diff_STC_ref1 = new TCanvas("c1_Diff_STC_ref1",Form("c1_Diff_STC_%d",ref1STC));
    c1_Diff_STC_ref1->Divide(2,2);
    c1_Diff_STC_ref1->cd(1)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffSTCADC_%d",ref1STC)))->Draw();
    c1_Diff_STC_ref1->cd(2)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffSTCTOT_%d",ref1STC)))->Draw();
    c1_Diff_STC_ref1->cd(3)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffSTCTcTp1_%d",ref1STC)))->Draw();
    c1_Diff_STC_ref1->cd(4)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffSTCTcTp2_%d",ref1STC)))->Draw();
    
    TCanvas *c1_Energy_STC_ref1 = new TCanvas("c1_Energy_STC_ref1",Form("c1_Energy_STC_%d",ref1STC));
    c1_Energy_STC_ref1->Divide(2,2);
    c1_Energy_STC_ref1->cd(1)->SetLogy();
    ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",ref1STC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",ref1STC)))->Draw("sames");
    c1_Energy_STC_ref1->cd(2)->SetLogy(); 
    ((TH1F *) list->FindObject(Form("hECONTTcTp3_%d",ref1STC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp3_%d",ref1STC)))->Draw("sames");
    c1_Energy_STC_ref1->cd(3)->SetLogy();
    ((TH1F *) list->FindObject(Form("hECONTTcTp1_%d",ref1STC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp1_%d",ref1STC)))->Draw("sames");
    c1_Energy_STC_ref1->cd(4)->SetLogy(); 
    ((TH1F *) list->FindObject(Form("hECONTTcTp2_%d",ref1STC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp2_%d",ref1STC)))->Draw("sames");

    int ref2STC = 0;
    TCanvas *c1_Diff_STC_ref2 = new TCanvas("c1_Diff_STC_ref2",Form("c1_Diff_STC_%d",ref2STC));
    c1_Diff_STC_ref2->Divide(2,2);
    c1_Diff_STC_ref2->cd(1)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffSTCADC_%d",ref2STC)))->Draw();
    c1_Diff_STC_ref2->cd(2)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffSTCTOT_%d",ref2STC)))->Draw();
    c1_Diff_STC_ref2->cd(3)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffSTCTcTp1_%d",ref2STC)))->Draw();
    c1_Diff_STC_ref2->cd(4)->SetLogy(); ((TH1F *) list->FindObject(Form("hCompressDiffSTCTcTp2_%d",ref2STC)))->Draw();
    
    TCanvas *c1_Energy_STC_ref2 = new TCanvas("c1_Energy_STC_ref2",Form("c1_Energy_STC_%d",ref2STC));
    c1_Energy_STC_ref2->Divide(2,2);
    c1_Energy_STC_ref2->cd(1)->SetLogy();
    ((TH1F *) list->FindObject(Form("hECONTTcTp0_%d",ref2STC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp0_%d",ref2STC)))->Draw("sames");
    c1_Energy_STC_ref2->cd(2)->SetLogy(); 
    ((TH1F *) list->FindObject(Form("hECONTTcTp3_%d",ref2STC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp3_%d",ref2STC)))->Draw("sames");
    c1_Energy_STC_ref2->cd(3)->SetLogy();
    ((TH1F *) list->FindObject(Form("hECONTTcTp1_%d",ref2STC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp1_%d",ref2STC)))->Draw("sames");
    c1_Energy_STC_ref2->cd(4)->SetLogy(); 
    ((TH1F *) list->FindObject(Form("hECONTTcTp2_%d",ref2STC)))->Draw();
    ((TH1F *) list->FindObject(Form("hEmulTcTp2_%d",ref2STC)))->Draw("sames");

    //===============================================================================================================================

    //===============================================================================================================================
    //Save histograms
    //===============================================================================================================================
    fout->cd();
    dir_diff->Write();

    c1_Energy_STC_ref1->Write();
    c1_Diff_STC_ref1->Write();
    c1_Energy_STC_ref2->Write();
    c1_Diff_STC_ref2->Write();
    c1_Energy_STC_ref2->Write();
    c1_Diff_STC_ref2->Write();

    c1_CompE_ADC->Write();
    c1_CompE_TOT->Write();
    c1_CompE_TcTp1->Write();
    c1_CompE_TcTp2->Write();

    c1_Diff_ADC->Write();
    c1_Diff_TOT->Write();
    c1_Diff_TcTp1->Write();
    c1_Diff_TcTp2->Write();

    dir_charge->Write();
    fout->Close();
    //===============================================================================================================================

  }//isSTC4
  
}
