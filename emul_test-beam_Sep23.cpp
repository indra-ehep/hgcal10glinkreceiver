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

const long double maxEvent = 7e6; //6e5

//1395761; //iloop:3
//1710962; iloop:4
const uint64_t refPrE = 1710962; //iloop:3

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
    
    printf("iloop : %d, minEventTrig = %lu, maxEventTrig = %lu, minEventDAQ = %lu, maxEventDAQ = %lu\n",ieloop,minEventTrig, maxEventTrig, minEventDAQ, maxEventDAQ);

    //if(ieloop!=0) continue;
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
  
  moddata.clear();
  rocdata.clear();
  eventList.clear();
  econtarray.clear();
  econtemularray.clear();
  modarray.clear();
  hrocarray.clear();
  
  fout->cd();
  dir_diff->Write();
  dir_charge->Write();
  fout->Close();
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
      hCompressDiffTCADC[itc] = new TH1F(Form("hCompressDiffTCADC_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with totflag==0",itc), 200, -99, 101);
      hCompressDiffTCADC[itc]->SetMinimum(1.e-1);
      hCompressDiffTCADC[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffTCADC[itc]->SetLineColor(kRed);
      hCompressDiffTCADC[itc]->SetDirectory(dir_diff);
    }
    for(int itc=0;itc<48;itc++){
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
    TH1F *hEventCount = new TH1F("hEventCount","Event count", 11, -0.5, 10.5);
    hEventCount->SetMinimum(1.e-1);
    hEventCount->SetLineColor(kRed);
    hEventCount->GetXaxis()->SetBinLabel(2,"Total");
    hEventCount->GetXaxis()->SetBinLabel(3,"passed TcTp=0/3 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(4,"atleast a TcTp=1");
    hEventCount->GetXaxis()->SetBinLabel(5,"passed TcTp=1 (excl)");
    hEventCount->GetXaxis()->SetBinLabel(6,"TcTp=1/2");
    hEventCount->GetXaxis()->SetBinLabel(7,"atleast a TcTp=2");
    hEventCount->GetXaxis()->SetBinLabel(8,"passed TcTp=2 (excl)");
    hEventCount->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1 = new TH2F("hTCvsTcTp1","TC vs (Emulator - ECONT) for TcTp==1 (excl)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1->SetLineColor(kRed);
    hTCvsTcTp1->SetOption("box");
    hTCvsTcTp1->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp2 = new TH2F("hTCvsTcTp2","TC vs (Emulator - ECONT) for TcTp==2 (excl)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp2->GetXaxis()->SetTitle("TC");
    hTCvsTcTp2->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp2->SetLineColor(kBlue);
    hTCvsTcTp2->SetOption("box");
    hTCvsTcTp2->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1et2 = new TH2F("hTCvsTcTp1et2","TC vs (Emulator - ECONT) for (TcTp==1 and TcTp==2)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1et2->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1et2->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1et2->SetLineColor(kMagenta);
    hTCvsTcTp1et2->SetOption("box");
    hTCvsTcTp1et2->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1S = new TH2F("hTCvsTcTp1S","TC vs (Emulator - ECONT) for single TcTp==1 (excl)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1S->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1S->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1S->SetLineColor(kRed);
    hTCvsTcTp1S->SetOption("box");
    hTCvsTcTp1S->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1D = new TH2F("hTCvsTcTp1D","TC vs (Emulator - ECONT) for double TcTp==1 (excl)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1D->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1D->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1D->SetLineColor(kRed);
    hTCvsTcTp1D->SetOption("box");
    hTCvsTcTp1D->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1Sat = new TH2F("hTCvsTcTp1Sat","TC vs (Emulator - ECONT) for TcTp==1 (excl) and saturated ADC (excl)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1Sat->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1Sat->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1Sat->SetLineColor(kRed);
    hTCvsTcTp1Sat->SetOption("box");
    hTCvsTcTp1Sat->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1Ushoot = new TH2F("hTCvsTcTp1Ushoot","TC vs (Emulator - ECONT) for TcTp==1 (excl) and undershoot ADC (excl)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1Ushoot->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1Ushoot->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1Ushoot->SetLineColor(kRed);
    hTCvsTcTp1Ushoot->SetOption("box");
    hTCvsTcTp1Ushoot->SetDirectory(dir_diff);
    TH2F *hTCvsTcTp1NoSatUshoot = new TH2F("hTCvsTcTp1NoSatUshoot","TC vs (Emulator - ECONT) for TcTp==1 (excl) and NO saturated/undershoot ADC (excl)", 50, -1.5, 48.5, 200, -99, 101);
    hTCvsTcTp1NoSatUshoot->GetXaxis()->SetTitle("TC");
    hTCvsTcTp1NoSatUshoot->GetYaxis()->SetTitle("Difference in (Emulator - ECONT) compression");
    hTCvsTcTp1NoSatUshoot->SetLineColor(kRed);
    hTCvsTcTp1NoSatUshoot->SetOption("box");
    hTCvsTcTp1NoSatUshoot->SetDirectory(dir_diff);    
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

    
  }else{
    
    TH1F *hCompressDiffSTCADC[48],*hCompressDiffSTCTOT[48];
    for(int istc=0;istc<12;istc++){
      hCompressDiffSTCADC[istc] = new TH1F(Form("hCompressDiffSTCADC_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC4A : %d with totflag==0",istc), 200, -99, 101);
      hCompressDiffSTCADC[istc]->SetMinimum(1.e-1);
      hCompressDiffSTCADC[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
      hCompressDiffSTCADC[istc]->SetLineColor(kRed);
      hCompressDiffSTCADC[istc]->SetDirectory(dir_diff);
    }
    for(int istc=0;istc<12;istc++){
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
	uint32_t nofTcTp1 = 0, nofTcTp2 = 0, nofSat = 0, nofUndsht = 0;
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
	    const TPGFEDataformat::HalfHgcrocChannelData& chdata = rocdata.at(rocid).getChannelData(rocpin);
	    if(chdata.getTcTp()==1 or chdata.getTcTp()==2){
	      isTcTp12[itc] = true;
	      isModTcTp12 = true;
	      if(chdata.getTcTp()==1){
		((TH1F *) list->FindObject("hAbsRocpinTcTp1"))->Fill(float( tcch ));
		((TH1F *) list->FindObject("hSeqTcTp1"))->Fill(float( absseq ));
		if(!chdata.isTot() and chdata.getAdc()>1020) nofSat++;
		if(!chdata.isTot() and chdata.getAdc()<4) nofUndsht++;
		nofTcTp1++;
	      }else{
		nofTcTp2++;
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
	
	//////////////////////////////////////////////////////////////////////////////////
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
		  if(TMath::Abs(ediff)>0) std::cerr << "Event: "<<event<<" has problem for (ADC)TC channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		}else{
		  ((TH1F *) list->FindObject(Form("hCompressDiffTCTOT_%d",emch)))->Fill(float( ediff ));
		  if(ediff>=1) std::cerr << "Event: "<<event<<" has problem for (TOT)TC channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		}
	      }else{
		if(!isTot)
		  ((TH1F *) list->FindObject("hBC9TCMissedADC"))->Fill(float( channel[sorted_idx[itc]] ));
		else
		  ((TH1F *) list->FindObject("hBC9TCMissedTOT"))->Fill(float( channel[sorted_idx[itc]] ));
	      }//cdiff condn
	    }else{
	      //TcTp=1/2
	      if(nofTcTp1>0){
		if(nofTcTp2==0){
		  ((TH1F *) list->FindObject("hTCvsTcTp1"))->Fill(emch, ediff);
		  if(nofSat>0 and nofUndsht==0) ((TH1F *) list->FindObject("hTCvsTcTp1Sat"))->Fill(emch, ediff);
		  if(nofSat==0 and nofUndsht>0) ((TH1F *) list->FindObject("hTCvsTcTp1Ushoot"))->Fill(emch, ediff);
		  if(nofSat==0 and nofUndsht==0) {
		    ((TH1F *) list->FindObject("hTCvsTcTp1NoSatUshoot"))->Fill(emch, ediff);
		    //std::cerr << "Event: "<<event<<" has NoSatUshoot with channel: "<< emch << ", emul: "<<emen<<", econt: "<<energy[sorted_idx[itc]]<<std::endl;
		  }
		  if(nofTcTp1==1) ((TH1F *) list->FindObject("hTCvsTcTp1S"))->Fill(emch, ediff);
		  if(nofTcTp1==2) ((TH1F *) list->FindObject("hTCvsTcTp1D"))->Fill(emch, ediff);
		}else
		  ((TH1F *) list->FindObject("hTCvsTcTp1et2"))->Fill(emch, ediff);	
	      }//TcTp > 0
	      if(nofTcTp1==0 and nofTcTp2>0) ((TH1F *) list->FindObject("hTCvsTcTp2"))->Fill(emch, ediff);	
	    }//TcTp condition
	  }//tc loop
	  int moddiff = econtemulmodsum - econtmodsum;
	  ((TH1F *) list->FindObject("hEventCount"))->Fill(1);
	  if(!isModTcTp12){
	    if(!isTotMod){
	      ((TH1F *) list->FindObject("hModSumDiffADC"))->Fill(float( moddiff ));
	      if(TMath::Abs(moddiff)>0)
		std::cerr << "Event: "<<event<<" has problem in (ADC)modsum emul:"<<econtemulmodsum<<", econt : "<<econtmodsum<<std::endl;
	    }else
	      ((TH1F *) list->FindObject("hModSumDiffTOT"))->Fill(float( moddiff ));
	    ((TH1F *) list->FindObject("hEventCount"))->Fill(2);
	  }else{
	    ((TH1F *) list->FindObject("hModSumDiffTcTp12"))->Fill(float( moddiff ));
	    if(nofTcTp1>0){
	      ((TH1F *) list->FindObject("hEventCount"))->Fill(3);
	      if(nofTcTp2==0)
		((TH1F *) list->FindObject("hEventCount"))->Fill(4);
	      else
		((TH1F *) list->FindObject("hEventCount"))->Fill(5);
	    }
	    if(nofTcTp2>0){
	      ((TH1F *) list->FindObject("hEventCount"))->Fill(6);
	      if(nofTcTp1==0)
		((TH1F *) list->FindObject("hEventCount"))->Fill(7);
	      else
		((TH1F *) list->FindObject("hEventCount"))->Fill(8);
	    }
	    // hEventCount->GetXaxis()->SetBinLabel(3,"passed TcTp=0/3 (excl)");
	    // hEventCount->GetXaxis()->SetBinLabel(4,"atleast a TcTp=1");
	    // hEventCount->GetXaxis()->SetBinLabel(5,"passed TcTp=1 (excl)");
	    // hEventCount->GetXaxis()->SetBinLabel(6,"TcTp=1/2");
	    // hEventCount->GetXaxis()->SetBinLabel(7,"atleast a TcTp=2");
	    // hEventCount->GetXaxis()->SetBinLabel(8,"passed TcTp=2 (excl)");
	  }
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
	delete []isTcTp12;
      }//econt emulation loop
    }//econt data loop
    
  }//event loop
}
