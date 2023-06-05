#ifndef Hgcal10gLinkReceiver_ProcessorFrontEnd_h
#define Hgcal10gLinkReceiver_ProcessorFrontEnd_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "RecordPrinter.h"
#include "DataFifo.h"

namespace Hgcal10gLinkReceiver {

  class ProcessorFrontEnd : public ProcessorBase {
    
  public:
    ProcessorFrontEnd() {
      _fifoCounter=0;
    }

    virtual ~ProcessorFrontEnd() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      ShmSingleton<RelayWriterDataFifo> shm2;
      //ptrFifoShm2=shm2.setup(fifoKey);
      ptrFifoShm2=nullptr;
      startFsm(rcKey);
    }

    virtual bool initializing() {
      std::string c("cd /home/cmx/rshukla/hgc-engine-tools;python3 roc_control_PD.py --source file --filename initLD-hgcrocv3.csv --dir write --roc s2");
      if(_printEnable) std::cout << "System call = " << c << std::endl;
      system(c.c_str());

      

      return true;
    }

    bool configuring() {
      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      _keyCfgA=r.processorKey(RunControlFrontEndFsmShmKey);
	
      if(_keyCfgA<=40) {
	uint16_t add;
	uint8_t val,msk;

	// Loop over halves
	for(unsigned i(0);i<2;i++) {

	  // Digitial half: Set characterisation mode
	  add=0x564+i*0x5c0;
	  val=0x40;
	  msk=0x40;
	  i2cWriteAndRead(add,val,msk);

	  // Global analogue: unset fixed ADC values
	  add=0x52a+i*0x5c0;
	  val=0x00;
	  msk=0x08;
	  i2cWriteAndRead(add,val,msk);

	  // Halfwise: disconnect all injection capacitors
	  add=0x584+i*0x5c0;
	  val=0x00;
	  msk=0x06;
	  i2cWrite(add,val,msk);
	}
      }
	
      _configuringBCounter=0;

      return true;
    }
    
    bool reconfiguring() {
      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      _keyCfgB=r.processorKey(RunControlFrontEndFsmShmKey);

      if(_keyCfgA>0 && _keyCfgA<=40) {
	uint16_t dac(0),chn(0);
	unsigned dacSteps(8);
	  
	dac=(4096/dacSteps)*(_configuringBCounter%dacSteps);

	if(_keyCfgA<=39) chn=_keyCfgA-1;
	else chn=(_configuringBCounter/dacSteps);

	if(_printEnable) {
	  std::cout << "configuringB() Key A = " << _keyCfgA
		    << ", DAC = " << dac << ", Channel = "
		    << chn << std::endl;
	}
	  
	uint16_t add;
	uint8_t val,msk;

	// Loop over halves
	for(unsigned i(0);i<2;i++) {
	    
	  // Halfwise: disconnect all injection capacitors
	  add=0x584+i*0x5c0;
	  val=0x00;
	  msk=0x06;
	  i2cWrite(add,val,msk);

	  // Set individual channel low range injection capacitor
	  add=0x0004+chn*0x020+i*0x05c0;
	  val=0x02;
	  msk=0x06;
	  i2cWriteAndRead(add,val,msk);
	    
	  // Reference voltage: set DAC value and IntCtest
	  add=0x0506+i*0x05c0;
	  val=dac&0xff;
	  msk=0xff;
	  i2cWriteAndRead(add,val,msk);

	  add=0x0507+i*0x05c0;
	  val=(dac>>8|0x40);
	  msk=0x4f;
	  i2cWriteAndRead(add,val,msk);
	}	  
      }

      _configuringBCounter++;

      return true;
    }

    bool starting() {
      return true;
    }

    bool pausing() {
      return true;
    }
    
    bool resuming() {
      return true;
    }
    
    bool stopping() {
      _eventNumberInConfiguration+=_eventNumberInRun;
      return true;
    }
    
    bool halting() {
      std::string c("cd /home/cmx/rshukla/hgc-engine-tools;python3 roc_control_PD.py --source file --filename initLD-hgcrocv3.csv --dir write --roc s2");
      if(_printEnable) std::cout << "System call = " << c << std::endl;
      system(c.c_str());
	
      return true;
    }
    
    bool resetting() {
      return true;
    }

    bool ending() {
      if(ptrFifoShm2!=nullptr) {
	ptrFifoShm2->end();
	if(_printEnable) {
	  std::cout << "Ending" << std::endl;
	  ptrFifoShm2->print();
	}
      }
      return true;
    }

    //////////////////////////////////////////////

    virtual void configured() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEnd::configured()" << std::endl;
      }
      /*
	RecordConfigured *r;
	while((r=(RecordConfigured*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(10);

	r->setHeader(++_fifoCounter);
	r->setState(FsmState::ConfiguredB);
      
	std::vector<uint32_t> v;
	_serenityTcds2.configuration(v);
	for(unsigned i(0);i<v.size();i++) r->addData32(v[i]);
	if(_printEnable) r->print();

	ptrFifoShm2->writeIncrement();
      */

      writeContinuing();
    }

    void running() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEnd::running()" << std::endl;
      }
      
      writeContinuing();
    }

    void paused() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEnd::paused()" << std::endl;
      }

      writeContinuing();
    }

    void writeContinuing() {
      if(ptrFifoShm2!=nullptr) {
	Record *r;
	while((r=ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->reset(FsmState::Continuing);
	ptrFifoShm2->writeIncrement();
      }
    }
    
    void i2cRead(uint16_t add, uint8_t msk) {
      std::ostringstream sout;
      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control_PD.py"
	   << " --roc s2 --reg 0x"
	   << std::hex << std::setfill('0') << std::setw(4) << add
	   << " --mask 0x" << std::setw(2) << uint16_t(msk);

      if(_printEnable) std::cout << "Read system call = "
				 << sout.str() << std::endl;
      system(sout.str().c_str());
    }
    
    void i2cWrite(uint16_t add, uint8_t val, uint8_t msk) {
      std::ostringstream sout;
      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control_PD.py"
	   << " --roc s2 --reg 0x"
	   << std::hex << std::setfill('0') << std::setw(4) << add
	   << " --mask 0x" << std::setw(2) << uint16_t(msk)
	   << "  --dir write --value " << uint16_t(val);

      if(_printEnable) std::cout << "Write system call = "
				 << sout.str() << std::endl;
      system(sout.str().c_str());
    }
    
    void i2cWriteAndRead(uint16_t add, uint8_t val, uint8_t msk) {
      i2cWrite(add,val,msk);
      i2cRead(add,msk);
    }
        
  protected:
    RelayWriterDataFifo *ptrFifoShm2;

    uint32_t _fifoCounter;

    /*
      uint32_t _evtSeqCounter;
      uint32_t _pauseCounter;

      uint32_t _superRunNumber;
      uint32_t _runNumber;
    */
    uint32_t _keyCfgA;
    uint32_t _keyCfgB;

    uint32_t _configuringBCounter;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;
  };

}

#endif
