#include <iostream>
#include <iomanip>
#include <cassert>

#include "TRandom3.h"

#include "SystemParameters.h"
#include "FileWriter.h"
#include "RecordPrinter.h"
#include "EcondHeader.h"
#include "HgcrocHalf.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char** argv) {
  if(argc<2) {
    std::cerr << argv[0] << ": no run number specified" << std::endl;
    return 1;
  }

  TRandom3 rndm;
  HgcrocHalf hHalf[2][6];

  for(unsigned i(0);i<2;i++) {
    for(unsigned j(0);j<6;j++) {
      hHalf[i][j].initialize(&rndm);
    }
  }  
  
  // 32-bit words = 2 + 6 x 39 + 1 = 237
  // 64-bit words = 1 + 3 x 39 + 0.5 = 118.5
  uint64_t econdP64[119]={
      0xf33af0000052eca6,
      0xe000001fffffffff,
      0x0000000000100000,
      0x0020000000300000,
      0x0040000000500000,
      0x0060000000700000,
      0x0080000000900000,
      0x00a0000000b00000,
      0x00c0000000d00000,
      0x00e0000000f00000,
      0x0100000001100000,
      0x0000000001200000,
      0x0130000001400000,
      0x0150000001600000,
      0x0170000001800000,
      0x0190000001a00000,
      0x01b0000001c00000,
      0x01d0000001e00000,
      0x01f0000002000000,
      0x0210000002200000,
      0x02300000e000001f,
      0xffffffff02400000,
      0x0250000002600000,
      0x0270000002800000,
      0x0290000002a00000,
      0x02b0000002c00000,
      0x02d0000002e00000,
      0x02f0000003000000,
      0x0310000003200000,
      0x0330000003400000,
      0x0350000000000000,
      0x0360000003700000,
      0x0380000003900000,
      0x03a0000003b00000,
      0x03c0000003d00000,
      0x03e0000003f00000,
      0x0400000004100000,
      0x0420000004300000,
      0x0440000004500000,
      0x0460000004700000,
      0xe000001fffffffff,
      0x0000000000100000,
      0x0020000000300000,
      0x0040000000500000,
      0x0060000000700000,
      0x0080000000900000,
      0x00a0000000b00000,
      0x00c0000000d00000,
      0x00e0000000f00000,
      0x0100000001100000,
      0x0000000001200000,
      0x0130000001400000,
      0x0150000001600000,
      0x0170000001800000,
      0x0190000001a00000,
      0x01b0000001c00000,
      0x01d0000001e00000,
      0x01f0000002000000,
      0x0210000002200000,
      0x02300000e000001f,
      0xffffffff02400000,
      0x0250000002600000,
      0x0270000002800000,
      0x0290000002a00000,
      0x02b0000002c00000,
      0x02d0000002e00000,
      0x02f0000003000000,
      0x0310000003200000,
      0x0330000003400000,
      0x0350000000000000,
      0x0360000003700000,
      0x0380000003900000,
      0x03a0000003b00000,
      0x03c0000003d00000,
      0x03e0000003f00000,
      0x0400000004100000,
      0x0420000004300000,
      0x0440000004500000,
      0x0460000004700000,
      0xe000001fffffffff,
      0x0000000000100000,
      0x0020000000300000,
      0x0040000000500000,
      0x0060000000700000,
      0x0080000000900000,
      0x00a0000000b00000,
      0x00c0000000d00000,
      0x00e0000000f00000,
      0x0100000001100000,
      0x0000000001200000,
      0x0130000001400000,
      0x0150000001600000,
      0x0170000001800000,
      0x0190000001a00000,
      0x01b0000001c00000,
      0x01d0000001e00000,
      0x01f0000002000000,
      0x0210000002200000,
      0x02300000e000001f,
      0xffffffff02400000,
      0x0250000002600000,
      0x0270000002800000,
      0x0290000002a00000,
      0x02b0000002c00000,
      0x02d0000002e00000,
      0x02f0000003000000,
      0x0310000003200000,
      0x0330000003400000,
      0x0350000000000000,
      0x0360000003700000,
      0x0380000003900000,
      0x03a0000003b00000,
      0x03c0000003d00000,
      0x03e0000003f00000,
      0x0400000004100000,
      0x0420000004300000,
      0x0440000004500000,
      0x0460000004700000,
      0xe4e37d0c00000000
      };

  uint32_t *econdP32((uint32_t*)econdP64);
  uint32_t econdPacket[237*2];
  
  for(unsigned i(0);i<119;i++) {
    econdPacket[2*i    ]=econdP32[2*i+1];
    if(i<118) econdPacket[2*i+  1]=econdP32[2*i  ];
    econdPacket[2*i+237]=econdP32[2*i+1];
    if(i<118) econdPacket[2*i+238]=econdP32[2*i  ];
  }

  for(unsigned i(0);i<2;i++) {

  EcondHeader *eh((EcondHeader*)(econdPacket+237*i));
    eh->print();

    for(unsigned j(0);j<6;j++) {
      EcondSubHeader *esh((EcondSubHeader*)(econdPacket+2+237*i+39*j));
      esh->print();
      for(unsigned k(0);k<37;k++) {
	HgcrocWord *hw((HgcrocWord*)(econdPacket+2+237*i+39*j+2+k));
	hw->print();
      }
    }

    std::cout << " CRC = " << econdPacket[2+237*i+39*6] << std::endl;
  }
  
  // Handle run number
  unsigned runNumber(time(0));
  /*
  std::istringstream issRun(argv[1]);
  issRun >> runNumber;

  if(runNumber==0) {
    std::cerr << argv[0] << ": run number uninterpretable" << std::endl;
    return 2;
  }
  */
  
  // Create the file reader
  Hgcal10gLinkReceiver::FileWriter _fileWriter;

  // Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);

  Hgcal10gLinkReceiver::RecordStarting rPermanent;
  
  // Set up specific records to interpet the formats
  Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);

  _fileWriter.openRun(runNumber,0);

  unsigned burst(3);
  
  rStart->setHeader();
  rStart->setRunNumber(runNumber);  
  //rStart->setMaxEvents(1000000000);
  rStart->setMaxEvents(1000000*burst);
  rStart->setMaxSeconds(10);
  rStart->setMaxSpills(0);
  rStart->print();

  _fileWriter.write(r);

  rPermanent=*rStart;
  
  unsigned nEvents;
  unsigned bx(0);
  
  for(nEvents=0;nEvents<rPermanent.maxEvents() && bx<40000000*rPermanent.maxSeconds();) {

    // Get FE counters
    //double dBx(4000.0*log(1.0/(1.0-rndm.Uniform())));
    double dBx(burst+rndm.Exp(3999.0-burst));
    bx+=unsigned(dBx);

    for(unsigned l1a(0);l1a<burst;l1a++,nEvents++) {
    rEvent->setHeader(nEvents+1);
    bx++;
    
    unsigned bc(((bx+16)%3564)+1);
    unsigned oc(((bx+16)/3564)%8);
    unsigned ec((nEvents+1)%64);
  
    SlinkBoe *sb(rEvent->getSlinkBoe());
    sb->reset();
    sb->setEventId(nEvents+1);
    sb->setL1aType(2);
    sb->setL1aSubType(0);
    sb->setSourceId(ProcessorDaqLink0FifoShmKey);
    
    //rEvent->print();

    rEvent->setPayloadLength(6);

    BePacketHeader *bph(rEvent->getBePacketHeader());
    bph->reset();
    bph->setBunchCounter(bc);
    bph->setEventCounter(ec);
    bph->setOrbitCounter(oc);
    bph->setEcondStatus(0,0);
    bph->setEcondStatus(3,0);
    
    rEvent->setPayloadLength(5+237);
    uint32_t *ep(rEvent->getEcondPayload());
    std::memcpy(ep,econdPacket,8*237);
    
    for(unsigned i(0);i<2;i++) {
      EcondHeader *eh((EcondHeader*)(ep+237*i));
      eh->reset();
      eh->setBx(bc);
      eh->setEvent(ec);
      eh->setOrbit(oc);
      eh->setPassthroughMode(true);
      eh->setPacketWords(235);
      eh->setCrc();
      //eh->print();

      for(unsigned j(0);j<6;j++) {
	hHalf[i][j].simulate(l1a==0);
      
	EcondSubHeader *esh((EcondSubHeader*)(ep+237*i+2+39*j));
	esh->setCommonMode(0,hHalf[i][j].commonMode(0));
	esh->setCommonMode(1,hHalf[i][j].commonMode(1));
	//esh->print();
	std::memcpy(esh+1,hHalf[i][j].hgcrocWords(),4*37);
	//for(unsigned k(0);k<37;k++) {
	//  ((HgcrocWord*)(ep+237*i+2+39*j+2+k))->print();
	//}
      }
    }
    
    SlinkEoe *se(rEvent->getSlinkEoe());
    se->reset();
    se->setEventLength(242);
    se->setDaqCrc(0xdead);
    se->setBxId((bx%3564)+1);
    se->setOrbitId(bx/3564);
    se->setCrc(0xdead);
    se->setStatus(0);

    if(nEvents<10) rEvent->print();
    
    _fileWriter.write(r);
    }
  }
  
  rStop->setHeader();
  rStop->setRunNumber(runNumber);
  rStop->setUtc(runNumber+(bx/40000000));
  rStop->setNumberOfEvents(nEvents);
  rStop->setNumberOfSeconds(bx/40000000);
  rStop->setNumberOfSpills(0);
  rStop->setNumberOfPauses(0);
  rStop->print();

  _fileWriter.write(r);


  /*  
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
      const uint32_t *pEcond(rEvent->econdPayload());
      
      // Check this is not an empty event
      if(pEcond!=nullptr) {

	if(print) {
	  std::cout << "First 10 words of ECON-D packet" << std::endl;
	  std::cout << std::hex << std::setfill('0');
	  for(unsigned i(0);i<10;i++) {
	    std::cout << "0x" << std::setw(8) << pEcond[i] << std::endl;
	  }
	  std::cout << std::dec << std::setfill(' ');
	  std::cout << std::endl;
	}

	// Do other stuff here

      }
    }
  }
  */

  std::cout << "Total number of event records seen = "
	    << nEvents << std::endl;

  _fileWriter.close();

  delete r;

  return 0;
}
