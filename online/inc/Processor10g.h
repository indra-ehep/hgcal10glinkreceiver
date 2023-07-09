#ifndef Hgcal10gLinkReceiver_Processor10g_h
#define Hgcal10gLinkReceiver_Processor10g_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include <yaml-cpp/yaml.h>

#include "Serenity10g.h"
#include "Serenity10gx.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"


#include "I2cInstruction.h"
#include "UhalInstruction.h"
#include "RecordConfigured.h"
#include "RecordYaml.h"


#include "RecordPrinter.h"

//#ifdef ProcessorHardware
//#include "uhal/uhal.hpp"
//#include "uhal/ValMem.hpp"
//#endif

namespace Hgcal10gLinkReceiver {

  class Processor10g : public ProcessorBase {
    
  public:
    Processor10g() {
      _fifoCounter=0;
    }

    virtual ~Processor10g() {
    }
  
    void setUpAll(uint32_t rcKey) {
      _serenity10g.makeTable();
      _serenity10gx.makeTable();
      ptrFifoShm2=nullptr;
      startFsm(rcKey);
    }

    virtual bool initializing() {
      _serenity10gx.setDefaults();
      _serenity10g.setDefaults();

      _serenity10g.print();
      _serenity10gx.print();
      return true;
    }

    bool configuring() {
	_configuringBCounter=0;

	RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
	if(_printEnable) r.print();

	//_keyCfgA=r.processorKey(RunControlDummyFsmShmKey);
	YAML::Node nRsa(YAML::Load(r.string()));
	_keyCfgA=nRsa["ProcessorKey"].as<uint32_t>();
	
	if(_keyCfgA==123) {
	  /*
	  // Do configuration; ones which could have been changed
	  _serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",30);
	  
	  // Hardwire CalComing
	  _serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length",2);
	  _serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",0);
	  _serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.data",(3500<<16)|0x0010);
	  //_serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",1);
	  _serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.data",(3510<<16)|0x0004);
	  */
	}

	_serenity10g.print();

	if(_keyCfgA==124) {
	  _serenity10gx.uhalWrite("ctrl.reg.duty_cycle",_configuringBCounter%4);
	}


      return true;
    }
    
    bool reconfiguring() {
	_configuringBCounter++;

	RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
	if(_printEnable) r.print();

	//_keyCfgB=r.processorKey(RunControlDummyFsmShmKey);

	if(_keyCfgA==123) {
	  //_serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",_keyCfgB);
	  //_serenity10gx.uhalWrite("ctrl.reg.duty_cycle",3);
	}
	
	if(_keyCfgA==124) {
	  _serenity10gx.uhalWrite("ctrl.reg.duty_cycle",_configuringBCounter%4);
	}

      return true;
    }

    bool starting() {
      // Enable sequencer (even if masked)
      //_serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_run_ctrl",3);
      
      // Release throttle
      //_serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",1);

      //_serenity10g.uhalWrite("payload.ctrl.reg.en",1);

      return true;
    }

    bool pausing() {
      //_serenity10g.uhalWrite("payload.ctrl.reg.en",0);
      return true;
    }
    
    bool resuming() {
      //_serenity10g.uhalWrite("payload.ctrl.reg.en",1);
      return true;
    }
    
    bool stopping() {
      //_serenity10g.uhalWrite("payload.ctrl.reg.en",0);
      _eventNumberInConfiguration+=_eventNumberInRun;

      // Impose throttle
      //_serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",0);
      
      // Disable sequencer
      //_serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_run_ctrl",0);
      
      return true;
    }
    
    bool halting() {
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
	std::cout << "Processor10g::configured()" << std::endl;
      }

        if(ptrFifoShm2!=nullptr) {

      RecordConfigured *r;
      while((r=(RecordConfigured*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(10);

      r->setHeader(++_fifoCounter);
      r->setState(FsmState::Configured);
      
      std::vector<uint32_t> v;
      _serenity10g.configuration(v);
      for(unsigned i(0);i<v.size();i++) r->addData32(v[i]);
      if(_printEnable) r->print();

      /*
      r->addData32(_serenity10g.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl"));
      r->addData32(_serenity10g.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl1"));
      r->addData32(_serenity10g.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2"));
      r->addData32(_serenity10g.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl3"));

      // Sequencer
      uint32_t length(_serenity10g.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length"));
      r->addData32(length);

      _serenity10g.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",0);
      for(unsigned i(0);i<length;i++) {
	r->addData32(_serenity10g.uhalRead("payload.fc_ctrl.tcds2_emu.seq_mem.data"));
      }
      */

      ptrFifoShm2->writeIncrement();
	}
      writeContinuing();
    }

    void running() {
      if(_printEnable) {
	std::cout << "Processor10g::running()" << std::endl;
      }

      _serenity10gx.uhalWrite("ctrl.reg.en",1);

      _ptrFsmInterface->setProcessState(FsmState::Running);
      
      while(_ptrFsmInterface->systemState()==FsmState::Running) usleep(1000);

      _serenity10gx.uhalWrite("ctrl.reg.en",0);
      
      writeContinuing();
    }

    void paused() {
      if(_printEnable) {
	std::cout << "Processor10g::paused()" << std::endl;
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
    
    void keyConfiguration(uint32_t key) {

      switch(key) {
	
      case 0: {
	break;
      }
	
      case 123: {
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

      Serenity10g _serenity10g;
      Serenity10gx _serenity10gx;

    };

  }

#endif
