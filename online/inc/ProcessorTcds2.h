#ifndef Hgcal10gLinkReceiver_ProcessorTcds2_h
#define Hgcal10gLinkReceiver_ProcessorTcds2_h



//#define REMOVE_FOR_TESTING



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
      _serenityTcds2.setThrottle(true);

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
      _strCfgA=nRsa["RunType"].as<std::string>();
	
      if((_keyCfgA>0 && _keyCfgA<=38) || _keyCfgA==123) {

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.calpulse_delay",95);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_physics",0);

	_serenityTcds2.uhalWrite("unpacker0.ctrl_stat.ctrl0.trig_threshold",127,true);
	_serenityTcds2.uhalWrite("unpacker1.ctrl_stat.ctrl0.trig_threshold",127,true);

	// Hardwire CalComing
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",1);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	//_serenityTcds2.uhalWrite("seq_mem.data",(3500<<16)|0x0010); // ECR
	//_serenityTcds2.uhalWrite("seq_mem.data",(   1<<16)|0x0040);
	//_serenityTcds2.uhalWrite("seq_mem.data",(   2<<16)|0x0040);
	//_serenityTcds2.uhalWrite("seq_mem.data",(   3<<16)|0x0040);
	//_serenityTcds2.uhalWrite("seq_mem.data",(   4<<16)|0x0040);
	//_serenityTcds2.uhalWrite("seq_mem.data",(   5<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(3510<<16)|0x0004); // CalComing: L1A BC = this+delay+1

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
	/*
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",5);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	_serenityTcds2.uhalWrite("seq_mem.data",(   1<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(   6<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(  11<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(  16<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(  21<<16)|0x0040);
	*/
      }

      if(_keyCfgA==128) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",5);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	_serenityTcds2.uhalWrite("seq_mem.data",(   1<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",( 701<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(1401<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(2101<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(2801<<16)|0x0040);
      }

      if(_strCfgA=="HgcrocBufferTest") {
	unsigned nL1A(33);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",nL1A);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	for(unsigned i(0);i<nL1A;i++) {
	  _serenityTcds2.uhalWrite("seq_mem.data",((i+1)<<16)|0x0040);
	}
      }

      if(_strCfgA=="CalPulseIntTimeScan") {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.calpulse_delay",30+_configuringBCounter);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",1);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	_serenityTcds2.uhalWrite("seq_mem.data",(3469<<16)|0x0004); // CalComing: L1A BC = this+delay+1
      }

      if(_strCfgA=="BeamRam1TimeScan") { // 133
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_physics",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl2.l1a_physics_mask",2);

	_serenityTcds2.uhalWrite("unpacker0.ctrl_stat.ctrl0.trig_threshold",127,true);
	_serenityTcds2.uhalWrite("unpacker1.ctrl_stat.ctrl0.trig_threshold",127,true);

	_serenityTcds2.uhalWrite("reg_320.ctrl1.ext_trigger_delay",10+(_configuringBCounter%20),true);
      }

      if(_strCfgA=="BeamRam1TimeScan2") { // 134
	_serenityTcds2.uhalWrite("reg_320.ctrl1.ext_trigger_delay",1+(_configuringBCounter%100),true);

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_physics",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_random",0);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl3.l1a_prbs_threshold",0xffffff-0x100);

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl2.l1a_physics_mask",2);

	_serenityTcds2.uhalWrite("unpacker0.ctrl_stat.ctrl0.trig_threshold",127,true);
	_serenityTcds2.uhalWrite("unpacker1.ctrl_stat.ctrl0.trig_threshold",127,true);
      }

      if(_strCfgA=="BeamScintTimeScan") {  // 135
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_physics",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl2.l1a_physics_mask",2);

	_serenityTcds2.uhalWrite("unpacker0.ctrl_stat.ctrl0.trig_threshold",127,true);
	_serenityTcds2.uhalWrite("unpacker1.ctrl_stat.ctrl0.trig_threshold",127,true);
      }

      // Allows playing with triggers by hand
      if(_keyCfgA==200 || _strCfgA=="NoTriggerTest") {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
      }

      // Set all these (except physics) for 5 L1As per orbit
      if(_keyCfgA==201 || _keyCfgA==205) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_physics",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
      }

      if(_keyCfgA==202 || _keyCfgA==205) {
	//_serenityTcds2.uhalWrite("ctrl_stat.ctrl3.l1a_prbs_threshold",0xffffff-91*256);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl3.l1a_prbs_threshold",0xffffff-50);
	//_serenityTcds2.uhalWrite("ctrl_stat.ctrl3.l1a_prbs_threshold",0xffffff-1); // ~2 Hz

	double rateKhz(nRsa["RandomRateKhz"].as<double>());
	if(rateKhz>0.0 && rateKhz<1000.0) _serenityTcds2.uhalWrite("ctrl_stat.ctrl3.l1a_prbs_threshold",0xffffff-uint32_t(419.43*rateKhz+0.5));

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_random",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
      }

      if(_keyCfgA==204 || _keyCfgA==205) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl2.l1a_regular_period",713-1); // 3565 = 5x23x31 = 5x713

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_regular",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
      }

      if(_keyCfgA==203 || _keyCfgA==205) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",6);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	_serenityTcds2.uhalWrite("seq_mem.data",(3562<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(3563<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(3564<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(   1<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(   2<<16)|0x0040);
	_serenityTcds2.uhalWrite("seq_mem.data",(   3<<16)|0x0040);

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",1);
      }

      if(_keyCfgA==209) {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl3.l1a_prbs_threshold",0xffffff-511*256);

	unsigned nL1A(27);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",nL1A);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	for(unsigned i(0);i<nL1A;i++) {
	  _serenityTcds2.uhalWrite("seq_mem.data",((128*i+1)<<16)|0x0040);
	}

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",1);
      }

      if(_keyCfgA==997 || _strCfgA=="EcontTriggerBeamRun") {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_physics",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl2.l1a_physics_mask",1);

	_serenityTcds2.uhalWrite("unpacker0.ctrl_stat.ctrl0.trig_threshold",127,true);
	_serenityTcds2.uhalWrite("unpacker1.ctrl_stat.ctrl0.trig_threshold",50,true);
      }

      if(_keyCfgA==998 || _strCfgA=="BeamAndRandomRun") {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_physics",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl2.l1a_physics_mask",2);

	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_random",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl3.l1a_prbs_threshold",0xffffff-20);

	_serenityTcds2.uhalWrite("unpacker0.ctrl_stat.ctrl0.trig_threshold",127,true);
	_serenityTcds2.uhalWrite("unpacker1.ctrl_stat.ctrl0.trig_threshold",127,true);
      }

      if(_keyCfgA==999 || _strCfgA=="ElectronBeamRun") {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_physics",1);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.en_l1a_software",0);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl2.l1a_physics_mask",2);

	_serenityTcds2.uhalWrite("unpacker0.ctrl_stat.ctrl0.trig_threshold",127,true);
	_serenityTcds2.uhalWrite("unpacker1.ctrl_stat.ctrl0.trig_threshold",127,true);
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
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.calpulse_delay",_configuringBCounter);
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
	  _serenityTcds2.uhalWrite("seq_mem.data",((1+200*i)<<16)|0x0040);
	}
      }

      if(_strCfgA=="HgcrocBufferTest") {
	unsigned nL1A(33+_configuringBCounter);
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",nL1A);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	for(unsigned i(0);i<nL1A;i++) {
	  _serenityTcds2.uhalWrite("seq_mem.data",((i+1)<<16)|0x0040);
	}
      }

      if(_strCfgA=="CalPulseIntTimeScan") {
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.calpulse_delay",30+_configuringBCounter);
	/*
	_serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_length",1);
	_serenityTcds2.uhalWrite("seq_mem.pointer",0);
	_serenityTcds2.uhalWrite("seq_mem.data",(3469<<16)|0x0004); // CalComing: L1A BC = this+delay+1
	*/
      }

      if(_strCfgA=="BeamRam1TimeScan") {
	_serenityTcds2.uhalWrite("reg_320.ctrl1.ext_trigger_delay",10+(_configuringBCounter%20),true);
      }

      if(_strCfgA=="BeamRam1TimeScan2") {
	_serenityTcds2.uhalWrite("reg_320.ctrl1.ext_trigger_delay",1+(_configuringBCounter%100),true);
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
      if(_keyCfgA==209) _serenityTcds2.uhalWrite("ctrl_stat.ctrl.seq_run_ctrl",1);

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
    /*    
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
    */

  protected:
    RelayWriterDataFifo *ptrFifoShm2;

    uint32_t _fifoCounter;

    bool _conForInitialize;
    bool _cfgForRunStart;

    uint32_t _keyCfgA;
    std::string _strCfgA;
    //uint32_t _keyCfgB;

    uint32_t _configuringBCounter;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;

    SerenityTcds2 _serenityTcds2;

  };

}

#endif
