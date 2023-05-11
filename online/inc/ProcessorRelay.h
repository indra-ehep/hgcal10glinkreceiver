#ifndef ProcessorRelay_h
#define ProcessorRelay_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"



#include "I2cInstruction.h"
#include "UhalInstruction.h"
#include "RecordConfigured.h"



#include "FileWriter.h"
#include "ShmKeys.h"


namespace Hgcal10gLinkReceiver {

  class ProcessorRelay : public ProcessorBase {
    
  public:

    ProcessorRelay() {
    }

    virtual ~ProcessorRelay() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey0, uint32_t fifoKey1) {
      ShmSingleton< DataFifoT<6,1024> > shm2;
      ptrFifoShm2=shm2.setup(fifoKey0);
      startFsm(rcKey);
    }

    virtual bool initializing(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	ptrFifoShm2->ColdStart();
      }
      return true;
    }

    bool configuringA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordConfiguringA &r((RecordConfiguringA&)(_ptrFsmInterface->commandPacket().record()));

	_fileWriter.openRelay(r.superRunNumber());
	_fileWriter.write(&r);


	_cfgSeqCounter=1;
	_evtSeqCounter=1;
      }

      return true;
    }
    
    bool configuringB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_fileWriter.write(&(_ptrFsmInterface->commandPacket().record()));

	_eventNumberInConfiguration=0;
      }
      return true;
    }

    bool starting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_fileWriter.write(&(_ptrFsmInterface->commandPacket().record()));

	_pauseCounter=0;
	_eventNumberInRun=0;
      }

      return true;
    }

    bool pausing(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_fileWriter.write(&(_ptrFsmInterface->commandPacket().record()));
      }
      return true;
    }
    
    bool resuming(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_fileWriter.write(&(_ptrFsmInterface->commandPacket().record()));
      }
      return true;
    }
    
    bool stopping(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordStopping r;
	r.deepCopy((_ptrFsmInterface->commandPacket().record()));

	_eventNumberInConfiguration+=_eventNumberInRun;

	r.setNumberOfEvents(_eventNumberInRun);
	r.setNumberOfPauses(_pauseCounter);
	if(_printEnable) r.print();

	_fileWriter.write(&r);
      }
      return true;
    }
    
    bool haltingB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_fileWriter.write(&(_ptrFsmInterface->commandPacket().record()));

	_eventNumberInSuperRun+=_eventNumberInConfiguration;
      }
      return true;
    }
    
    bool haltingA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_fileWriter.write(&(_ptrFsmInterface->commandPacket().record()));
	_fileWriter.close();
      }
      return true;
    }
    
    bool resetting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_fileWriter.close();
      }
      return true;
    }

    bool ending(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_fileWriter.close();
      }
      return true;
    }

    //////////////////////////////////////////////

    virtual void configuredA() {
      if(_printEnable) std::cout << "ProcessRelay::configuredA()" << std::endl;

      const Record *r;

      bool done(false);
      while(!done) {
	while((r=ptrFifoShm2->readRecord())==nullptr) usleep(10);
	if(_printEnable) r->print();

	if(r->state()!=FsmState::Continuing) {
	  _fileWriter.write(r);
	} else {
	  done=true;
	}
	ptrFifoShm2->readIncrement();
      }
    }
    
    virtual void running() {
      if(_printEnable) std::cout << "ProcessRelay::running()" << std::endl;

      const Record *r;

      bool done(false);
      while(!done) {
	while((r=ptrFifoShm2->readRecord())==nullptr) usleep(10);
	if(_printEnable) r->print();

	if(r->state()!=FsmState::Continuing) {
	  _fileWriter.write(r);
	} else {
	  done=true;
	}
	ptrFifoShm2->readIncrement();
      }
    }
    
    virtual void configuredB() {
      if(_printEnable) std::cout << "ProcessRelay::configuredB()" << std::endl;

      const Record *r;

      bool done(false);
      while(!done) {
	while((r=ptrFifoShm2->readRecord())==nullptr) usleep(10);
	if(_printEnable) r->print();

	if(r->state()!=FsmState::Continuing) {
	  _fileWriter.write(r);
	} else {
	  done=true;
	}
	ptrFifoShm2->readIncrement();
      }


#ifdef NOT_YET



      std::cout << "configuredB() super run = " << _superRunNumber << std::endl;

      //RecordConfiguredB *r;
      RecordConfigured *r;
      for(unsigned i(1);i<=3;i++) {

	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::ConfiguredB);
	r->setType(RecordConfigured::HGCROC);
	r->setLocation(0xfe00+i);
	r->print();
      
	I2cInstruction i2c;
	
	std::ostringstream sout;
	sout << "HgcrocCfg_ROCs" << i << ".cfg";
	
	std::ifstream fin;
	fin.open(sout.str().c_str());
	if(fin) {
	  char buffer[16];
	  
	  uint16_t add;
	  uint16_t val;
	  uint8_t mask(0xff);

	  std::cout << std::hex << std::setfill('0');

	  fin.getline(buffer,16);
	  while(fin) {
	    assert(buffer[ 1]=='x');
	    assert(buffer[10]=='x');	

	    buffer[ 6]='\0';
	    buffer[13]='\0';
	  
	    std::istringstream sAdd(buffer+ 2);
	    std::istringstream sVal(buffer+11);
	  
	    sAdd >> std::hex >> add;
	    sVal >> std::hex >> val;

	    i2c.setAddress(add);
	    i2c.setMask(mask);
	    i2c.setValue(val&0xff);
	    i2c.print();
	  
	    std::cout << "HGCROC cfg address 0x" << std::setw(4) << add
		      << ", mask 0x" << std::setw(2) << unsigned(mask)
		      << ", value 0x" << std::setw(2) << unsigned(val)
		      << std::endl;

	    r->addData32(i2c.data());
	    if(add<16) r->print();
	  
	    fin.getline(buffer,16);
	  }
	
	  std::cout << std::dec << std::setfill(' ');
	}
	fin.close();
	r->print();
	ptrFifoShm2->writeIncrement();
      }

      
      ///////////////
      /*
	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::ConfiguredB);
	r->setType(RecordConfigured::BE);
	r->setLocation(0xbe00);
	r->print();
      
	for(unsigned i(0);i<_uhalString.size() && i<10;i++) {
	r->addString(_uhalString[i]);
	}
	r->print();
	ptrFifoShm2->writeIncrement();
      
	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::ConfiguredB);
	r->setType(RecordConfigured::BE);
	r->setLocation(0xbe01);
	r->print();


	UhalInstruction xi;

	//uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
	//uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);


	for(unsigned i(0);i<_uhalString.size();i++) {
	xi.setAddress(i);
	#ifdef ProcessorHardware
	#ifdef JUNK
	const uhal::Node& lNode = lHW.getNode("payload."+_uhalString[i]);
	uhal::ValWord<uint32_t> lReg = lNode.read();
	lHW.dispatch();
	xi.setValue(lReg.value());
	#endif
	#else
	xi.setValue(0x1000*i);
	#endif
	xi.print();
	r->addData64(xi.data());
	}
    
	if(_printEnable) r->print();
	ptrFifoShm2->writeIncrement();
      */
          
      ///////////////
	
      /* 
	 char c='1';
	 for(uint8_t i(0);i<3;i++) {
	 RecordConfiguredB h;
	 h.setHeader(_cfgSeqCounter++);
	 //h.setState(FsmState::ConfiguredB);
	 //h.setPayloadLength(1);
	 h.setSuperRunNumber(_superRunNumber);
	 h.setConfigurationCounter(_cfgSeqCounter++);
	
	 std::string s("HGROC_");
	 s+=(c+i);
	 h.setConfigurationPacketHeader(s);
	 h.setConfigurationPacketValue(0xbeefbeefcafecafe);
	
	 h.print();
	
	 assert(ptrFifoShm2->write(h.totalLength(),(uint64_t*)(&h)));
	 }
      */      

#endif
    }


    void paused() {
      if(_printEnable) std::cout << "ProcessRelay::paused()" << std::endl;

      const Record *r;

      bool done(false);
      while(!done) {
	while((r=ptrFifoShm2->readRecord())==nullptr) usleep(10);
	if(_printEnable) r->print();

	if(r->state()!=FsmState::Continuing) {
	  _fileWriter.write(r);
	} else {
	  done=true;
	}
	ptrFifoShm2->readIncrement();
      }
      
      _pauseCounter++;
    }
    
  protected:
    DataFifoT<6,1024> *ptrFifoShm2;

    FileWriter _fileWriter;
    
    uint32_t _cfgSeqCounter;
    uint32_t _evtSeqCounter;
    uint32_t _pauseCounter;

    uint32_t _superRunNumber;
    uint32_t _runNumber;

    uint32_t _runNumberInSuperRun;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;
  };

}

#endif