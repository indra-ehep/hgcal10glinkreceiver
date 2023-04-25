#ifndef ProcessorDaqLink2_h
#define ProcessorDaqLink2_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"
#include "RecordHeader.h"
#include "FileWriter.h"


namespace Hgcal10gLinkReceiver {

  class ProcessorDaqLink2 : public ProcessorBase {

  public:
    ProcessorDaqLink2() {
    }

    virtual ~ProcessorDaqLink2() {
    }

    void setUpAll(uint32_t rcKey,uint32_t fifoKey) {

      _CB=124;

      ShmSingleton< DataFifoT<6,1024> > shmU;
      shmU.setup(fifoKey);
      ptrRunFileShm=shmU.payload();
      startFsm(rcKey);
    }
    /*    
    void initializing() {
      sleep(1);
    }
    */    
    void configuringA() {
      std::cout << "HERE " << _CB << std::endl;
      _fileWriter.open(_CB,2,true,true);
      RecordHeader h;
      h.setIdentifier(RecordHeader::StateData);
      h.setState(RunControlFsmEnums::ConfiguringA);
      h.setUtc();
      h.setPayloadLength(0);
      h.print();
      _fileWriter.write(&h);
      _CB++;
    }

    void configuringB() {
      RecordHeader h;
      h.setIdentifier(RecordHeader::StateData);
      h.setState(RunControlFsmEnums::ConfiguringB);
      h.setUtc();
      h.setPayloadLength(0);
      _fileWriter.write(&h);
    }
    
    void starting() {
      RecordHeader h;
      h.setIdentifier(RecordHeader::StateData);
      h.setState(RunControlFsmEnums::Starting);
      h.setUtc();
      h.setPayloadLength(0);
      _fileWriter.write(&h);
    }
    
    void pausing() {
      RecordHeader h;
      h.setIdentifier(RecordHeader::StateData);
      h.setState(RunControlFsmEnums::Pausing);
      h.setUtc();
      h.setPayloadLength(0);
      _fileWriter.write(&h);
    }
    
    void resuming() {
      RecordHeader h;
      h.setIdentifier(RecordHeader::StateData);
      h.setState(RunControlFsmEnums::Resuming);
      h.setUtc();
      h.setPayloadLength(0);
      _fileWriter.write(&h);
    }
    
    void stopping() {
      RecordHeader h;
      h.setIdentifier(RecordHeader::StateData);
      h.setState(RunControlFsmEnums::Stopping);
      h.setUtc();
      h.setPayloadLength(0);
      _fileWriter.write(&h);
    }
    
    void haltingB() {
      RecordHeader h;
      h.setIdentifier(RecordHeader::StateData);
      h.setState(RunControlFsmEnums::HaltingB);
      h.setUtc();
      h.setPayloadLength(0);
      _fileWriter.write(&h);
    }

    void haltingA() {
      RecordHeader h;
      h.setIdentifier(RecordHeader::StateData);
      h.setState(RunControlFsmEnums::HaltingA);
      h.setUtc();
      h.setPayloadLength(0);
      _fileWriter.write(&h);
      _fileWriter.close();
    }
    /*
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
      /*
      RecordHeader h;
      h.setIdentifier(RecordHeader::EventData);
      h.setState(RunControlFsmEnums::ConfiguredB);
      h.setLength(20);
      h.setUtc();
      h.print();
      */
      uint64_t buffer[128];
      RecordHeader *h((RecordHeader*)buffer);
      while(ptrRunFileShm->read(buffer)>0) {
	h->print();
	_fileWriter.write(h);
      }
      sleep(1);
      while(ptrRunFileShm->read(buffer)>0) {
	h->print();
	_fileWriter.write(h);
      }
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
    DataFifoT<6,1024> *ptrRunFileShm;
    unsigned _CB;
    FileWriter _fileWriter;
  };

}

#endif
