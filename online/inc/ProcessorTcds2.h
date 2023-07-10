#ifndef Hgcal10gLinkReceiver_ProcessorTcds2_h
#define Hgcal10gLinkReceiver_ProcessorTcds2_h

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

//#ifdef ProcessorHardware
//#include "uhal/uhal.hpp"
//#include "uhal/ValMem.hpp"
//#endif

namespace Hgcal10gLinkReceiver {

  class ProcessorTcds2 : public ProcessorBase {
    
  public:
    ProcessorTcds2() {
      _fifoCounter=0;
      _cfgForRunStart=false;
      _conForInitialize=true;
    }

    virtual ~ProcessorTcds2() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      _serenityTcds2.makeTable();

      ShmSingleton<RelayWriterDataFifo> shm2;
      ptrFifoShm2=shm2.setup(fifoKey);

      startFsm(rcKey);
    }

    virtual bool initializing() {
      _serenityTcds2.setDefaults();
      if(_printEnable) _serenityTcds2.print();

      ///////////////////////////////////////////////////////
      
      _conForInitialize=true;

      RecordYaml *r;
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(1000);
      r->setHeader(++_fifoCounter);
      r->setState(FsmState::Constants);
	
      // Replace with "constants" call to SerenityTcds2
      YAML::Node n;
      n["Source"]="TCDS2";
      n["PayloadVersion"]=_serenityTcds2.payloadVersion();
      n["ElectronicsId"]=0x0fffffff;
	
      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());
	
      if(_printEnable) r->print();

      if(_printEnable) ptrFifoShm2->print();
      ptrFifoShm2->writeIncrement();

      writeContinuing();

