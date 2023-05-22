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
      std::string c("cd /home/cmx/rshukla/hgc-engine-tools;python3 roc_control.py --source file --filename initLD-hgcrocv3.csv --dir write --roc s2");
      if(_printEnable) std::cout << "System call = " << c << std::endl;
      system(c.c_str());
      return true;
    }

    bool configuringA() {
	RecordConfiguringA &r((RecordConfiguringA&)(_ptrFsmInterface->record()));
	if(_printEnable) r.print();

	_keyCfgA=r.processorKey(RunControlFrontEndFsmShmKey);
	
	if(_keyCfgA<=38) {
	  uint16_t add,val;
	  for(unsigned i(0);i<2;i++) {

	    // Unset characterisation mode
	    {
	      add=0x564+i*0x5c0;
	      val=0x00;
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --dir write --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add << " --value " << val
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }
	    {
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }


	    // Remove fixed ADC values
	    {
	      add=0x52a+i*0x5c0;
	      val=0x00;
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --dir write --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add << " --value " << val
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }
	    {
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }


	    // Disconnect all injection capacitors
	    {
	      add=0x584+i*0x5c0;
	      val=0x00;
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --dir write --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add << " --value " << val
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }
	  }
	}
	
	_configuringBCounter=0;

      return true;
    }
    
    bool configuringB() {
	RecordConfiguringB &r((RecordConfiguringB&)(_ptrFsmInterface->record()));
	if(_printEnable) r.print();

	_keyCfgB=r.processorKey(RunControlFrontEndFsmShmKey);

	if(_keyCfgA<=38) {
	  uint16_t dac(0),chn(0);
	  uint16_t add(0),val(0);
	  for(unsigned i(0);i<2;i++) {
	    
	    if(_keyCfgA<=37) {
	      dac=128*(_configuringBCounter%8);
	      chn=_keyCfgA-1;
	    } else if(_keyCfgA==38) {
	      dac=128*(_configuringBCounter%8);
	      chn=(_keyCfgB/8);
	    }

	    // Disconnect all injection capacitors
	    {
	      add=0x584+i*0x5c0;
	      val=0x00;
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --dir write --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add << " --value " << val
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }

	    // Set channel injection capacitor
	    {
	      add=0x0004+i*0x05c0+chn*0x020;
	      val=0x02;
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --dir write --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add << " --value " << val
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }
	    {
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }

	    // Set DAC value
	    {
	      add=0x0506+i*0x05c0;
	      val=dac&0xff;
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --dir write --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add << " --value " << val
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }
	    {
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }

	    {
	      add=0x0507+i*0x05c0;
	      val=(dac>>8|0x40);
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --dir write --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add << " --value " << val
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }
	    {
	      std::ostringstream sout;
	      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control.py --reg 0x" 
		   << std::hex << std::setfill('0') << std::setw(4) << add
		   << " --roc s2";
	      if(_printEnable) std::cout << "System call = " << sout.str() << std::endl;
	      system(sout.str().c_str());
	    }
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
    
    bool haltingB() {
      return true;
    }
    
    bool haltingA() {
      std::string c("cd /home/cmx/rshukla/hgc-engine-tools;python3 roc_control.py --source file --filename initLD-hgcrocv3.csv --dir write --roc s2");
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


    virtual void configuredA() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEnd::configuredA()" << std::endl;
      }
      
      writeContinuing();
    }

    virtual void configuredB() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEnd::configuredB()" << std::endl;
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
