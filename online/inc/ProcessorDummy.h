#ifndef Hgcal10gLinkReceiver_ProcessorDummy_h
#define Hgcal10gLinkReceiver_ProcessorDummy_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

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
    
    bool configuring() {
      sleep(1);
      return true;
    }
    
    bool reconfiguring() {
      sleep(1);
      return true;
    }
    
    bool starting() {
      sleep(1);
      return true;
    }
    
    bool pausing() {
      sleep(1);
      return true;
    }
    
    bool resuming() {
      sleep(1);
      return true;
    }
    
    bool stopping() {
      sleep(1);
      return true;
    }
    
    bool halting() {
      sleep(1);
      return true;
    }
    
    bool resetting() {
      sleep(1);
      return true;
    }

    bool ending() {
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
    
    void configured() {
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
