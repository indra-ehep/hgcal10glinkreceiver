#ifndef ProcessorDaqLink2_h
#define ProcessorDaqLink2_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"
#include "RecordHeader.h"
#include "RecordHaltingA.h"
#include "RecordResetting.h"
#include "FileWriter.h"


namespace Hgcal10gLinkReceiver {

  class ProcessorDaqLink2 : public ProcessorBase {

  public:
    ProcessorDaqLink2() {
    }

    virtual ~ProcessorDaqLink2() {
    }

    void setUpAll(uint32_t rcKey, uint32_t fifoKey2,
		  uint32_t fifoKey0, uint32_t fifoKey1) {
      ShmSingleton< DataFifoT<6,1024> > shm0;
      _ptrFifo0=shm0.setup(fifoKey0);
      //ptrRunFileShm=shm0.payload();

      ShmSingleton< DataFifoT<6,1024> > shm1;
      _ptrFifo1=shm1.setup(fifoKey1);
      //ptrRunFileShm=shm1.payload();

      ShmSingleton< DataFifoT<6,1024> > shm2;
      shm2.setup(fifoKey2);
      _ptrFifo2=shm2.payload();

      startFsm(rcKey);
    }

    bool initializing(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_ptrFifo2->ColdStart();
      }
      return true;
    }

    bool configuringA(FsmInterface::HandshakeState s) {
      std::cout << "HERE Handshake = " << FsmInterface::handshakeName(s)
		<< std::endl;

      if(s==FsmInterface::Change) {
	RecordConfiguringA rca;
	while(_ptrFifo2->read((uint64_t*)(&rca))==0) usleep(10);
	rca.print();
	_fileWriter.openRelay(rca.superRunNumber());
	_fileWriter.write(&rca);
      }
      
      return true;
    }

    bool configuringB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordT<1024> r;
	while(_ptrFifo2->read((uint64_t*)(&r))==0) usleep(10);
	if(_printEnable) r.print();
	_fileWriter.write(&r);
      }
      return true;
    }
    
    bool starting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordT<1024> r;
	while(_ptrFifo2->read((uint64_t*)(&r))==0) usleep(10);
	_fileWriter.write(&r);
      }
      return true;
    }
    
    bool pausing(FsmInterface::HandshakeState s) {
      return true;
    }
    
    bool resuming(FsmInterface::HandshakeState s) {
      return true;
    }
    
    bool stopping(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordT<1024> r;
	while(_ptrFifo2->read((uint64_t*)(&r))==0) usleep(10);
	_fileWriter.write(&r);
      }
	return true;
    }    

    bool haltingB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordT<1024> r;
	while(_ptrFifo2->read((uint64_t*)(&r))==0) usleep(10);
	RecordPrinter(&r);
	_fileWriter.write(&r);
      }
      return true;
    }
    
    bool haltingA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordHaltingA rha;
	while(_ptrFifo2->read((uint64_t*)(&rha))==0) usleep(10);
	rha.print();
	_fileWriter.write(&rha);
	_fileWriter.close();
      }
      return true;
    }

      bool resetting(FsmInterface::HandshakeState s) {
	if(s==FsmInterface::Change) {
	  RecordResetting r;
	  r.print();
	  _fileWriter.write(&r);
	  _fileWriter.close();
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
      //assert(false);
      /*
	RecordHeader h;
	h.setIdentifier(RecordHeader::EventData);
	h.setState(FsmState::ConfiguredB);
	h.setLength(20);
	h.setUtc();
	h.print();
      */
      uint64_t buffer[1024];
      RecordHeader *h((RecordHeader*)buffer);
      h->setState(FsmState::ConfiguredB);
      while(h->state()!=FsmState::Continuing) {
	while(_ptrFifo2->read(buffer)>0) {
	  h->print();
	  assert(h->state()==FsmState::Continuing ||
		 h->state()==FsmState::ConfiguredB);
	  if(h->state()==FsmState::ConfiguredB) _fileWriter.write(h);
	}
      }
      //sleep(1);
      //while(_ptrFifo2->read(buffer)>0) {
      //h->print();
      //_fileWriter.write(h);
      //}
    }

    virtual void running() {
      uint32_t backPressureOn(16);
      uint32_t backPressureOff(48);

      while(_ptrFsmInterface->isIdle()) {	
	if(_ptrFifo2->backPressure()) {
	  if(_ptrFifo0->writeable()>=backPressureOff &&
	     _ptrFifo1->writeable()>=backPressureOff) {
	    _ptrFifo2->setBackPressure(false);

	    if(_printEnable) {
	      std::cout << "Backpressure turned off"
			<< std::endl;
	    }
	    sleep(1);
	  }

	} else {
	  if(_ptrFifo0->writeable()<backPressureOn ||
	     _ptrFifo1->writeable()<backPressureOn) {
	    _ptrFifo2->setBackPressure(true);

	    if(_printEnable) {
	      std::cout << "Backpressure turned on"
			<< std::endl;
	    }
	    sleep(1);
	  }
	}
      }
    }
   /*
      void running(RunControlFsmShm* const p) {
      RecordHeader h;
      h.setIdentifier(RecordHeader::EventData);
      h.setState(FsmState::Running);
      h.setLength(20);
      h.setUtc();
      h.print();
      
      std::memcpy(_ptrFifo2->_buffer[_ptrFifo2->_writePtr%RunFileShm2::BufferDepth],
      &h,8*h.length());
      _ptrFifo2->_writePtr++;
      //sleep(10);
      //p->setRequest(FsmState::Stopping);
      }
    
      void paused() {
      //sleep(1);
      }
    */
   
  private:
    const DataFifoT<6,1024> *_ptrFifo0;
    const DataFifoT<6,1024> *_ptrFifo1;
    DataFifoT<6,1024> *_ptrFifo2;
    FileWriter _fileWriter;
  };

}

#endif
