#include <iostream>
#include <iomanip>
#include <cassert>

#include "TH1I.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TFileHandler.h"

#include "FileReader.h"
#include "EcondHeader.h"
#include "HgcrocWord.h"
#include "Average.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char** argv) {
  if(argc<2) {
    std::cerr << argv[0] << ": no run number specified" << std::endl;
    return 1;
  }

  // Handle run number
  unsigned runNumber(0);
  std::istringstream issRun(argv[1]);
  issRun >> runNumber;

  if(runNumber==0) {
    std::cerr << argv[0] << ": run number uninterpretable" << std::endl;
    return 2;
  }

  TFileHandler tfh(std::string("ExampleRunReader")+argv[1]);
  TH1I *hDbx=new TH1I("Dbx",";Difference in BX;Number",100,0,100000);
  TProfile *hPedPro=new TProfile("PedPro",";Channel;Average",80,-0.5,79.5);

  TH1D *hPed[80];
  TH1D *hPedSig[80];
  TH2D *hPed2D;
  TH2D *hPedSig2D;
  
  hPed2D=new TH2D("Ped2D",";Channel;CalPulseInt delay;Average",
		  80,-0.5,79.5,50,19.5,69.5);
  hPedSig2D=new TH2D("PedSig2D",";Channel;CalPulseInt delay;RMS",
		  80,-0.5,79.5,50,19.5,69.5);

  for(unsigned k(0);k<80;k++) {
    std::ostringstream sout;
    sout << "Ped" << std::setfill('0') << std::setw(2) << k;
    hPed[k]=new TH1D(sout.str().c_str(),";CalPulseInt delay;Average",50,19.5,69.5);

    sout << "Sig";
    hPedSig[k]=new TH1D(sout.str().c_str(),";CalPulseInt delay;RMS",50,19.5,69.5);
  }
  //TH1D *hCorrPed=new TH1D("CorrPed",";Channel;Average",80,-0.5,79.5);
  //TH1D *hCorrPedSig=new TH1D("CorrPedSig",";Channel;RMS",80,-0.5,79.5);
  //TH2D *hCmCorr=new TH2D("CmCorr",";CM0;CM1;Number",41,79.5,120.5,41,79.5,120.5);

  Average avg[50][80];
  
  // Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;

  // Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);

  // Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  
  unsigned nEvents(0);
  unsigned cpDelay(0);
  unsigned configurationBCounter(0);

  // Defaults to the files being in directory "dat"
  // Can call setDirectory("blah") to change this
  //_fileReader.setDirectory("somewhere/else");
  for(unsigned relay(0);relay<50;relay++) {
    _fileReader.openRun(runNumber+30*relay,0);

    cpDelay=(relay%50);

    while(_fileReader.read(r)) {
    if(     r->state()==Hgcal10gLinkReceiver::FsmState::Starting) {
      rStart->print();
      std::cout << std::endl;

    } else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping) {
      rStop->print();
      std::cout << std::endl;

    } else if(r->state()==Hgcal10gLinkReceiver::FsmState::ConfiguringB) {
      cpDelay=(configurationBCounter%50);
      configurationBCounter++;
      std::cout << std::endl;

    } else {

      // We have an event record
      nEvents++;

      bool print(nEvents<=1);

      if(print) {
	rEvent->print();
	std::cout << std::endl;
      }

      // Check id is correct
      if(!rEvent->valid()) rEvent->print();

      // Access payload
      const uint32_t *pData((const uint32_t*)(rEvent->payload()));
      
      // Check this is not an empty event
      if(pData!=nullptr) {

	if(print) {
	  std::cout << "Words of payload" << std::endl;
	  std::cout << std::hex << std::setfill('0');
	  for(unsigned i(0);i<2*(rEvent->payloadLength());i++) {
	    std::cout << std::setw(3) << i << " 0x" << std::setw(8) << pData[i] << std::endl;
	  }
	  std::cout << std::dec << std::setfill(' ');
	  std::cout << std::endl;
	}
      	
	for(unsigned k(0);k<80;k++) {
	  const HgcrocWord *hw((const HgcrocWord*)(pData+k));
	  if(print) hw->print();

	  avg[cpDelay][k]+=hw->adcM();

	  //hPedPro->Fill(k,hw->adc());
	  //hCmCorr->Fill(pEsh->commonMode(0),pEsh->commonMode(1));
	}

	// Do other stuff here

      }
    }
  }
  _fileReader.close();
  }
  
  for(unsigned j(0);j<50;j++) {
    for(unsigned k(0);k<80;k++) {
      if(k!=0 && k!=1 && k!=39 && k!=40 && k!=41 && k!=79) {
	hPed[k]->SetBinContent(j+1,avg[j][k].average());
	hPed[k]->SetBinError(j+1,avg[j][k].errorOnAverage());
	hPedSig[k]->SetBinContent(j+1,avg[j][k].sigma());
	hPedSig[k]->SetBinError(j+1,avg[j][k].errorOnSigma());
	
	hPed2D->SetBinContent(k+1,j+1,avg[j][k].average());
	hPedSig2D->SetBinContent(k+1,j+1,avg[j][k].sigma());
      }
    }
  }
  
  std::cout << "Total number of event records seen = "
	    << nEvents << std::endl;

  delete r;

  return 0;
}
