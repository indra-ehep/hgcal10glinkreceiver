#ifndef BufferStatus_HH
#define BufferStatus_HH

#include <sys/types.h>
#include <iostream>
#include "RecordHeader.hh"

class BufferStatus {

public:

  BufferStatus() {
    reset();
  }

  void reset() {
    _nIn=0;
    _nOut=0;
    _producerPid=0;
    _consumerPid=0;
    _producerSleep=false;
    _consumerSleep=false;
    _totalProducerSleep=0;
    _totalConsumerSleep=0;

    _runNumber=0;
    _runType=0;
    _runStartTime=0;
    _numberOfConfigurations=0;
    _numberOfSpills=0;
    _numberOfEvents=0;

    _lastRecordHeaderIn=RecordHeader();
    _lastRecordHeaderOut=RecordHeader();
  }

  bool shutDown() const {
    if(_lastRecordHeaderIn.recordType() ==RecordHeader::shutDown &&
       _lastRecordHeaderOut.recordType()==RecordHeader::shutDown)
      return true;
    return false;
  }

  void print(ostream& o) const {
    o << "BufferStatus::print  Producer pid " << _producerPid;
    if(_producerSleep) o << " asleep";
    else               o << " awake ";
    o << "  Time asleep " << _totalProducerSleep 
      << " ms  Records in " << _nIn;
    //    if(_nIn>0) o << " (last buffer " << (_nIn-1)%arraySize << ")";
    o << std::endl;

    o << "                     Consumer pid " << _consumerPid;
    if(_consumerSleep) o << " asleep";
    else               o << " awake ";
    o << "  Time asleep " << _totalConsumerSleep 
      << " ms  Records out " << _nOut;
    //    if(_nOut>0) o << " (last buffer " << (_nOut-1)%arraySize << ")";
    o << std::endl;

    o << "                     Outstanding records " 
      << _nIn-_nOut << std::endl << std::endl;

    if(_runNumber==0) {
      o << "Run not active: Duration since last run ";
    } else {
      o << "Run active: Run Number " << _runNumber << "  Type "
	<< _runType << "  Duration ";
    }

    unsigned dt(time(0)-_runStartTime);
    unsigned dth(dt/3600),dtm((dt%3600)/60),dts(dt%60);
    if(dth<10) o << "0";
    o << dth << ":";
    if(dtm<10) o << "0";
    o << dtm << ":";
    if(dts<10) o << "0";
    o << dts;

    if(_runNumber==0) {
      o << endl;
    } else {
      o << "  Numbers of Configurations " << _numberOfConfigurations 
	<< ", Spills " << _numberOfSpills 
	<< ", Events " << _numberOfEvents << std::endl;
    }

    o << "Header of latest buffer in:" << std::endl;
    _lastRecordHeaderIn.print(o);
    o << "Header of latest buffer out:" << std::endl;
    _lastRecordHeaderOut.print(o);
  }

protected:
  unsigned _nIn, _nOut;
  pid_t _producerPid, _consumerPid;
  bool _producerSleep, _consumerSleep;
  unsigned _totalProducerSleep, _totalConsumerSleep;
  unsigned _runNumber,_runType,_runStartTime;
  unsigned _numberOfConfigurations,_numberOfSpills,_numberOfEvents;
  RecordHeader _lastRecordHeaderIn, _lastRecordHeaderOut;
};

#endif
