#ifndef ProcessorDummy_h
#define ProcessorDummy_h

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

    void initializing() {
      sleep(1);
    }
    
    void configuringA() {
      sleep(1);
    }
    
    void configuringB() {
      sleep(1);
    }
    
    void starting() {
      sleep(1);
    }
    
    void pausing() {
      sleep(1);
    }
    
    void resuming() {
      sleep(1);
    }
    
    void stopping() {
      sleep(1);
    }
    
    void haltingB() {
      sleep(1);
    }
    
    void haltingA() {
      sleep(1);
    }
    
    void resetting() {
      sleep(1);
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
    
    void running(RunControlFsmShm* const p) {
      sleep(1);
    }
    
    void paused() {
      sleep(1);
    }
   
  private:

  };

}

#endif
