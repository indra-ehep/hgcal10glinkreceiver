#ifndef ProcessorFastControlPlusDaq_h
#define ProcessorFastControlPlusDaq_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstring>

//#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "DataFifo.h"
#include "RecordPrinter.h"
#include "ShmKeys.h"
#include "SlinkBoe.h"
#include "SlinkEoe.h"

#include "ProcessorFastControl.h"

namespace Hgcal10gLinkReceiver {

  class ProcessorFastControlPlusDaq : public ProcessorFastControl {

  public:
    ProcessorFastControlPlusDaq() {
    }

    virtual ~ProcessorFastControlPlusDaq() {
    }

    void setUpAll(uint32_t rcKey, uint32_t fifoKey2,
                  uint32_t fifoKey0, uint32_t fifoKey1) {

      ShmSingleton< DataFifoT<6,1024> > shm0;
      shm0.setup(fifoKey0);
      ptrFifoShm0=shm0.payload();

      std::cout << "SHM HERE" << std::endl;

      ShmSingleton< DataFifoT<6,1024> > shm1;
      shm1.setup(fifoKey1);
      ptrFifoShm1=shm1.payload();
      //ptrFifoShm1=0;
      
      ProcessorFastControl::setUpAll(rcKey,fifoKey2);
    }

    bool readRxSummaryFile(unsigned nr=1) {
      const unsigned offset(27); // RX
      //const unsigned offset(28); // TX

      for(unsigned n(0);n<nr && n<28;n++) {
	
	std::ostringstream sout;
	sout << "rm data/rx_summary.txt; rm data/tx_summary.txt; source ./emp_capture_single.sh " << 128*n+1;
	system(sout.str().c_str());
      
	//system("ls -l data/");
	//system("rm data/rx_summary.txt; rm data/tx_summary.txt; source ./emp_capture_single.sh 1");
      //system("ls -l data/");
	
	std::ifstream fin;
	//fin.open("data/tx_summary.txt");
	fin.open("data/rx_summary.txt");
	if(!fin) return false;
	
	char buffer[1024];
	uint64_t value;
	
	fin.getline(buffer,1024);
	fin.getline(buffer,1024);
	fin.getline(buffer,1024);
	fin.getline(buffer,1024);
      
	std::cout << "rxSummary data" << std::endl;
	for(unsigned i(0);i<128;i++) {
	  for(unsigned j(0);j<8;j++) {
	    fin.getline(buffer,1024);
	    buffer[offset+8]='\0';
	    std::istringstream sin(buffer+offset);
	    sin >> std::hex >> value;

	    if(i>0 || n>0) _rxSummaryData[j][i-1+128*n]|=value;
	    _rxSummaryData[j][i+128*n]=(value<<32);
	  }
	}
	fin.close();
      }

      for(unsigned i(0);i<128*nr;i++) {
	for(unsigned j(0);j<8;j++) {
	  std::cout << " 0x" << std::hex << std::setfill('0') 
		    << std::setw(8) << _rxSummaryData[j][i]
		    << std::dec << std::setfill(' ');
	}
	std::cout << std::endl;
      }      
      
      return true;
    }

