#ifndef ProcessorTcds2_h
#define ProcessorTcds2_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "SerenityTcds2.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"



#include "I2cInstruction.h"
#include "UhalInstruction.h"
#include "RecordConfigured.h"



#include "RecordPrinter.h"
#include "ShmKeys.h"

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class ProcessorTcds2 : public ProcessorBase {
    
  public:
    ProcessorTcds2() {
    }

    virtual ~ProcessorTcds2() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      _serenityTcds2.makeTable();

      ShmSingleton< DataFifoT<6,1024> > shm2;
      ptrFifoShm2=shm2.setup(fifoKey);

      startFsm(rcKey);
    }

    virtual bool initializing(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_serenityTcds2.setDefaults();
	_serenityTcds2.print();
      }
	
      return true;
    }

    bool configuringA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordConfiguringA &r((RecordConfiguringA&)(_ptrFsmInterface->commandPacket().record()));
	if(_printEnable) r.print();

	_keyCfgA=r.processorKey(RunControlTcds2FsmShmKey);
	
	if(_keyCfgA==123) {

	  // Do configuration; ones which could have been changed
	  _serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",30);
	  
	  // Hardwire CalComing
	  _serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length",1);
	  _serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",0);
	  _serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.data",(3510<<16)|0x0004);
	}
	
	_configuringBCounter=0;
      }

      return true;
    }
    
    bool configuringB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordConfiguringB &r((RecordConfiguringB&)(_ptrFsmInterface->commandPacket().record()));
	if(_printEnable) r.print();

	_keyCfgB=r.processorKey(RunControlTcds2FsmShmKey);

	if(_keyCfgA==123) {
	  _serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",_keyCfgB);
	}
	
	_configuringBCounter++;
      }
      return true;
    }

    bool starting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {

	// Enable sequencer (even if masked)
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_run_ctrl",3);

	// Release throttle
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",1);
      }

      return true;
    }

    bool pausing(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
      }
      return true;
    }
    
    bool resuming(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
      }
      return true;
    }
    
    bool stopping(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_eventNumberInConfiguration+=_eventNumberInRun;

	// Impose throttle
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",0);
	
	// Disable sequencer
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_run_ctrl",0);

      }
      return true;
    }
    
    bool haltingB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
      }
      return true;
    }
    
    bool haltingA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
      }
      return true;
    }
    
    bool resetting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
      }
      return true;
    }

    bool ending(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
      }
      return true;
    }

    //////////////////////////////////////////////


    virtual void configuredB() {
      /* 
	 SHOULD CAPTURE ALL CONFIG VALUES AND SHIP OUT?
      */
    }

    void paused() {
    }
    
    void keyConfiguration(uint32_t key) {

      switch(key) {
	
      case 0: {
	break;
      }
	
      case 123: {
	//_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",20+(_runNumberInSuperRun%50));

				 break;
				 }
	
	  case 999: {
	    break;
	  }
	
	default: {
	  break;
	}

      };
      }
    
    protected:
      DataFifoT<6,1024> *ptrFifoShm2;

      /*
      uint32_t _cfgSeqCounter;
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

      SerenityTcds2 _serenityTcds2;

    };

  }

#endif
