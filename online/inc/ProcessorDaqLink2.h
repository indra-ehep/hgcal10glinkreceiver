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

    void setUpAll(uint32_t rcKey,uint32_t fifoKey) {

      _CB=124;

      ShmSingleton< DataFifoT<6,1024> > shmU;
      shmU.setup(fifoKey);
      ptrRunFileShm=shmU.payload();
      startFsm(rcKey);
    }

    bool initializing(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	ptrRunFileShm->ColdStart();
      }
      return true;
    }

    bool configuringA(FsmInterface::HandshakeState s) {
      std::cout << "HERE Handshake = " << FsmInterface::handshakeName(s)
		<< std::endl;

      if(s==FsmInterface::Change) {
	RecordConfiguringA rca;
	while(ptrRunFileShm->read((uint64_t*)(&rca))==0) usleep(10);
	rca.print();
	_fileWriter.open(rca.superRunNumber(),2,true,true);
	_fileWriter.write(&rca);
      }
      
      return true;
    }

    bool configuringB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordT<1024> r;
	while(ptrRunFileShm->read((uint64_t*)(&r))==0) usleep(10);
	_fileWriter.write(&r);
      }
      return true;
    }
    
    bool starting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordT<1024> r;
	while(ptrRunFileShm->read((uint64_t*)(&r))==0) usleep(10);
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
	while(ptrRunFileShm->read((uint64_t*)(&r))==0) usleep(10);
	_fileWriter.write(&r);
      }
	return true;
    }    

    bool haltingB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordT<1024> r;
	while(ptrRunFileShm->read((uint64_t*)(&r))==0) usleep(10);
	RecordPrinter(&r);
	_fileWriter.write(&r);
      }
      return true;
    }
    
    bool haltingA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordHaltingA rha;
	while(ptrRunFileShm->read((uint64_t*)(&rha))==0) usleep(10);
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
      uint64_t buffer[128];
      RecordHeader *h((RecordHeader*)buffer);
      h->setState(FsmState::ConfiguredB);
      while(h->state()!=FsmState::Continuing) {
	while(ptrRunFileShm->read(buffer)>0) {
	  h->print();
	  assert(h->state()==FsmState::Continuing ||
		 h->state()==FsmState::ConfiguredB);
	  if(h->state()==FsmState::ConfiguredB) _fileWriter.write(h);
	}
      }
      //sleep(1);
      //while(ptrRunFileShm->read(buffer)>0) {
      //h->print();
      //_fileWriter.write(h);
      //}
    }

    /*
      void running(RunControlFsmShm* const p) {
      RecordHeader h;
      h.setIdentifier(RecordHeader::EventData);
      h.setState(FsmState::Running);
      h.setLength(20);
      h.setUtc();
      h.print();
      
      std::memcpy(ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm2::BufferDepth],
      &h,8*h.length());
      ptrRunFileShm->_writePtr++;
      //sleep(10);
      //p->setRequest(FsmState::Stopping);
      }
    
      void paused() {
      //sleep(1);
      }
    */
   
  private:
    DataFifoT<6,1024> *ptrRunFileShm;
    unsigned _CB;
    FileWriter _fileWriter;
  };

}

#endif
