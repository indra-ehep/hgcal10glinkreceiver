#ifndef RecordBuffer_HH
#define RecordBuffer_HH

/***********************************************************************
 RecordBuffer - Buffer between reader and writer to store records
                after VME readout but before being written to disk.
***********************************************************************/

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <ctime>
#include <iostream>

#include "Record.hh"
#include "BufferStatus.hh"

#define DUMMYPID 123456789
#define SLEEPPERIOD 100     // in millisec

void RecordBufferSignalHandler(int signal) {
  //  std::cout << "Process " << getpid() << " received signal " 
  //  << signal << std::endl;
}


class RecordBuffer : public BufferStatus {

public:

  RecordBuffer() : BufferStatus() {
    reset();
  }

  bool setProducerPid(bool dummy=false) {
    if(_producerPid!=0) return false;

    if(dummy) {
      _producerPid=DUMMYPID;
    } else {
      _producerPid=getpid();
      signal(SIGUSR1,RecordBufferSignalHandler);
    }

    return true;
  }
  
  void zeroProducerPid() {
    _producerPid=0;
    signal(SIGUSR1,SIG_DFL);
  }
  
  bool dummyProducerPid() {
    if(_producerPid==DUMMYPID) return true;
    return false;
  }
  
  bool setConsumerPid(bool dummy=false) {
    if(_consumerPid!=0) return false;

    if(dummy) {
      _consumerPid=DUMMYPID;
    } else {
      _consumerPid=getpid();
      signal(SIGUSR2,RecordBufferSignalHandler);
    }

    return true;
  }
  
  bool dummyConsumerPid() {
    if(_consumerPid==DUMMYPID) return true;
    return false;
  }
  
  void zeroConsumerPid() {
    _consumerPid=0;
    signal(SIGUSR2,SIG_DFL);
  }
  
  Record* getNewEvent() {
    while(_nIn>=(_nOut+arraySize) && _consumerPid!=DUMMYPID) {
      _producerSleep=true;
      _totalProducerSleep+=SLEEPPERIOD-usleep(1000*SLEEPPERIOD);
      _producerSleep=false;
    }
    return (Record*)array[_nIn%arraySize];
  }

  void releaseNewEvent() {
    _lastRecordHeaderIn=*((RecordHeader*)array[_nIn%arraySize]);

    switch (_lastRecordHeaderIn.recordType()) {
    case RecordHeader::startOfRun:
      _runNumber=_lastRecordHeaderIn.runNumber();
      _runType=_lastRecordHeaderIn.runType();
      _runStartTime=_lastRecordHeaderIn.recordTime();
      _numberOfConfigurations=0;
      _numberOfSpills=0;
      _numberOfEvents=0;
      break;
    case RecordHeader::startOfConfiguration:
      _numberOfConfigurations++;
      break;
    case RecordHeader::startOfSpill:
      _numberOfSpills++;
      break;
    case RecordHeader::event:
      _numberOfEvents++;
      break;
    case RecordHeader::endOfRun:
      _runNumber=0;
      _runType=0;
      _runStartTime=_lastRecordHeaderIn.recordTime();
      _numberOfConfigurations=0;
      _numberOfSpills=0;
      _numberOfEvents=0;
      break;
    default:
      break;
    };

    _nIn++;
    
    if(_consumerPid==DUMMYPID) {
      _lastRecordHeaderOut=_lastRecordHeaderIn;
      _nOut=_nIn;
    }

    if(_consumerSleep) kill(_consumerPid,SIGUSR2);
  }

  Record* getOldEvent() {
    while(_nOut>=_nIn && _producerPid!=DUMMYPID) {
      _consumerSleep=true;
      _totalConsumerSleep+=SLEEPPERIOD-usleep(1000*SLEEPPERIOD);
      _consumerSleep=false;
    }
    return (Record*)array[_nOut%arraySize];
  }

  void releaseOldEvent() {
    _lastRecordHeaderOut=*((RecordHeader*)array[_nOut%arraySize]);
    _nOut++;

    if(_producerPid==DUMMYPID) {
      _lastRecordHeaderIn=*((RecordHeader*)array[_nOut%arraySize]);
      _nIn=_nOut+1;
    }

    if(_producerSleep) kill(_producerPid,SIGUSR1);
  }
  
  void print(ostream &o, unsigned printLevel=0) const {
    BufferStatus::print(o);

    if(printLevel>1) {
      for(unsigned i(0);i<arraySize;i++) {
	unsigned *p((unsigned*)(array+i));
	std::cout << "Buffer ";
	if(i<10) std::cout << " ";
	std::cout << i << " header";
	for(unsigned j(0);j<6;j++) std::cout << " " << p[j];
	std::cout << std::endl;
      }
    }
  }


private:
  enum {
    arraySize = 32
  };
  
  unsigned array[arraySize][8192];
};

#endif