    void running() {
      _serenityUhal.uhalWrite("payload.fc_ctrl.fpga_fc.ctrl.tts",1);

      while(_ptrFsmInterface->isIdle()) {

	if(!ptrFifoShm2->backPressure()) {
	  _eventNumberInRun++;
	  uint64_t bxNumberInRun(1637*_eventNumberInRun);
	  uint64_t bc((bxNumberInRun%3564)+1);
	  uint64_t oc(bxNumberInRun/3564);

	  bool hgcrocData(true);

	  if(hgcrocData) readRxSummaryFile(1);

	  
	  uint64_t miniDaq[120]={
	    0xfe00b34ffffffffb,
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

	  miniDaq[0]&=0xfe00000fffffffff;
	  miniDaq[0]|=bc<<45|uint64_t(_eventNumberInRun%64)<<39|(oc%8)<<36;

	  uint32_t *p32((uint32_t*)miniDaq);





	  RecordRunning *r;
	  while((r=(RecordRunning*)(ptrFifoShm0->getWriteRecord()))==nullptr) usleep(1);

	  
	  /*
	  if(ptrFifoShm0!=0) {
	    //assert(ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr)));
	    if(!ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr))) {
	      std::cerr << "Failed to write event" << std::endl;
	      std::cout << "Failed to write event" << std::endl;
	      ptrFifoShm0->print();
	    }
	  }
	  */






	  /*
	  RecordT<1023> buffer;

	  RecordRunning &rr((RecordRunning&)buffer);
	  */
	  //r->setIdentifier(RecordHeader::EventData);
	  //r->setState(FsmState::Running);
	  r->setHeader(_eventNumberInRun);
	  //rr.setUtc(_evtSeqCounter++);
	  //rr.setUtc(_eventNumberInRun);

	  if(hgcrocData) {
	    r->setPayloadLength(40);

	    uint32_t *ptr((uint32_t*)(r->getPayload()));
	    bool found(false);
	    _rxBitShift=7;
	    for(unsigned j(0);j<87 && !found;j++) {
	      if((((_rxSummaryData[0][j]>>_rxBitShift)&0xffffffff) != 0xaccccccc) &&
		 (((_rxSummaryData[0][j]>>_rxBitShift)&0xffffffff) != 0x9ccccccc)) {

		ptr[0]=(_rxSummaryData[0][j]>>_rxBitShift)&0xffffffff;
		for(unsigned i(1);i<40;i++) {
		  ptr[i   ]=(_rxSummaryData[0][j+i]>>_rxBitShift)&0xffffffff;
		}
		for(unsigned i(0);i<40;i++) {
		  ptr[i+40]=(_rxSummaryData[1][j+i]>>_rxBitShift)&0xffffffff;
		}
		found=true;
	      }
	    }

	    if(found) {
	      std::cout << "HGCROC packets" << std::endl;
	      for(unsigned i(0);i<80;i++) {
		std::cout << " Word " << std::setw(4) << i 
			  << " = 0x" << std::hex << std::setfill('0') 
			  << std::setw(8) << ptr[i]
			  << std::dec << std::setfill(' ')
			  << std::endl;
	      }
	    }

	  } else {
	    r->setPayloadLength(4+120);
	    SlinkBoe *boe(r->getSlinkBoe());
	    *boe=SlinkBoe();
	    boe->setEventId(_eventNumberInRun);
	    boe->setL1aSubType(rand()%256);
	    boe->setL1aType(rand()%64);
	    boe->setSourceId(ProcessorDaqLink0FifoShmKey);
	    
	    uint32_t *ptr(r->getDaqPayload());
	    for(unsigned i(0);i<120;i++) {
	      //ptr[2*i+1]=p32[2*i  ];
	      //ptr[2*i  ]=p32[2*i+1];
	      ptr[2*i  ]=p32[2*i  ];
	      ptr[2*i+1]=p32[2*i+1];
	    }
	    
	    SlinkEoe *eoe(r->getSlinkEoe());
	    *eoe=SlinkEoe();
	    
	    eoe->setEventLength(2+60);
	    eoe->setCrc(0xdead);
	    boe->setEventId(_eventNumberInRun);
	    
	    eoe->setBxId((bxNumberInRun%3564)+1);
	    eoe->setOrbitId(bxNumberInRun/3564);
	    
	    if(eoe->eoeHeader()!=SlinkEoe::EoePattern) {
	      eoe->print();
	      r->print();
	      assert(false);
	    }
	    
	    if(_printEnable) {
	      if(_evtSeqCounter<10) {
		std::cout << "HERE" << std::endl;
		r->print();
	      }
	    }
	  }

	  ptrFifoShm0->writeIncrement();
	  /*	
	  if(ptrFifoShm0!=0) {
	    if(_printEnable) ptrFifoShm0->print();
	    //assert(ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr)));
	    if(!ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr))) {
	      std::cerr << "Failed to write event" << std::endl;
	      std::cout << "Failed to write event" << std::endl;
	      ptrFifoShm0->print();
	    }
	  }
	  */

	  
	  if(ptrFifoShm1!=0) {
	    RecordRunning *r;
	    while((r=(RecordRunning*)(ptrFifoShm1->getWriteRecord()))==nullptr) usleep(1);

	    r->setPayloadLength(4+8);
	    SlinkBoe *boe(r->getSlinkBoe());
	    *boe=SlinkBoe();
	    boe->setEventId(_eventNumberInRun);
	    boe->setL1aSubType(rand()%256);
	    boe->setL1aType(rand()%64);
	    boe->setSourceId(ProcessorDaqLink1FifoShmKey);

	    SlinkEoe *eoe(r->getSlinkEoe());
	    *eoe=SlinkEoe();
	    
	    eoe->setEventLength(2+4);
	    eoe->setCrc(0xdead);
	    boe->setEventId(_eventNumberInRun);
	    
	    eoe->setBxId((bxNumberInRun%3564)+1);
	    eoe->setOrbitId(bxNumberInRun/3564);

	    assert(ptrFifoShm1->write(r->totalLength(),(uint64_t*)r));
	  }
	  
	  //usleep(100);
	} else {
	  if(_printEnable) {
	    std::cout << "Backpressured" << std::endl;

	  }
	}
      }

	// Throttle
      _serenityUhal.uhalWrite("payload.fc_ctrl.fpga_fc.ctrl.tts",0);

      RecordContinuing rc;
      rc.setHeader();
      rc.print();
      assert(ptrFifoShm2->write(rc.totalLength(),(uint64_t*)(&rc)));   
    }

    bool ending(FsmInterface::HandshakeState s) {
      ProcessorFastControl::ending(s);

      if(s==FsmInterface::Change) {
        std::cout << "Ending" << std::endl;
        ptrFifoShm0->end();
        ptrFifoShm0->print();
        ptrFifoShm1->end();
        ptrFifoShm1->print();
      }
      return true;
    }


    
  private:
    bool     _rxSummaryValid[8];
    uint64_t _rxSummaryData[8][4000];
    unsigned _rxBitShift;

    DataFifoT<6,1024> *ptrFifoShm0;
    DataFifoT<6,1024> *ptrFifoShm1;

  };

}

#endif
