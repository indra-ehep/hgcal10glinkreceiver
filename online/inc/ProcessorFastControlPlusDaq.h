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
#include "ProcessorFastControl.h"
#include "DataFifo.h"
#include "RecordPrinter.h"
#include "ShmKeys.h"
#include "SlinkBoe.h"
#include "SlinkEoe.h"

#undef ProcessorFastControlPlusDaqHardware
//#define ProcessorFastControlPlusDaqHardware

#ifdef ProcessorFastControlPlusDaqHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

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

    void readRxSummaryFile() {
      const unsigned offset(27);

      std::ifstream fin;
      fin.open("rx_summary.txt");
      if(!fin) return;
      
      char buffer[1024];
      fin.getline(buffer,1024);
      fin.getline(buffer,1024);
      fin.getline(buffer,1024);
      fin.getline(buffer,1024);
      
      for(unsigned i(0);i<128;i++) {
	for(unsigned j(0);j<8;j++) {
	  fin.getline(buffer,1024);
	  buffer[offset+8]='\0';
	  std::istringstream sin(buffer+offset);
	  sin >> std::hex >> _rxSummaryData[j][i];
	}
      }      

      fin.close();
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
#else
      if(s==FsmInterface::Change) {
	RecordInitializing ri;
	ri.print();
      }

      // Read in HGCROC capture file
      unsigned ncount[8][32],mcount[8],kcount[8];

      for(unsigned j(0);j<8;j++) {
	mcount[j]=0;
	kcount[j]=32;
      }
      
      for(unsigned k(0);k<1;k++) {
	readRxSummaryFile();

	for(unsigned j(0);j<8;j++) {
	  ncount[j][k]=0;

	  for(unsigned i(0);i<128;i++) {
	    if(_rxSummaryData[j][i]==0xaccccccc ||
	       _rxSummaryData[j][i]==0x9ccccccc ||
	       _rxSummaryData[j][i]==0x3333332b) ncount[j][k]++;
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
	_rxSummaryValid[j]=(mcount[j]>10 && kcount[j]<32);
      }
      
      for(unsigned j(0);j<8;j++) {
	std::cout << "RX channel " << j << ", max matches = " << mcount[j]
		  << " out of 128, for offset " << kcount[j]
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
      while(_ptrFsmInterface->isIdle()) {
	if(!ptrFifoShm2->backPressure()) {
	  _eventNumberInRun++;
	  
	  readRxSummaryFile();

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
	  RecordT<128> buffer;

	RecordRunning &rr((RecordRunning&)buffer);
	rr.setIdentifier(RecordHeader::EventData);
	rr.setState(FsmState::Running);
	rr.setPayloadLength(4+120);
	rr.setUtc(_evtSeqCounter++);

	SlinkBoe *boe(rr.getSlinkBoe());
	*boe=SlinkBoe();
	boe->setSourceId(ProcessorDaqLink0FifoShmKey);

	uint64_t *ptr(rr.getDaqPayload());
	for(unsigned i(0);i<120;i++) {
	  ptr[i]=miniDaq[i];
	}
	
	
	SlinkEoe *eoe(rr.getSlinkEoe());
	*eoe=SlinkEoe();

	eoe->setEventLength(2+60);
	eoe->setCrc(0xdead);
	boe->setL1aSubType(rand()%256);
	boe->setL1aType(rand()%64);
	boe->setEventId(_eventNumberInRun);
	
	eoe->setBxId((rand()%3564)+1);
	eoe->setOrbitId(time(0)); // CLUDGE!

	if(_printEnable) {
	  if(_evtSeqCounter<10) {
	    std::cout << "HERE" << std::endl;
	    rr.print();
	  }
	}
	
	if(ptrFifoShm0!=0) {
	  if(_printEnable) ptrFifoShm0->print();
	  //assert(ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr)));
	  if(!ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr))) {
	    std::cerr << "Failed to write event" << std::endl;
	    std::cout << "Failed to write event" << std::endl;
	    ptrFifoShm0->print();
	  }
	}

#ifdef ProcessorFastControlPlusDaqHardware
	  std::ifstream fin("/home/cmx/pdauncey/data/rx_summary.txt");
	  if(!fin) {
	    std::cout << "Failed to open file" << std::endl;
	    //return false;
	  }

	  uint64_t singled[128];
	  uint64_t doubled[127];

	  char buffer[1024];
	  fin.getline(buffer,1024);
	  //if(sfhDebug) 
	  std::cout << buffer << std::endl;
	  fin.getline(buffer,1024);
	  //if(sfhDebug) 
	  std::cout << buffer << std::endl;
      
	  std::string str;
	  fin >> str;
	  ///if(sfhDebug)
	  std::cout << str;
	  fin >> str;
	  std::cout << " " << str;
	  fin >> str;
	  std::cout << " " << str;
	  uint64_t n64;
	  for(unsigned i(0);i<128 && fin;i++) {
      
	    fin >> str;
	    std::cout << " " << str;
	    fin >> str;
	    std::cout << " " << str << std::endl;

	    fin >> std::hex >> singled[i]
	      std::cout << std::hex << singled[i] << std::endl;
	    fin >> str;
	    std::cout << " " << str;
	  }
	  std::cout << std::dec << std::endl;

	  fin.close();
#endif


	  //fin >> n64;
	  //std::cout << n64 << std::endl;

	  /*
	    unsigned nLinks(0);
	    for(unsigned i(0);i<120 && str!="Frame";i++) {
	    nLinks++;
	
	    if(!append) {
	    v.push_back(SummaryLinkRawData());
	    v.back().link(str);
	    v.back().utc(timestamp);
	    v.back().bx(bxStart);
	    }
	
	    if(i>0 && sfhDebug) std::cout << " " << str;
	    fin >> str;
	    }
      
	    assert(nLinks==v.size());
	    if(sfhDebug) std::cout << std::endl;
	  */



	  /*
	    unsigned m;
	    uint64_t d;
	    unsigned f;
      
	    for(unsigned i(0);i<2048 && fin;i++) {
	    assert(str=="Frame");
	
	    fin >> f;
	    assert(f==i);
	
	    if(sfhDebug) {
	    std::cout << "Frame " << std::setw(4) << i
	    << ", command and data = ";
	    }
	
	    for(unsigned j(0);j<v.size();j++) {
	    fin >> m >> std::hex >> d >> std::dec;
	  
	    uint8_t c(0);
	    if( (m/1000    )==1) c+=8;
	    if(((m/100 )%10)==1) c+=4;
	    if(((m/10  )%10)==1) c+=2;
	    if( (m      %10)==1) c+=1;
	  
	    if(sfhDebug) {
	    if(j>0) std::cout << ", ";
	    std::cout << " 0x0x" << std::hex << std::setfill('0') 
	    << std::setw(2) << unsigned(c)
	    << " x" << std::setw(16) << d
	    << std::dec << std::setfill(' ');
	    }
	  
	    v[j].append(c,d);
	    }
	
	    if(sfhDebug) std::cout << std::endl;
	
	    fin >> str;
	    }
      
	    if(!append) {
	    for(unsigned j(0);j<v.size();j++) v[j].setPhase();
	    }
	  */

	  boe->setSourceId(ProcessorDaqLink1FifoShmKey);
	  
	  if(ptrFifoShm1!=0) {
	    assert(ptrFifoShm1->write(rr.totalLength(),(uint64_t*)(&rr)));
	  }
	  
	//usleep(100);
	} else {
	  if(_printEnable) {
	    std::cout << "Backpressured" << std::endl;

	  }
	}
      }
    }

    
  void paused() {
    _pauseCounter++;
  }

   
  private:
  bool     _rxSummaryValid[8];
  uint32_t _rxSummaryData[8][128];

    DataFifoT<6,1024> *ptrFifoShm0;
    DataFifoT<6,1024> *ptrFifoShm1;

  };

}

#endif
