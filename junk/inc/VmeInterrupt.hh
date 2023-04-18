#ifndef VmeInterrupt_HH
#define VmeInterrupt_HH

#include <sys/types.h>
#include <signal.h>

#include <iostream>

void VmeInterruptSignalHandler(int signal) {
  //    std::cout << "Process " << getpid() << " received signal " 
  //	    << signal << std::endl;
}

class VmeInterrupt {

public:

  VmeInterrupt() : _trigger(false), _spill(false) {
  }

  void reset() {
    _producerPid=0;
    _consumerPid=0;
    _trigger=false;
    _spill=false;
  }

  bool trigger() const {
    return _trigger;
  }

  void trigger(bool t) {
    _trigger=t;
  }

  bool spill() const {
    return _spill;
  }

  void spill(bool s) {
    _spill=s;
  }

  bool setProducerPid() {
    if(_producerPid!=0) return false;
    signal(SIGUSR1,SIG_IGN);
    _producerPid=getpid();
  }

  void enableProducerSignal() {
    signal(SIGUSR1,VmeInterruptSignalHandler);
  }

  void disableProducerSignal() {
    signal(SIGUSR1,SIG_IGN);
  }

  void signalConsumer() {
    if(_consumerPid!=0) kill(_consumerPid,SIGUSR2);
  }

  bool setConsumerPid() {
    if(_consumerPid!=0) return false;
    signal(SIGUSR2,SIG_IGN);
    _consumerPid=getpid();
  }
  
  void enableConsumerSignal() {
    signal(SIGUSR2,VmeInterruptSignalHandler);
  }

  void disableConsumerSignal() {
    signal(SIGUSR2,SIG_IGN);
  }

  void signalProducer() {
    if(_producerPid!=0) kill(_producerPid,SIGUSR1);
  }


 void print(ostream &o) {
   o << "VmeInterrupt::print  Pids = " 
     << _producerPid << " " << _consumerPid << "  Trigger ";
    if(_trigger) o << "true ";
    else         o << "false";
    o << "  Spill ";
    if(_spill) o << "true ";
    else       o << "false";
    o << std::endl;
  }
  
private:
  pid_t _producerPid, _consumerPid;
  bool _trigger,_spill;
};

#endif
