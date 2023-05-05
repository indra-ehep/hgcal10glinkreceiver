#include <iostream>
#include <iomanip>
#include <cassert>

#include "FileReader.h"
#include "EcondHeader.h"

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

  // Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;

  // Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);

  // Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);

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
      
      // Access the BE packet header
      const Hgcal10gLinkReceiver::BePacketHeader *bph(rEvent->bePacketHeader());
      if(bph!=nullptr && print) bph->print();
      
      // Access ECON-D packet
      const uint32_t *pData(rEvent->econdPayload());
      
      // Check this is not an empty event
      if(pData!=nullptr) {

	if(print) {
	  std::cout << "First 60 words of ECON-D packet" << std::endl;
	  std::cout << std::hex << std::setfill('0');
	  for(unsigned i(0);i<2*rEvent->payloadLength();i++) {
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
	
	// Do other stuff here

      }
    }
  }

  std::cout << "Total number of event records seen = "
	    << nEvents << std::endl;

  delete r;

  return 0;
}
