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

    bool initializing() {
      sleep(1);
      return true;
    }
    
    bool configuringA(FsmInterface::Handshake s) {
      sleep(1);
      return true;
    }
    
    bool configuringB(FsmInterface::Handshake s) {
      sleep(1);
      return true;
    }
    
    bool starting(FsmInterface::Handshake s) {
      sleep(1);
      return true;
    }
    
    bool pausing(FsmInterface::Handshake s) {
      sleep(1);
      return true;
    }
    
    bool resuming(FsmInterface::Handshake s) {
      sleep(1);
      return true;
    }
    
    bool stopping(FsmInterface::Handshake s) {
      sleep(1);
      return true;
    }
    
    bool haltingB(FsmInterface::Handshake s) {
      sleep(1);
      return true;
    }
    
    bool haltingA(FsmInterface::Handshake s) {
      sleep(1);
      return true;
    }
    
    bool resetting(FsmInterface::Handshake s) {
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
