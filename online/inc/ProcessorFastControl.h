#ifndef ProcessorFastControl_h
#define ProcessorFastControl_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"
#include "RecordHeader.h"


namespace Hgcal10gLinkReceiver {

  class ProcessorFastControl : public ProcessorBase {

  public:
    ProcessorFastControl() {
    }

    virtual ~ProcessorFastControl() {
    }

    void setUpAll(uint32_t rcKey, uint32_t fifoKey2,
		  uint32_t fifoKey0, uint32_t fifoKey1) {
      _CB=0;
      
      ShmSingleton< DataFifoT<6,1024> > shm0;
      shm0.setup(fifoKey0);
      ptrFifoShm0=shm0.payload();
      ShmSingleton< DataFifoT<6,1024> > shm1;
      shm1.setup(fifoKey1);
      ptrFifoShm1=shm1.payload();

      setUpAll(rcKey,fifoKey2);
    }
    
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      ShmSingleton< DataFifoT<6,1024> > shm2;
      shm2.setup(fifoKey);
      ptrFifoShm2=shm2.payload();
      startFsm(rcKey);
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
    */    

    virtual void configuredB() {
      //assert(false);
      _CB++;
      std::cout << "HERE " << _CB << std::endl;

      RecordHeader h;
      h.setIdentifier(RecordHeader::EventData);
      h.setState(RunControlFsmEnums::ConfiguredB);
      h.setPayloadLength(_CB);
      h.setUtc();
      h.print();

      assert(ptrFifoShm2->write(h.totalLength(),(uint64_t*)(&h)));
      sleep(1);
    }

    /*
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
    */
   
  private:
    DataFifoT<6,1024> *ptrFifoShm2;
    DataFifoT<6,1024> *ptrFifoShm0;
    DataFifoT<6,1024> *ptrFifoShm1;
    unsigned _CB;
  };

}

#endif
