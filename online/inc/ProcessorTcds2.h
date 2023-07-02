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
      return true;
    }

    bool configuring() {
      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      _configuringBCounter=0;

      _keyCfgA=r.processorKey(RunControlTcds2FsmShmKey);
	
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
	std::cout << "L1A BC = " << l1aBc << std::endl;

	_serenityTcds2.uhalWrite("seq_mem.data",(l1aBc<<16)|0x0040);
      }


      return true;
    }
    
    bool reconfiguring() {
      RecordReconfiguring &r((RecordReconfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      _configuringBCounter++;

      _keyCfgB=r.processorKey(RunControlTcds2FsmShmKey);

      if(_keyCfgA==123) {
	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",_keyCfgB);
      }
	
      if(_keyCfgA==125) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",1);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);

	uint16_t l1aBc(((3548+(_configuringBCounter%32))%3564)+1);
	std::cout << "L1A BC = " << l1aBc << std::endl;

	_serenityTcds2.uhalWrite("seq_mem.data",(l1aBc<<16)|0x0040);
      }

      return true;
    }

    bool starting() {

      // Enable sequencer (even if masked)
      _serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_run_ctrl",3);

      // Do resets
      _serenityTcds2.sendEbr();
      _serenityTcds2.sendEcr();
      _serenityTcds2.sendOcr();

      // Release throttle
      _serenityTcds2.uhalWrite("ctrl_stat.ctrl2.tts_tcds2",1);
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

      // Impose throttle
      _serenityTcds2.uhalWrite("ctrl_stat.ctrl2.tts_tcds2",0);
	
      // Disable sequencer
      _serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_run_ctrl",0);



      return true;
    }
    
    bool halting() {
      return true;
    }
    
    bool resetting() {
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

      RecordYaml *r;
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(1000);
      r->setHeader(++_fifoCounter);
      r->setState(FsmState::Halted);
      
      // Replace with "constants" call to SerenityTcds2
      YAML::Node n;
      n["Source"]="TCDS2";
      n["PayloadVersion"]=_serenityTcds2.payloadVersion();
      n["ElectronicsId"]=0x0fffffff;

      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());

      if(_printEnable) r->print();
      ptrFifoShm2->writeIncrement();
      
      writeContinuing();
      if(_printEnable) ptrFifoShm2->print();
    }

    virtual void configured() {
      if(_printEnable) {
	std::cout << "ProcessorTcds2::configured()" << std::endl;
      }

      RecordYaml *r;
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(100);

      r->setHeader(++_fifoCounter);
      r->setState(FsmState::Configured);
      
      //std::vector<uint32_t> v;
      //_serenityTcds2.configuration(v);
      //for(unsigned i(0);i<v.size();i++) r->addData32(v[i]);

      //r->setType(RecordConfigured::TCDS2);
      //r->setLocation(0);

      //std::unordered_map<std::string,uint32_t> m;
      //_serenityTcds2.configuration(m);

      YAML::Node n;
      n["Source"]="TCDS2";
      n["ElectronicsId"]=0x0fffffff;

      //n["Configuration"]=0;
      //_serenityTcds2.configuration(n["Configuration"]);
      
      YAML::Node nc;
      _serenityTcds2.configuration(nc);
      n["Configuration"]=nc;

      std::cout << "Yaml" << std::endl << n << std::endl;

      /*
      if(_printEnable) {
	std::cout << "Serenity configuration" << std::endl;
	_serenityTcds2.print();
	
	std::cout << "Serenity map" << std::endl;
	for(auto i(m.begin());i!=m.end();i++) {
	  std::cout << " " << i->first << " = " << i->second << std::endl;
	}
      }
      */
      
      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());

      //r->setConfiguration(m);
      /*      
      if(_printEnable) {
	std::unordered_map<std::string,uint32_t> m2;
	r->configuration(m2);
        
	std::cout << "Record configuration unpacked" << std::endl;
	for(auto i(m2.begin());i!=m2.end();i++) {
	  std::cout << " " << i->first << " = " << i->second << std::endl;
	}
      }
      */
      if(_printEnable) r->print();

      /*
	r->addData32(_serenityTcds2.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl"));
	r->addData32(_serenityTcds2.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl1"));
	r->addData32(_serenityTcds2.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2"));
	r->addData32(_serenityTcds2.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl3"));

	// Sequencer
	uint32_t length(_serenityTcds2.uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length"));
	r->addData32(length);

	_serenityTcds2.uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",0);
	for(unsigned i(0);i<length;i++) {
	r->addData32(_serenityTcds2.uhalRead("payload.fc_ctrl.tcds2_emu.seq_mem.data"));
	}
      */

      ptrFifoShm2->writeIncrement();

      writeContinuing();
    }

    void running() {
      if(_printEnable) {
	std::cout << "ProcessorTcds2::running()" << std::endl;
      }
      
      writeContinuing();
    }

    void paused() {
      if(_printEnable) {
	std::cout << "ProcessorTcds2::paused()" << std::endl;
      }

      writeContinuing();
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

    SerenityTcds2 _serenityTcds2;

  };

}

#endif
