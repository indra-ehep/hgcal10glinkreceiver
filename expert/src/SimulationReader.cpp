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
  TProfile *hPedPro=new TProfile("PedPro",";Channel;Average",37,-0.5,36.5);

  TH1D *hPed=new TH1D("Ped",";Channel;Average",37,-0.5,36.5);
  TH1D *hPedSig=new TH1D("PedSig",";Channel;RMS",37,-0.5,36.5);
  TH1D *hCorrPed=new TH1D("CorrPed",";Channel;Average",37,-0.5,36.5);
  TH1D *hCorrPedSig=new TH1D("CorrPedSig",";Channel;RMS",37,-0.5,36.5);
  TH2D *hCmCorr=new TH2D("CmCorr",";CM0;CM1;Number",41,79.5,120.5,41,79.5,120.5);

  Average avg[2][6][37][2];
  
  // Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;

  // Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);

  // Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);

  // Storage for previous ADC values
  uint64_t bx,previousBx;
  uint16_t adcM[2][6][37];
  
  // Defaults to the files being in directory "dat"
  // Can call setDirectory("blah") to change this
  //_fileReader.setDirectory("somewhere/else");
  _fileReader.openRun(runNumber,0);

  unsigned nEvents(0);

  while(_fileReader.read(r)) {
    if(     r->state()==Hgcal10gLinkReceiver::FsmState::Starting) {
      rStart->print();
      std::cout << std::endl;

    } else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping) {
      rStop->print();
      std::cout << std::endl;

    } else {

      // We have an event record
      nEvents++;

      bool print(nEvents<=2);

      if(print) {
	rEvent->print();
	std::cout << std::endl;
      }

      // Check id is correct
      if(!rEvent->valid()) rEvent->print();

      // Access the Slink header ("begin-of-event")
      // This should always be present; check pattern is correct
      const Hgcal10gLinkReceiver::SlinkBoe *b(rEvent->slinkBoe());
      assert(b!=nullptr);
      if(!b->validPattern()) b->print();
      
      // Access the Slink trailer ("end-of-event")
      // This should always be present; check pattern is correct
      const Hgcal10gLinkReceiver::SlinkEoe *e(rEvent->slinkEoe());
      assert(e!=nullptr);
      if(!e->validPattern()) e->print();

      bx=3564*e->orbitId()+e->bxId();
      if(nEvents>1) {
	hDbx->Fill(bx-previousBx);
      }
      
	// Access the BE packet header
      const Hgcal10gLinkReceiver::BePacketHeader *bph(rEvent->bePacketHeader());
      if(bph!=nullptr && print) bph->print();
      
      // Access ECON-D packet
      const uint32_t *pData(rEvent->econdPayload());
      
      // Check this is not an empty event
      if(pData!=nullptr) {

	if(print) {
	  std::cout << "Words of ECON-D packet" << std::endl;
	  std::cout << std::hex << std::setfill('0');
	  for(unsigned i(0);i<2*(rEvent->payloadLength()-5);i++) {
	    std::cout << std::setw(3) << i << " 0x" << std::setw(8) << pData[i] << std::endl;
	  }
	  std::cout << std::dec << std::setfill(' ');
	  std::cout << std::endl;

	  const Hgcal10gLinkReceiver::EcondHeader *pEcond((const Hgcal10gLinkReceiver::EcondHeader*)rEvent->econdPayload());
	  pEcond->print();
	  for(unsigned i(0);i<3;i++) {
	    const Hgcal10gLinkReceiver::EcondSubHeader *pEsh((const Hgcal10gLinkReceiver::EcondSubHeader*)(pEcond+1+39*i));
	    pEsh->print();
	  }
	  std::cout << std::endl;
	}
	
	const Hgcal10gLinkReceiver::EcondSubHeader *pEsh((const Hgcal10gLinkReceiver::EcondSubHeader*)(pData+2));
	for(unsigned k(0);k<37;k++) {
	  const HgcrocWord *hw((const HgcrocWord*)(pData+4+k));
	  if(bx==previousBx+1) {
	    assert(hw->adcM()==adcM[0][0][k]);
	  }
	  
	  adcM[0][0][k]=hw->adc();

	  hPedPro->Fill(k,hw->adc());
	  avg[0][0][k][0]+=hw->adc();
	  avg[0][0][k][1]+=hw->adc()-0.5*(pEsh->commonMode(0)+pEsh->commonMode(1));
	  hCmCorr->Fill(pEsh->commonMode(0),pEsh->commonMode(1));
	}

	// Do other stuff here

      }

      previousBx=bx;
    }
  }

	for(unsigned k(0);k<37;k++) {
	  hPed->SetBinContent(k+1,avg[0][0][k][0].average());
	  hPed->SetBinError(k+1,avg[0][0][k][0].errorOnAverage());
	  hPedSig->SetBinContent(k+1,avg[0][0][k][0].sigma());
	  hPedSig->SetBinError(k+1,avg[0][0][k][0].errorOnSigma());

	  hCorrPed->SetBinContent(k+1,avg[0][0][k][1].average());
	  hCorrPed->SetBinError(k+1,avg[0][0][k][1].errorOnAverage());
	  hCorrPedSig->SetBinContent(k+1,avg[0][0][k][1].sigma());
	  hCorrPedSig->SetBinError(k+1,avg[0][0][k][1].errorOnSigma());
	}
  
  
  std::cout << "Total number of event records seen = "
	    << nEvents << std::endl;

  delete r;

  return 0;
}
