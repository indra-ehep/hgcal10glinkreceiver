#ifndef ProcessorDaqLink0_h
#define ProcessorDaqLink0_h

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

  class ProcessorDaqLink0 : public ProcessorBase {

  public:
    ProcessorDaqLink0() {
    }

    virtual ~ProcessorDaqLink0() {
    }

    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
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
    
    bool starting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	assert(_ptrFsmInterface->commandPacket().record().state()==FsmState::Starting);
	RecordStarting r;
	std::cout << "HERE0 starting" << std::endl;
	r.deepCopy(_ptrFsmInterface->commandPacket().record());
	r.print();
	_fileWriter.open(r.runNumber(),0,false,true);
	_fileWriter.write(&r);

	/*
	while(ptrRunFileShm->read((uint64_t*)(&r))==0) usleep(10);
	std::cout << "HERE1 starting" << std::endl;
	r.print();
	_fileWriter.open(r.runNumber(),0,false,true);
	_fileWriter.write(&r);
	*/
      }
      return true;
    }
    
    bool pausing(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	
	RecordPrinter(&(_ptrFsmInterface->commandPacket().record()));
	RecordPausing r;
	std::cout << "HERE2 pausing" << std::endl;
	r.deepCopy(_ptrFsmInterface->commandPacket().record());
	r.print();
	_fileWriter.write(&r);
      }
      return true;
    }
    
    bool resuming(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	assert(_ptrFsmInterface->commandPacket().record().state()==FsmState::Resuming);
	RecordResuming r;
	std::cout << "HERE2 starting" << std::endl;
	r.deepCopy(_ptrFsmInterface->commandPacket().record());
	r.print();
	_fileWriter.write(&r);
      }
      return true;
    }
    
    bool stopping(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	assert(_ptrFsmInterface->commandPacket().record().state()==FsmState::Stopping);
	RecordStopping r;
	std::cout << "HERE2 starting" << std::endl;
	r.deepCopy(_ptrFsmInterface->commandPacket().record());
	r.print();
	_fileWriter.write(&r);
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

    virtual void configuredB() {
    }
      */
    void running() {
	RecordT<1024> r;
	RecordRunning *rr((RecordRunning*)&r);
	while(_ptrFsmInterface->isIdle()) {
	  ptrRunFileShm->print();
	  if(ptrRunFileShm->read((uint64_t*)(&r))==0) {
	    usleep(10);
	  } else {
	    rr->print();
	    _fileWriter.write(&r);
	  }
	}

	while(ptrRunFileShm->_writePtr>ptrRunFileShm->_readPtr) {
	  //while(ptrRunFileShm->read((uint64_t*)(&r))==0) usleep(10);
	  rr->print();
	  _fileWriter.write(&r);	  
	}
      }
    
      void paused() {
      //sleep(1);
      }
   
  private:
    DataFifoT<6,1024> *ptrRunFileShm;
    FileWriter _fileWriter;
  };

}

#endif
