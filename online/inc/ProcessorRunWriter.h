#ifndef Hgcal10gLinkReceiver_ProcessorRunWriter_h
#define Hgcal10gLinkReceiver_ProcessorRunWriter_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include <yaml-cpp/yaml.h>

#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"
#include "RecordHeader.h"
#include "RecordHalting.h"
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
      ShmSingleton<RunWriterDataFifo> shmU;
      _ptrFifoShm=shmU.setup(fifoKey);

      _ptrFifoShm->coldStart();

      startFsm(rcKey);
    }

    void setDummyReader(bool b) {
      _dummyReader=b;
    }
    
    bool initializing() {

      //std::cout << "Initializing flushing FIFO..." << std::endl;
      //_ptrFifoShm->flush();
      //_ptrFifoShm->print();
      
      _ptrFifoShm->initialize();
      _ptrFifoShm->print();
      /*
      while(!_ptrFifoShm->isEmpty()) {
	std::cout << "Flushing FIFO..." << std::endl;
	_ptrFifoShm->initialize();
	_ptrFifoShm->print();
      }
      */
      return true;
    }
    
    bool configuring() {
      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));

      YAML::Node nRsa(YAML::Load(r.string()));
      _relayNumber=nRsa["RelayNumber"].as<uint32_t>();

      if(_relayNumber<0xffffffff) {
	std::ostringstream sout;
	sout << "dat/Relay" << std::setfill('0')
	     << std::setw(10) << _relayNumber;
      _fileWriter.setDirectory(sout.str());
      }
      
      return true;
    }

    bool starting() {
      std::cout << "Starting flushing FIFO..." << std::endl;
      _ptrFifoShm->flush();
      std::cout << "Flushing complete" << std::endl;
      //while(!_ptrFifoShm->isEmpty()) {
      //std::cout << "Flushing FIFO..." << std::endl;
      //_ptrFifoShm->initialize();
      //_ptrFifoShm->print();
      //}

      _eventNumber=0;
	
      _ptrFifoShm->starting();

      assert(_ptrFsmInterface->record().state()==FsmState::Starting);

      const RecordStarting *ry((const RecordStarting*)&(_ptrFsmInterface->record()));
      if(_printEnable) ry->print();
      YAML::Node nRsa(YAML::Load(ry->string()));
      _runNumber=nRsa["RunNumber"].as<uint32_t>();

      RecordStarting r;
      std::cout << "HERE0 starting" << std::endl;
      r.deepCopy(_ptrFsmInterface->record());
      r.print();
      _fileWriter.openRun(_runNumber,_linkNumber);
      _fileWriter.write(&r);

      /*
	while(_ptrFifoShm->read((uint64_t*)(&r))==0) usleep(10);
	std::cout << "HERE1 starting" << std::endl;
	r.print();
	_fileWriter.open(_runNumber,0,false,true);
	_fileWriter.write(&r);
      */

      return true;
    }
    
    bool pausing() {
	
      RecordPrinter(&(_ptrFsmInterface->record()));
      RecordPausing r;
      std::cout << "HERE2 pausing" << std::endl;
      r.deepCopy(_ptrFsmInterface->record());
      r.print();
      //_fileWriter.write(&r); NO!

      return true;
    }
    
    bool resuming() {
      assert(_ptrFsmInterface->record().state()==FsmState::Resuming);
      RecordResuming r;
      std::cout << "HERE2 starting" << std::endl;
      r.deepCopy(_ptrFsmInterface->record());
      r.print();
      //_fileWriter.write(&r); NO!

      return true;
    }
    
    bool stopping() {
      _ptrFifoShm->print();

      assert(_ptrFsmInterface->record().state()==FsmState::Stopping);
      RecordStopping r;
      std::cout << "HERE2 stopping" << std::endl;
      r.deepCopy(_ptrFsmInterface->record());
      r.setNumberOfEvents(_eventNumber);
      r.print();
      _fileWriter.write(&r);
      _fileWriter.close();

      _ptrFifoShm->print();
      return true;
    }    

    bool halting() {
      _ptrFifoShm->initialize();
      _ptrFifoShm->print();
      return true;
    }    

    bool resetting() {
      _ptrFifoShm->print();
      RecordResetting r;
      if(_printEnable) r.print();
      _fileWriter.write(&r);
      _fileWriter.close();
      _ptrFifoShm->print();
      return true;
    }

    void configured() {
      _ptrFifoShm->print();
    }

    void halted() {
      _ptrFifoShm->print();
      
      /*      
	      _ptrFsmInterface->idle();
	      while(_ptrFsmInterface->isIdle()) {
	      usleep(1000);
	      }
	      _ptrFifoShm->print();

	      if(_ptrFifoShm->readable()>0) {
	      const Record *rrr;
	      std::cout << "HALTED" << std::endl;
	      rrr=_ptrFifoShm->readRecord();
	      rrr->print();
	      }
      */
    }

    void running() {
      //RecordT<1024> r;
      //RecordRunning *rr((RecordRunning*)&r);
      const Record *rrr;

      _ptrFsmInterface->setProcessState(FsmState::Running);
      
      //while(_ptrFsmInterface->isIdle()) {
      while(_ptrFsmInterface->systemState()==FsmState::Running) {
	//if(_printEnable) _ptrFifoShm->print();
	
	if(_ptrFifoShm->readable()==0) {
	  if(_dummyReader) _ptrFifoShm->writeIncrement();
	  //if(_ptrFifoShm->read((uint64_t*)(&r))==0) {
	  usleep(10);
	} else {
	  rrr=_ptrFifoShm->readRecord();
	  //if(_printEnable) rrr->RecordHeader::print();

	  _fileWriter.write(rrr);
	  _ptrFifoShm->readIncrement();
	  if(_dummyReader) _ptrFifoShm->writeIncrement();
	  _eventNumber++;
	}
      }

      std::cout << std::endl << "Finished loop, checking for other events, currently "
		<< _eventNumber << std::endl;
      _ptrFsmInterface->print();
      _ptrFifoShm->print();

#ifdef FIRST_ATTEMPT
      //while(_ptrFifoShm->_writePtr>_ptrFifoShm->_readPtr) {
      while(_ptrFifoShm->readable()>0) {
	//if(_printEnable) _ptrFifoShm->print();
	
	//assert(_ptrFifoShm->read((uint64_t*)(&r))>0);
	rrr=_ptrFifoShm->readRecord();
	//if(_printEnable) rrr->RecordHeader::print();

	_fileWriter.write(rrr);
	_ptrFifoShm->readIncrement();
	//if(_dummyReader) _ptrFifoShm->writeIncrement();
	_eventNumber++;
      }
	
      std::cout << std::endl << "Finished loop 1, re-checking for other events, currently "
		<< _eventNumber << std::endl;
      _ptrFsmInterface->print();
	
      //usleep(100000);
      //usleep(1000000);
      sleep(10);
      
      while(_ptrFifoShm->readable()>0) {
	//if(_printEnable) _ptrFifoShm->print();
	
	rrr=_ptrFifoShm->readRecord();
	//if(_printEnable) rrr->RecordHeader::print();

	_fileWriter.write(rrr);
	_ptrFifoShm->readIncrement();	
	//if(_dummyReader) _ptrFifoShm->writeIncrement();
	_eventNumber++;
      }

      std::cout << std::endl << "Finished loop 2, no more checking for other events, currently "
		<< _eventNumber << std::endl;
      _ptrFsmInterface->print();
#endif

      usleep(10000);

      unsigned nLoop(0);
      while(_ptrFifoShm->readable()>0) {
	while(_ptrFifoShm->readable()>0) {
	  rrr=_ptrFifoShm->readRecord();
	  _fileWriter.write(rrr);
	  _ptrFifoShm->readIncrement();
	  _eventNumber++;
	}

	nLoop++;
	_ptrFifoShm->print();
	usleep(1000);
      }

      std::cout << std::endl << "Finished loop " << nLoop
		<< ", no more checking for other events, currently "
		<< _eventNumber << std::endl;
      _ptrFifoShm->print();
    }
   
  private:
    RunWriterDataFifo *_ptrFifoShm;
    FileWriter _fileWriter;
    unsigned _linkNumber;
    unsigned _eventNumber;
    unsigned _runNumber;
    unsigned _relayNumber;

    bool _dummyReader;
  };

}

#endif
