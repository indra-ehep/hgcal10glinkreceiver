#ifndef Hgcal10gLinkReceiver_ProcessorRunWriter_h
#define Hgcal10gLinkReceiver_ProcessorRunWriter_h

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

  class ProcessorRunWriter : public ProcessorBase {

  public:
    ProcessorRunWriter(unsigned l) {
      _linkNumber=l;
      _dummyReader=false;
    }

    virtual ~ProcessorRunWriter() {
    }

    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      ShmSingleton< DataFifoT<6,1024> > shmU;
      ptrRunFileShm=shmU.setup(fifoKey);
      //ptrRunFileShm=shmU.payload();

      ptrRunFileShm->coldStart();

      startFsm(rcKey);
    }

    void setDummyReader(bool b) {
      _dummyReader=b;
    }
    
    bool initializing() {
      return true;
    }
    
    bool starting(FsmInterface::Handshake s) {
      if(s==FsmInterface::GoToTransient) {
	_eventNumber=0;
	
	ptrRunFileShm->starting();

	assert(_ptrFsmInterface->record().state()==FsmState::Starting);
	RecordStarting r;
	std::cout << "HERE0 starting" << std::endl;
	r.deepCopy(_ptrFsmInterface->record());
	r.print();
	_fileWriter.openRun(r.runNumber(),_linkNumber);
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
    
    bool pausing(FsmInterface::Handshake s) {
      if(s==FsmInterface::GoToTransient) {
	
	RecordPrinter(&(_ptrFsmInterface->record()));
	RecordPausing r;
	std::cout << "HERE2 pausing" << std::endl;
	r.deepCopy(_ptrFsmInterface->record());
	r.print();
	//_fileWriter.write(&r); NO!
      }
      return true;
    }
    
    bool resuming(FsmInterface::Handshake s) {
      if(s==FsmInterface::GoToTransient) {
	assert(_ptrFsmInterface->record().state()==FsmState::Resuming);
	RecordResuming r;
	std::cout << "HERE2 starting" << std::endl;
	r.deepCopy(_ptrFsmInterface->record());
	r.print();
	//_fileWriter.write(&r); NO!
      }
      return true;
    }
    
    bool stopping(FsmInterface::Handshake s) {
      if(s==FsmInterface::GoToTransient) {
	assert(_ptrFsmInterface->record().state()==FsmState::Stopping);
	RecordStopping r;
	std::cout << "HERE2 stopping" << std::endl;
	r.deepCopy(_ptrFsmInterface->record());
	r.setNumberOfEvents(_eventNumber);
	r.print();
	_fileWriter.write(&r);
	_fileWriter.close();
      }
      return true;
    }    

    bool resetting(FsmInterface::Handshake s) {
      if(s==FsmInterface::GoToTransient) {
	RecordResetting r;
	r.print();
	_fileWriter.write(&r);
	_fileWriter.close();
      }
      return true;
    }

    void running() {
      //RecordT<1024> r;
      //RecordRunning *rr((RecordRunning*)&r);
      const Record *rrr;

      _ptrFsmInterface->idle();
      
      while(_ptrFsmInterface->isIdle()) {
	//if(_printEnable) ptrRunFileShm->print();
	
	if(ptrRunFileShm->readable()==0) {
	  if(_dummyReader) ptrRunFileShm->writeIncrement();
	  //if(ptrRunFileShm->read((uint64_t*)(&r))==0) {
	  usleep(10);
	} else {
	  rrr=ptrRunFileShm->readRecord();
	  //if(_printEnable) rrr->RecordHeader::print();

	  _fileWriter.write(rrr);
	  ptrRunFileShm->readIncrement();
	  if(_dummyReader) ptrRunFileShm->writeIncrement();
	  _eventNumber++;
	}
      }

      _ptrFsmInterface->print();
      ptrRunFileShm->print();
      std::cout << "Finished loop, checking for other events" << std::endl;
	
      //while(ptrRunFileShm->_writePtr>ptrRunFileShm->_readPtr) {
      while(ptrRunFileShm->readable()>0) {
	//if(_printEnable) ptrRunFileShm->print();
	
	//assert(ptrRunFileShm->read((uint64_t*)(&r))>0);
	rrr=ptrRunFileShm->readRecord();
	//if(_printEnable) rrr->RecordHeader::print();

	_fileWriter.write(rrr);
	ptrRunFileShm->readIncrement();
	//if(_dummyReader) ptrRunFileShm->writeIncrement();
	_eventNumber++;
      }
	
      usleep(100000);
	
      while(ptrRunFileShm->readable()>0) {
	//if(_printEnable) ptrRunFileShm->print();
	
	rrr=ptrRunFileShm->readRecord();
	//if(_printEnable) rrr->RecordHeader::print();

	_fileWriter.write(rrr);
	ptrRunFileShm->readIncrement();	
	//if(_dummyReader) ptrRunFileShm->writeIncrement();
	_eventNumber++;
      }
    }
   
  private:
    DataFifoT<6,1024> *ptrRunFileShm;
    FileWriter _fileWriter;
    unsigned _linkNumber;
    unsigned _eventNumber;

    bool _dummyReader;
  };

}

#endif