      return true;
    }

    bool configuring() {
      _cfgForRunStart=true;
      _configuringBCounter=0;

      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      //_keyCfgA=r.processorKey(RunControlTcds2FsmShmKey);
      YAML::Node nRsa(YAML::Load(r.string()));
      _keyCfgA=nRsa["ProcessorKey"].as<uint32_t>();
	
      if((_keyCfgA>0 && _keyCfgA<=38) || _keyCfgA==123) {

	// Do configuration; ones which could have been changed
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",30);
	  
	// Hardwire CalComing
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length",2);
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",0);
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.data",(3500<<16)|0x0010);
	//_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",1);
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.data",(3510<<16)|0x0004);
      }

      if(_keyCfgA==125) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",1);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);

	uint16_t l1aBc(((3548+(_configuringBCounter%32))%3564)+1);
	if(_printEnable) std::cout << "L1A BC = " << l1aBc << std::endl;

	_serenityTcds2.uhalWrite("seq_mem.data",(l1aBc<<16)|0x0040);
      }

      if(_keyCfgA==126) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",1);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	_serenityTcds2.uhalWrite("seq_mem.data",(   1<<16)|0x0040);
	//_serenityTcds2.uhalWrite("seq_mem.data",(1783<<16)|0x0040);
	
	//_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",0);
      }


      return true;
    }
    
    bool reconfiguring() {
      _cfgForRunStart=true;

      RecordReconfiguring &r((RecordReconfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      _configuringBCounter++;

      //_keyCfgB=r.processorKey(RunControlTcds2FsmShmKey);

      if(_keyCfgA==123) {
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",_configuringBCounter);
      }
	
      if(_keyCfgA==125) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",1);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);

	uint16_t l1aBc(((3548+(_configuringBCounter%32))%3564)+1);
	std::cout << "L1A BC = " << l1aBc << std::endl;

	_serenityTcds2.uhalWrite("seq_mem.data",(l1aBc<<16)|0x0040);
      }

      if(_keyCfgA==126) {
        _serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",1+_configuringBCounter);
        _serenityTcds2.uhalWrite("seq_mem.pointer",0);
	for(unsigned i(0);i<1+_configuringBCounter;i++) {
	  _serenityTcds2.uhalWrite("seq_mem.data",((1+i)<<16)|0x0040);
	}
      }

      return true;
    }

    bool starting() {

      // Configuration at run start
      RecordYaml *r; 
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(1000);

      r->setHeader(++_fifoCounter);
      r->setState(FsmState::Configuration);
     
      YAML::Node n;
      n["Source"]="TCDS2";
      n["ElectronicsId"]=0x0fffffff;
      
      YAML::Node nc;
      _serenityTcds2.configuration(nc);
      n["Configuration"]=nc;

      std::cout << "Yaml configuration" << std::endl << n << std::endl;

      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());

      if(_printEnable) r->print();
      
      ptrFifoShm2->writeIncrement();

      writeContinuing();

      /////////////////////////////////////////////////////
      
      // Do counter resets
      _serenityTcds2.sendEbr();
      _serenityTcds2.sendEcr();
      _serenityTcds2.sendOcr();

      // Enable sequencer (even if masked)
      _serenityTcds2.resetSequencer();
      _serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_run_ctrl",3);

      // Reset event counters
      _serenityTcds2.resetCounters();

      // Release throttle
      _serenityTcds2.setThrottle(false);

      return true;
    }

    bool pausing() {
      return true;
    }
    
    bool resuming() {
      return true;
    }
    
    bool stopping() {
      _cfgForRunStart=false;

      _eventNumberInConfiguration+=_eventNumberInRun;

      // Impose throttle
      _serenityTcds2.setThrottle(true);
	
      // Disable sequencer
      _serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_run_ctrl",0);

      /////////////////////////////////////////////////////
      
      // Status at run end
      RecordYaml *r; 
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(1000);
	
      r->setHeader(++_fifoCounter);
      r->setState(FsmState::Status);
      
      YAML::Node n;
      n["Source"]="TCDS2";
      n["ElectronicsId"]=0x0fffffff;
      
      YAML::Node ns;
      _serenityTcds2.status(ns);
      n["Status"]=ns;
      
      if(_printEnable) std::cout << "Yaml status" << std::endl << n << std::endl;
      
      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());
      
      if(_printEnable) r->print();
      
      ptrFifoShm2->writeIncrement();
      
      writeContinuing();
      return true;
    }
    
    bool halting() {
      _serenityTcds2.setDefaults();
      if(_printEnable) _serenityTcds2.print();

      _conForInitialize=false;
      return true;
    }
    
    bool resetting() {
      _conForInitialize=false;
      _cfgForRunStart=false;
      return true;
    }

    bool ending() {
      ptrFifoShm2->end();
      if(_printEnable) {
	std::cout << "Ending" << std::endl;
        ptrFifoShm2->print();
      }
      return true;
    }

    //////////////////////////////////////////////

    virtual void halted() {
      if(_printEnable) {
	std::cout << "ProcessorTcds2::halted()" << std::endl;
      }
    }

    virtual void configured() {
      if(_printEnable) {
	std::cout << "ProcessorTcds2::configured()" << std::endl;
      }
    }

    void running() {
      if(_printEnable) {
	std::cout << "ProcessorTcds2::running()" << std::endl;
      }
    }

    void paused() {
      if(_printEnable) {
	std::cout << "ProcessorTcds2::paused()" << std::endl;
      }
    }

    void writeContinuing() {
      Record *r;
      while((r=ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      r->reset(FsmState::Continuing);
      if(_printEnable) {
	std::cout << "writeContinuing()" << std::endl;
	r->print();
      }
      ptrFifoShm2->writeIncrement();
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
    RelayWriterDataFifo *ptrFifoShm2;

    uint32_t _fifoCounter;

    bool _conForInitialize;
    bool _cfgForRunStart;

    uint32_t _keyCfgA;
    //uint32_t _keyCfgB;

    uint32_t _configuringBCounter;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;

    SerenityTcds2 _serenityTcds2;

  };

}

#endif
