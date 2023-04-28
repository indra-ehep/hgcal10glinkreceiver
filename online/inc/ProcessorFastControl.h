#ifndef ProcessorFastControl_h
#define ProcessorFastControl_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

//#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"
#include "RecordHeader.h"
#include "RecordInitializing.h"
#include "RecordConfiguringA.h"
#include "RecordConfiguringB.h"
#include "RecordConfiguredB.h"
#include "RecordStarting.h"
#include "RecordPausing.h"
#include "RecordResuming.h"
#include "RecordStopping.h"
#include "RecordHaltingB.h"
#include "RecordHaltingA.h"
#include "RecordResetting.h"
#include "RecordRunning.h"
#include "ShmKeys.h"
#include "SlinkBoe.h"
#include "SlinkEoe.h"

#undef ProcessorFastControlHardware
//#define ProcessorFastControlHardware

#ifdef ProcessorFastControlHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class ProcessorFastControl : public ProcessorBase {

  public:
    ProcessorFastControl() {
    }

    virtual ~ProcessorFastControl() {
    }

    void setUpAll(uint32_t rcKey, uint32_t fifoKey2,
		  uint32_t fifoKey0, uint32_t fifoKey1) {
      _CB=0;
      
      ShmSingleton< DataFifoT<6,1024> > shm0;
      shm0.setup(fifoKey0);
      ptrFifoShm0=shm0.payload();
      ShmSingleton< DataFifoT<6,1024> > shm1;
      shm1.setup(fifoKey1);
      ptrFifoShm1=shm1.payload();

      setUpAll(rcKey,fifoKey2);
    }
    
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      ShmSingleton< DataFifoT<6,1024> > shm2;
      shm2.setup(fifoKey);
      ptrFifoShm2=shm2.payload();

      ptrFifoShm0=0;
      ptrFifoShm1=0;

      startFsm(rcKey);
    }

    bool initializing(FsmInterface::HandshakeState s) {
#ifdef ProcessorFastControlHardware
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
#endif
      return true;
    }

    bool configuringA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_cfgSeqCounter=1;
	_evtSeqCounter=1;

	RecordConfiguringA rca;
	_superRunNumber=rca.superRunNumber();
	
	rca.deepCopy(_ptrFsmInterface->commandPacket().record());
	rca.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rca.totalLength(),(uint64_t*)(&rca)));
      }
      return true;
    }
    
    bool configuringB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordConfiguringB rcb;
	rcb.deepCopy(_ptrFsmInterface->commandPacket().record());
	rcb.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rcb.totalLength(),(uint64_t*)(&rcb)));
      }
      return true;
    }

    bool starting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_pauseCounter=0;
	_eventNumber=0;
      
	RecordStarting rr;
	_runNumber=rr.runNumber();

	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      }

      return true;
    }

    bool pausing(FsmInterface::HandshakeState s) {
      /* NOT FOR Shm2
	 if(s==FsmInterface::Change) {
	 RecordPausing rr;
	 rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	 rr.print();
	 ptrFifoShm2->print();
	 assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
	 }
      */
      return true;
    }
    
    bool resuming(FsmInterface::HandshakeState s) {
      /* NOT FOR Shm2
	 if(s==FsmInterface::Change) {
	 RecordResuming rr;
	 rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	 rr.print();
	 ptrFifoShm2->print();
	 assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
	 }
      */
      return true;
    }
    
    bool stopping(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordStopping rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.setNumberOfEvents(_eventNumber);
	rr.setNumberOfPauses(_pauseCounter);
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      }
      return true;
    }
    
    bool haltingB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordHaltingB rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      }
      return true;
    }
    
    bool haltingA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordHaltingA rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      }
      return true;
    }
    
    bool resetting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordResetting rr;
	rr.print();
      }
      return true;
    }
    /*
//////////////////////////////////////////////
    
void initial() {
//sleep(1);
}
    
void halted() {
//sleep(1);
}
    
void configuredA() {
//sleep(1);
}
    */    

    virtual void configuredB() {
      char c='1';
      for(uint8_t i(0);i<3;i++) {
	RecordConfiguredB h;
	h.setHeader(_cfgSeqCounter++);
	h.setState(FsmState::ConfiguredB);
	h.setPayloadLength(1);
	h.setSuperRunNumber(_superRunNumber);
	h.setConfigurationCounter(_cfgSeqCounter++);
	
	std::string s("HGROC_");
	s+=(c+i);
	h.setConfigurationPacketHeader(s);
	h.setConfigurationPacketValue(0xbeefbeefcafecafe);
	
	h.print();
	
	assert(ptrFifoShm2->write(h.totalLength(),(uint64_t*)(&h)));
      }
    }

    void running() {
      while(_ptrFsmInterface->isIdle()) {
	RecordT<128> buffer;

	RecordRunning &rr((RecordRunning&)buffer);
	rr.setIdentifier(RecordHeader::EventData);
	rr.setState(FsmState::Running);
	rr.setPayloadLength(4);
	rr.setUtc(_evtSeqCounter++);

	SlinkBoe *boe(rr.slinkBoe());
	*boe=SlinkBoe();
	boe->setSourceId(RunControlDaqLink0ShmKey);

	SlinkEoe *eoe(rr.slinkEoe());
	*eoe=SlinkEoe();
	eoe->setEventLength(2);
	eoe->setCrc(0xdead);

	if(_evtSeqCounter<10) {
	std::cout << "HERE" << std::endl;
	rr.print();
	}
	
	if(ptrFifoShm0!=0) {
	  ptrFifoShm0->print();
	  assert(ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr)));
	}
      
	  _eventNumber++;

	  boe->setL1aSubType(rand()%256);
	  boe->setL1aType(rand()%64);
	  boe->setEventId(_eventNumber);

	  eoe->setBxId((rand()%3564)+1);
	  eoe->setOrbitId(rand());

	if(_evtSeqCounter<10) {
	  rr.print();
	}	

	  if(ptrFifoShm0!=0) {
	    assert(ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr)));
	  }
#ifdef ProcessorFastControlHardware
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
	    std::cout << " 0x" << std::hex << std::setfill('0') 
	    << std::setw(2) << unsigned(c)
	    << " 0x" << std::setw(16) << d
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
      

	  boe->setSourceId(RunControlDaqLink0ShmKey+1);
	  if(ptrFifoShm0!=0) {
	    assert(ptrFifoShm1->write(rr.totalLength(),(uint64_t*)(&rr)));
	  }
	if(_evtSeqCounter<10) {
	  rr.print();
	}	
      }
    }

    
  void paused() {
    _pauseCounter++;
  }

   
  private:
    DataFifoT<6,1024> *ptrFifoShm2;
    DataFifoT<6,1024> *ptrFifoShm0;
    DataFifoT<6,1024> *ptrFifoShm1;
    unsigned _CB;
    uint32_t _cfgSeqCounter;
    uint32_t _evtSeqCounter;
    uint32_t _pauseCounter;

    uint32_t _superRunNumber;
    uint32_t _runNumber;
    uint32_t _eventNumber;
  };

}

#endif
