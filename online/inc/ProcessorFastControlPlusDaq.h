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

      //ShmSingleton< DataFifoT<6,1024> > shm1;
      //shm1.setup(fifoKey1);
      //ptrFifoShm1=shm1.payload();
      ptrFifoShm1=0;
      
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

	    if(i>0 || n>0) _rxSummaryData[j][i-1+128*n]|=(value<<32);
	    _rxSummaryData[j][i+128*n]=value;
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

   
    bool initializing(FsmInterface::HandshakeState s) {
      ProcessorFastControl::initializing(s);
      
#ifdef ProcessorFastControlPlusDaqHardware
      system("/home/cmx/pdauncey/source setFC.sh");

      const std::string lConnectionFilePath = "etc/connections.xml";
      const std::string lDeviceId = "x0";
  
      std::vector<std::string> lRegisterName;
      std::vector<std::string> lRegisterNameW;
  
      lRegisterName.push_back("tcds2_emu.ctrl_stat.ctrl.seq_length");

      uhal::setLogLevelTo(uhal::Error());  
      uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
      uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);
  
  
      const uhal::Node& lNode = lHW.getNode("payload.fc_ctrl." + lRegisterName[0]);
      std::cout << "Reading from register '" << lRegisterName[0] << "' ..." << std::endl;
      uhal::ValWord<uint32_t> lReg = lNode.read();
      lHW.dispatch();
      std::cout << "... success!" << std::endl << "Value = 0x" << std::hex << lReg.value() << std::endl;

      std::vector<std::string> ids;//, ids_filtered;
      std::vector<uint32_t> nds;//, ids_filtered;
      //ids = lHW.getNode("payload.fc_ctrl.tcds2_emu").getNodes();
      ids = lHW.getNode("payload.fc_ctrl").getNodes();
  
      std::cout << "payload.fc_ctrl.tcds2_emu.getNodes(): ";
      /*
	std::copy(ids.begin(),
	ids.end(),
	std::ostream_iterator<std::string>(std::cout,", "));
      */
      for(unsigned i(0);i<ids.size();i++) {
	std::cout << ids[i] << std::endl;
      }
      std::cout << std::endl << std::endl;




      for(unsigned i(0);i<ids.size();i++) {
  
	// PART 3: Reading from the register
	const uhal::Node& lNode = lHW.getNode("payload.fc_ctrl." + ids[i]);

	std::cout << "Reading from register '" << ids[i] << "' ..." << std::endl;

	uhal::ValWord<uint32_t> lReg = lNode.read();
	// dispatch method sends read request to hardware, and waits for result to return
	// N.B. Before dispatch, lReg.valid() == false, and lReg.value() will throw
	lHW.dispatch();

	std::cout << "... success!" << std::endl << "Value = 0x" << std::hex << lReg.value() << std::endl;
	nds.push_back(lReg.value());
      }

      // Read in HGCROC capture file
      unsigned ncount[8][256],mcount[8],kcount[8];

      for(unsigned j(0);j<8;j++) {
	mcount[j]=0;
	kcount[j]=256;
      }
      
      /*
	uhalWrite("lpgbt0.lpgbt_frame.shift_elink0",0);
	uhalWrite("lpgbt0.lpgbt_frame.shift_elink1",0);
	uhalWrite("lpgbt0.lpgbt_frame.shift_elink2",0);
	uhalWrite("lpgbt0.lpgbt_frame.shift_elink3",0);
	uhalWrite("lpgbt0.lpgbt_frame.shift_elink4",k);
	uhalWrite("lpgbt0.lpgbt_frame.shift_elink5",k);
	uhalWrite("lpgbt0.lpgbt_frame.shift_elink6",0);

	uhalWrite("lpgbt1.lpgbt_frame.shift_elink0",0);
	uhalWrite("lpgbt1.lpgbt_frame.shift_elink1",0);
	uhalWrite("lpgbt1.lpgbt_frame.shift_elink2",0);
	uhalWrite("lpgbt1.lpgbt_frame.shift_elink3",0);
	uhalWrite("lpgbt1.lpgbt_frame.shift_elink4",k);
	uhalWrite("lpgbt1.lpgbt_frame.shift_elink5",k);
	uhalWrite("lpgbt1.lpgbt_frame.shift_elink6",0);
      */
      std::cout << "readRxSummaryFile returns " << readRxSummaryFile(1) << std::endl;

      for(unsigned k(0);k<32;k++) {
	for(unsigned j(0);j<8;j++) {
	  ncount[j][k]=0;

	  for(unsigned i(0);i<128;i++) {
	    if(_rxSummaryData[j][i]==0xaccccccc ||
	       _rxSummaryData[j][i]==0x9ccccccc) ncount[j][k]++;
	  }
	}
	
	for(unsigned j(0);j<8;j++) {
	  if(mcount[j]<ncount[j][k]) {
	    mcount[j]=ncount[j][k];
	    kcount[j]=k;
	  }
	}
      }

      for(unsigned j(0);j<8;j++) {
	_rxSummaryValid[j]=(mcount[j]>10 && kcount[j]<256);
      }
      
      for(unsigned j(0);j<8;j++) {
	std::cout << "RX channel " << j << ", max matches = " << mcount[j]
		  << " out of 127, for offset " << kcount[j]
		  << ", valid = " << _rxSummaryValid[j] << std::endl;
      }
      
      // HGCROC
      //bool idle0(_vData[0][w]==0xaccccccc || _vData[0][w]==0x9ccccccc);
      //bool idle1(_vData[1][w]==0xaccccccc || _vData[1][w]==0x9ccccccc);

      // ECON-D
      //const uint32_t idleWord(0x77777700);
      //const uint32_t idleWord(0xaaaaaa00);
      //const uint32_t idleWord(0x55555500);
      //const uint32_t idleWord(0xaaaaff00);


#endif
      return true;
    }

    void running() {
      _serenityUhal.uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",1); // TEMP
      _serenityUhal.uhalWrite("fc_ctrl.fpga_fc.ctrl.tts",1);

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
	    r->setPayloadLength(4+40);

	    uint32_t *ptr((uint32_t*)(r->getPayload()));
	    for(unsigned i(0);i<40;i++) {
	      ptr[i]=i;
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
	    //boe->setSourceId(ProcessorDaqLink1FifoShmKey);
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
      _serenityUhal.uhalWrite("fc_ctrl.fpga_fc.ctrl.tts",0);
      _serenityUhal.uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",0); // TEMP
    }

    
    void paused() {
      _pauseCounter++;
    }

   
  private:
    bool     _rxSummaryValid[8];
    uint64_t _rxSummaryData[8][4000];

    DataFifoT<6,1024> *ptrFifoShm0;
    DataFifoT<6,1024> *ptrFifoShm1;

  };

}

#endif
