#ifndef ProcessorFastControl_h
#define ProcessorFastControl_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "RunFileShm.h"
#include "RecordHeader.h"


namespace Hgcal10gLinkReceiver {

  class ProcessorFastControl : public ProcessorBase {

  public:
    ProcessorFastControl() {
    }

    void setUpBuffer() {
      ShmSingleton<RunFileShm2> shmU;
      shmU.setup(ProcessorFastControlDataShmKey);
      ptrRunFileShm=shmU.payload();
    }
    /*    
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
    */    
    void running(RunControlFsmShm* const p) {
      RecordHeader h;
      h.setIdentifier(RecordHeader::EventData);
      h.setState(RunControlFsmEnums::Running);
      h.setLength(20);
      h.setUtc();
      h.print();
      
      std::memcpy(ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm2::BufferDepth],
		  &h,8*h.length());
      ptrRunFileShm->_writePtr++;
      //sleep(10);
      //p->setRequest(RunControlFsmEnums::Stop);
    }
    
    void paused() {
      sleep(1);
    }
   
  private:
    RunFileShm2 *ptrRunFileShm;

  };

}

#endif
