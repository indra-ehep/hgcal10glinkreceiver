#ifndef Hgcal10gLinkReceiver_ProcessorFastControl_h
#define Hgcal10gLinkReceiver_ProcessorFastControl_h




//#define REMOVE_FOR_TESTING




#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "SerenityEncoder.h"
#include "SerenityLpgbt.h"
#include "SerenityMiniDaq.h"
#include "Serenity10g.h"
#include "Serenity10gx.h"
#include "SerenityTrgDaq.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"



#include "I2cInstruction.h"
#include "UhalInstruction.h"
#include "RecordConfigured.h"



#include "RecordPrinter.h"
#include "RecordHalted.h"

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class ProcessorFastControl : public ProcessorBase {
    
  public:
    ProcessorFastControl() {      
      std::system("empbutler -c etc/connections.xml do x0 info");
      daqBoard=3;
    }

    virtual ~ProcessorFastControl() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      _serenityEncoder.makeTable();
      _serenityLpgbt.makeTable();
      _serenityMiniDaq.makeTable();
      _serenity10g.makeTable();
      //_serenity10gx.makeTable();
      _serenityTrgDaq.makeTable();
      
      ShmSingleton<RelayWriterDataFifo> shm2;
      ptrFifoShm2=shm2.setup(fifoKey);
      startFsm(rcKey);
    }

    virtual bool initializing() {
      _serenityEncoder.setDefaults();
      _serenityEncoder.print();

      _serenityLpgbt.setDefaults();
      _serenityLpgbt.print();
      
      _serenityMiniDaq.setDefaults();
      _serenityMiniDaq.print();

      _serenity10g.setDefaults();
      _serenity10g.print();

      //_serenity10gx.setDefaults();
      //_serenity10gx.print();

      _serenityTrgDaq.setDefaults();
      _serenityTrgDaq.print();

      ///////////////////////////////////////////////////////
      
      RecordYaml *r;
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(1000);
      r->setHeader(_cfgSeqCounter++);
      r->setState(FsmState::Constants);
      
      // Replace with "constants" call to SerenityTcds2
      YAML::Node n;
      n["Source"]="Serenity";
      n["HardwareVersion"]="PrototypeV1.1";
      n["PayloadVersion"]=_serenityEncoder.payloadVersion();
      n["ElectronicsId"]=daqBoard<<22|0x3fffff;
       
      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());
	
      if(_printEnable) r->print();

      if(_printEnable) ptrFifoShm2->print();
      ptrFifoShm2->writeIncrement();

      writeContinuing();

      _serenityTrgDaq.uhalWrite("trigger_ro.SLink.source_id"  ,0xce000000|daqBoard<<4|0,true);
      _serenityEncoder.uhalWrite("DAQ_SLink_readout.source_id",0xce000000|daqBoard<<4|1,true);

      return true;
    }

    bool configuring() {
      if(_printEnable) {
	std::cout << "Configuring" << std::endl;
	RecordPrinter(_ptrFsmInterface->record());
	std::cout << std::endl;
      }

      if(_checkEnable) {
	if(_ptrFsmInterface->record().state()!=FsmState::Configuring) {
	  std::cerr << "State does not match" << std::endl;
	  std::cout << "State does not match" << std::endl;
	  if(_assertEnable) assert(false);
	}
      }
      
      //assert(_ptrFsmInterface->commandPacket().record().state()==FsmState::Configuring);

      _cfgSeqCounter=1;
      _evtSeqCounter=1;
      _configuringBCounter=0;

      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      //_keyCfgA=r.processorKey(RunControlTcds2FsmShmKey);
      YAML::Node nRsa(YAML::Load(r.string()));
      _keyCfgA=nRsa["ProcessorKey"].as<uint32_t>();

      if(_keyCfgA==125) {
	_serenityEncoder.uhalWrite("ctrl.l1a_stretch",_configuringBCounter/32);
      }

      /*
	if(_keyCfgA==126) {
	_serenityLpgbt.uhalWrite("ctrl.user_bx",3);
	_serenityLpgbt.uhalWrite("fc_cmd.user",0xa9);
	_serenityLpgbt.uhalWrite("ctrl.loop_user_cmd",1);
	}
      */
      // Do configuration; ones which could have been changed
      //_serenityEncoder.uhalWrite("calpulse_ctrl.calpulse_int_del",8);


 	if(_keyCfgA==127 || _keyCfgA==128) {
	  _serenityMiniDaq.setNumberOfEconds(2);
	}
      /*
	RecordConfiguringA *r;
	while((r=(RecordConfiguringA*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);

	if(_printEnable) {
	std::cout << "Got buffer Record pointer" << std::endl;
	std::cout << std::endl;
	}

	r->deepCopy(_ptrFsmInterface->commandPacket().record());
	_superRunNumber=r->superRunNumber();

	_runNumberInSuperRun=0;
	_eventNumberInSuperRun=0;

	
	if(_checkEnable) {
	if(!r->valid()) {
	std::cerr << "Record is not valid" << std::endl;
	std::cout << "Record is not valid" << std::endl;
	r->print();
	if(_assertEnable) assert(false);
	}
	}
	
	if(_printEnable) {
	std::cout << "Releasing Record in buffer" << std::endl;
	r->print();
	std::cout << std::endl;
	}

	ptrFifoShm2->writeIncrement();
	//rca.print();
	//assert(ptrFifoShm2->write(rca.totalLength(),(uint64_t*)(&rca)));
	*/
      return true;
    }
    
    bool reconfiguring() {
      _configuringBCounter++;

      if(_keyCfgA==125) {
	_serenityEncoder.uhalWrite("ctrl.l1a_stretch",_configuringBCounter/32);
      }

      /*
	RecordConfiguringB rcb;
	rcb.deepCopy(_ptrFsmInterface->commandPacket().record());

	_eventNumberInConfiguration=0;


	rcb.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rcb.totalLength(),(uint64_t*)(&rcb)));
      */

      return true;
    }

    bool starting() {
      /*
	_pauseCounter=0;
	_eventNumberInRun=0;

	RecordStarting *r;
	while((r=(RecordStarting*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(10);
	r->deepCopy(_ptrFsmInterface->commandPacket().record());

	_runNumber=r->runNumber();
	if(_printEnable) r->print();
	ptrFifoShm2->writeIncrement();

	//ptrFifoShm2->print();
	//assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
	*/

      RecordYaml *ry;
      while((ry=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      ry->setHeader(_cfgSeqCounter++);
      ry->setState(FsmState::Configuration);
      ry->print();
      
      YAML::Node total;
      total["Source"]="Serenity";
      total["DaqBoard"]=daqBoard;
      total["ElectronicsId"]=daqBoard<<22|0x3fffff;

      YAML::Node ne;
      _serenityEncoder.configuration(ne);
      total["FcEncoder"]=ne;

      YAML::Node ntd;
      _serenityTrgDaq.configuration(ntd);
      total["TriggerDaq"]=ntd;
      
      YAML::Node n10g;
      _serenity10g.configuration(n10g);
      total["Eth10G"]=n10g;
      
      total["LpgbtPair"]["0"]["Id"]=0;

      YAML::Node nm;
      _serenityMiniDaq.configuration(nm);
      total["LpgbtPair"]["0"]["MiniDaq"]=nm;

      YAML::Node nl;
      _serenityLpgbt.configuration(nl);
      total["LpgbtPair"]["0"]["FcStream"]=nl;
      
      std::ostringstream sout;
      sout << total;
      ry->setString(sout.str());
      ry->print();

      ptrFifoShm2->writeIncrement();

      writeContinuing();

      ///////////////////////////////////////////////////
      
      //_serenity10gx.uhalWrite("ctrl.reg.en",1);

      _serenityMiniDaq.reset();
      _serenityEncoder.resetSlinkFifo();

      _serenityEncoder.resetDaqReadout();
      _serenityEncoder.resetTrgReadout();

      _serenity10g.reset();

      usleep(2*1000000);
      return true;
    }

    bool pausing() {
      /* NOT FOR Shm2
	 RecordPausing rr;
	 rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	 rr.print();
	 ptrFifoShm2->print();
	 assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      */
      return true;
    }
    
    bool resuming() {
      /*
	RecordResuming rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      */
      return true;
    }
    
    bool stopping() {
      //_serenity10gx.uhalWrite("ctrl.reg.en",0);
      /*
	_eventNumberInConfiguration+=_eventNumberInRun;

	RecordStopping *r;
	while((r=(RecordStopping*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(10);
	r->deepCopy(_ptrFsmInterface->commandPacket().record());

	r->setNumberOfEvents(_eventNumberInRun);
	r->setNumberOfPauses(_pauseCounter);
	if(_printEnable) r->print();
	ptrFifoShm2->writeIncrement();

	//RecordStopping rr;
	//rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	//rr.setNumberOfEvents(_eventNumberInRun);
	//rr.setNumberOfPauses(_pauseCounter);
	//rr.print();
	//ptrFifoShm2->print();
	//assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
	*/

      /////////////////////////////////////////////////////
      
      // Status at run end
      RecordYaml *r; 
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(1000);
	
      r->setHeader(_cfgSeqCounter++);
      r->setState(FsmState::Status);
      
      YAML::Node total;
      total["Source"]="Serenity";
      total["DaqBoard"]=daqBoard;
      total["ElectronicsId"]=daqBoard<<22|0x3fffff;

      YAML::Node ne;
      _serenityEncoder.status(ne);
      total["FcEncoder"]=ne;

      total["LpgbtPair"]["0"]["Id"]=0;

      YAML::Node nm;
      _serenityMiniDaq.status(nm);
      total["LpgbtPair"]["0"]["MiniDaq"]=nm;

      if(_printEnable) std::cout << "Yaml status" << std::endl << total << std::endl;
      
      std::ostringstream sout;
      sout << total;
      r->setString(sout.str());
      
      if(_printEnable) r->print();
      
      ptrFifoShm2->writeIncrement();
      
      writeContinuing();

      return true;
    }

    bool halting() {
      _serenityEncoder.setDefaults();
      _serenityEncoder.print();
      
      _serenityLpgbt.setDefaults();
      _serenityLpgbt.print();
      
      _serenityMiniDaq.setDefaults();
      _serenityMiniDaq.print();

      //_serenity10g.setDefaults();
      //_serenity10g.print();

      //_serenity10gx.setDefaults();
      //_serenity10gx.print();

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
      /*
      RecordHalted *r;
      if(_printEnable) {
	std::cout << "halted() waiting for record" << std::endl;
	ptrFifoShm2->print();
      }
      while((r=(RecordHalted*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      r->setHeader(_cfgSeqCounter++);

      // Replace with "constants" call to Serenity
      YAML::Node n;
      n["Source"]="Serenity";
      n["HardwareVersion"]="PrototypeV1.1";
      n["PayloadVersion"]=_serenityEncoder.payloadVersion();
      n["ElectronicsId"]=daqBoard<<22|0x3fffff;
       
      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());
      r->print();
      
      ptrFifoShm2->writeIncrement();

      writeContinuing();
      */
    }
    
    virtual void configured() {

      std::cout << "configured() relay = " << _relayNumber << std::endl;

#ifdef HGCROC_JUNK
      RecordConfigured *r;
      for(unsigned i(1);i<=3 && false;i++) {

	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::Configured);
	r->setType(RecordConfigured::HGCROC);
	r->setLocation(0xfe00+i);
	if(_printEnable) r->print();
      
	I2cInstruction i2c;
	
	std::ostringstream sout;
	sout << "HgcrocCfg_ROCs" << i << ".cfg";
	
	std::ifstream fin;
	fin.open(sout.str().c_str());
	if(fin) {
	  char buffer[16];
	  
	  uint16_t add;
	  uint16_t val;
	  uint8_t mask(0xff);

	  std::cout << std::hex << std::setfill('0');

	  fin.getline(buffer,16);
	  while(fin) {
	    assert(buffer[ 1]=='x');
	    assert(buffer[10]=='x');	

	    buffer[ 6]='\0';
	    buffer[13]='\0';
	  
	    std::istringstream sAdd(buffer+ 2);
	    std::istringstream sVal(buffer+11);
	  
	    sAdd >> std::hex >> add;
	    sVal >> std::hex >> val;

	    i2c.setAddress(add);
	    i2c.setMask(mask);
	    i2c.setValue(val&0xff);
	    if(_printEnable) {
	      i2c.print();
	  
	      std::cout << "HGCROC cfg address 0x" << std::setw(4) << add
			<< ", mask 0x" << std::setw(2) << unsigned(mask)
			<< ", value 0x" << std::setw(2) << unsigned(val)
			<< std::endl;
	    }
	    
	    r->addData32(i2c.data());
	    if(_printEnable && add<16) r->print();
	  
	    fin.getline(buffer,16);
	  }
	
	  std::cout << std::dec << std::setfill(' ');
	}
	fin.close();
	if(_printEnable) r->print();

	ptrFifoShm2->writeIncrement();
      }
#endif

#ifdef USE_CONFIGURED
      RecordYaml *ry;
      while((ry=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      ry->setHeader(_cfgSeqCounter++);
      ry->setState(FsmState::Configured);
      ry->print();
      
      YAML::Node total;
      total["Source"]="Serenity";
      total["DaqBoard"]=daqBoard;
      total["ElectronicsId"]=daqBoard<<22|0x3fffff;

      YAML::Node ne;
      _serenityEncoder.configuration(ne);
      total["FcEncoder"]=ne;

      total["LpgbtPair"]["0"]["Id"]=0;

      YAML::Node nm;
      //nm[0]="NULL";
      _serenityMiniDaq.configuration(nm);
      total["LpgbtPair"]["0"]["MiniDaq"]=nm;

      YAML::Node nl;
      _serenityLpgbt.configuration(nl);
      total["LpgbtPair"]["0"]["FcStream"]=nl;
      /*
	total["LpgbtPair"][0]["FcStream"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream0"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream0"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream1"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream1"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream1"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream2"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream2"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream2"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream3"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream3"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream3"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream4"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream4"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream4"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream5"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream5"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream5"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream6"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream6"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream6"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream7"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream7"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream7"]["ctrl.calpulse_type"]=0;
      */
      total["LpgbtPair"]["0"]["Unpacker"]["0"]["Id"]=0;
      total["LpgbtPair"]["0"]["Unpacker"]["1"]["Id"]=1;
      total["LpgbtPair"]["0"]["Unpacker"]["2"]["Id"]=2;
      total["LpgbtPair"]["0"]["Unpacker"]["3"]["Id"]=3;


      total["LpgbtPair"]["1"]["Id"]=1;

      total["LpgbtPair"]["1"]["MiniDaq"]="XXX: YYY";

      total["LpgbtPair"]["1"]["FcStream0"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream0"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream0"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream1"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream1"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream1"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream2"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream2"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream2"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream3"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream3"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream3"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream4"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream4"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream4"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream5"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream5"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream5"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream6"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream6"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream6"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream7"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream7"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream7"]["ctrl.calpulse_type"]=0;

      total["LpgbtPair"]["1"]["Unpacker"]["0"]["Id"]=0;
      total["LpgbtPair"]["1"]["Unpacker"]["1"]["Id"]=1;
      total["LpgbtPair"]["1"]["Unpacker"]["2"]["Id"]=2;
      total["LpgbtPair"]["1"]["Unpacker"]["3"]["Id"]=3;


      YAML::Node lp;
      lp["cmd"]=0x36;
      //_serenityLpgbt.configuration(lp);


      YAML::Node md;
      md["header"]=0x154;
      //_serenityLpgbt.configuration(md);
      //total["LpgbtPair"][1]=md;
      //total["MiniDaq"]=md;

      /*
	YAML::Node pair0;
	pair0["FcStream_0"]="Stuff0";
	total["LpgbtPair"][2]=pair0;

	YAML::Node pair1;
	pair1["FcStream_1"]="Stuff1";
	total["LpgbtPair"][3]=pair1;
      
	YAML::Node unp;
	unp["Unpacker_0"]="Stuff0";
	total["Unpacker"][0]=unp;
	unp["Unpacker_1"]="Stuff1";
	total["Unpacker"][1]=unp;
      */
      //total["LpgbtPair"][0]=lp;
      //total["LpgbtPair"][1]=md;

      
      std::ostringstream sout;
      sout << total;
      ry->setString(sout.str());
      ry->print();

      ptrFifoShm2->writeIncrement();
            
#endif
    }

    virtual void running() {
      _serenityEncoder.uhalWrite("ctrl.tts",1);

      _ptrFsmInterface->setProcessState(FsmState::Running);

      while(_ptrFsmInterface->systemState()==FsmState::Running) usleep(1000);
      
      _serenityEncoder.uhalWrite("ctrl.tts",0);
    }
    
    void paused() {
      _pauseCounter++;
    }

    void writeContinuing() {
      Record *r;
      while((r=ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
      r->reset(FsmState::Continuing);
      ptrFifoShm2->writeIncrement();
    }

    
  protected:
    RelayWriterDataFifo *ptrFifoShm2;

    unsigned daqBoard;

    uint32_t _cfgSeqCounter;
    uint32_t _evtSeqCounter;
    uint32_t _pauseCounter;

    uint32_t _configuringBCounter;
    uint32_t _keyCfgA;

    uint32_t _relayNumber;
    uint32_t _runNumber;

    uint32_t _runNumberInSuperRun;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;

    SerenityEncoder _serenityEncoder;
    SerenityLpgbt _serenityLpgbt;
    SerenityMiniDaq _serenityMiniDaq;  
    Serenity10g _serenity10g;
    Serenity10gx _serenity10gx;
    SerenityTrgDaq _serenityTrgDaq;
  };

}

#endif
