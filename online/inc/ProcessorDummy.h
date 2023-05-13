#ifndef Hgcal10gLinkReceiver_ProcessorDummy_h
#define Hgcal10gLinkReceiver_ProcessorDummy_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"

namespace Hgcal10gLinkReceiver {

  class ProcessorDummy : public ProcessorBase {

  public:
    ProcessorDummy() {
    }

    bool initializing(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool configuringA(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool configuringB(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool starting(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool pausing(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool resuming(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool stopping(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool haltingB(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool haltingA(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }
    
    bool resetting(FsmInterface::HandshakeState s) {
      sleep(1);
      return true;
    }

    //////////////////////////////////////////////
    
    void initial() {
      sleep(1);
    }
    
    void halted() {
      sleep(1);
    }
    
    void configuredA() {
      sleep(1);
    }
    
    void configuredB() {
      sleep(1);
    }
    
    void running() {
      sleep(1);
    }
    
    void paused() {
      sleep(1);
    }
   
  private:

  };

}

#endif
