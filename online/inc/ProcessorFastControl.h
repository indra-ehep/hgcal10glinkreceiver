#ifndef Hgcal10gLinkReceiver_ProcessorFastControl_h
#define Hgcal10gLinkReceiver_ProcessorFastControl_h

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
#ifdef ProcessorHardware
#ifdef JUNK
  ProcessorFastControl() : lConnectionFilePath("etc/connections.xml"),
      lDeviceId("x0"),
      lConnectionMgr("file://" + lConnectionFilePath),
      lHW(lConnectionMgr.getDevice(lDeviceId)) {
      
      uhal::setLogLevelTo(uhal::Error());  
      //lHW = lConnectionMgr.getDevice(lDeviceId);
    }
#endif
#else
    ProcessorFastControl() {
    }
#endif

    virtual ~ProcessorFastControl() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      _serenityEncoder.makeTable();
      _serenityLpgbt.makeTable();
      _serenityMiniDaq.makeTable();

      ShmSingleton<RelayWriterDataFifo> shm2;
      ptrFifoShm2=shm2.setup(fifoKey);
      //ptrFifoShm2=shm2.payload();
      startFsm(rcKey);
    }

    virtual bool initializing() {
      _serenityEncoder.setDefaults();
      _serenityEncoder.print();
      _serenityLpgbt.setDefaults();
      _serenityLpgbt.print();
      _serenityMiniDaq.setDefaults();
      _serenityMiniDaq.print();
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


      // Do configuration; ones which could have been changed
      _serenityEncoder.uhalWrite("calpulse_ctrl.calpulse_int_del",8);
	
      if(_printEnable) {
	std::cout << "Waiting for space in buffer" << std::endl;
	ptrFifoShm2->print();
	std::cout << std::endl;
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

      return true;
    }

    bool halting() {
      _serenityEncoder.setDefaults();
      _serenityEncoder.print();
      _serenityLpgbt.setDefaults();
      _serenityLpgbt.print();
      _serenityMiniDaq.setDefaults();
      _serenityMiniDaq.print();
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
      RecordHalted *r;
      while((r=(RecordHalted*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
      r->setHeader(_cfgSeqCounter++);

      // Replace with "constants" call to Serenity
      YAML::Node n;
      n["Serenity"]=0;
      n["PayloadVersion"]=_serenityEncoder.payloadVersion();
      
      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());
      r->print();
      
      //ptrFifoShm2->writeIncrement(); // NOT YET!
    }
    
    virtual void configured() {

      std::cout << "configured() relay = " << _relayNumber << std::endl;

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

      RecordYaml *ry;
      while((ry=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(100);
      ry->setHeader(_cfgSeqCounter++);
      ry->print();
      
      YAML::Node total;
      total["Serenity"]=0;

      YAML::Node ne;
      _serenityEncoder.configuration(ne);
      total["FcEncoder"]=ne;

      total["LpgbtPair"][0]["Id"]=0;

      YAML::Node nm;
      _serenityMiniDaq.configuration(nm);
      total["LpgbtPair"][0]["MiniDaq"]=nm;

      YAML::Node nl;
      _serenityLpgbt.configuration(nl);
      total["LpgbtPair"][0]["FcStream"]=nl;
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
      total["LpgbtPair"][0]["Unpacker"][0]["Id"]=0;
      total["LpgbtPair"][0]["Unpacker"][1]["Id"]=1;
      total["LpgbtPair"][0]["Unpacker"][2]["Id"]=2;
      total["LpgbtPair"][0]["Unpacker"][3]["Id"]=3;


      total["LpgbtPair"][1]["Id"]=1;

      total["LpgbtPair"][1]["MiniDaq"]="XXX: YYY";

      total["LpgbtPair"][1]["FcStream0"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"][1]["FcStream0"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"][1]["FcStream0"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"][1]["FcStream1"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"][1]["FcStream1"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"][1]["FcStream1"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"][1]["FcStream2"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"][1]["FcStream2"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"][1]["FcStream2"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"][1]["FcStream3"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"][1]["FcStream3"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"][1]["FcStream3"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"][1]["FcStream4"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"][1]["FcStream4"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"][1]["FcStream4"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"][1]["FcStream5"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"][1]["FcStream5"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"][1]["FcStream5"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"][1]["FcStream6"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"][1]["FcStream6"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"][1]["FcStream6"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"][1]["FcStream7"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"][1]["FcStream7"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"][1]["FcStream7"]["ctrl.calpulse_type"]=0;

      total["LpgbtPair"][1]["Unpacker"][0]["Id"]=0;
      total["LpgbtPair"][1]["Unpacker"][1]["Id"]=1;
      total["LpgbtPair"][1]["Unpacker"][2]["Id"]=2;
      total["LpgbtPair"][1]["Unpacker"][3]["Id"]=3;


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
      
      
      for(unsigned i(1);i<=3;i++) {
	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(100);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::Configured);
	r->setType(RecordConfigured::BE);
	r->setLocation(i);

	std::unordered_map<std::string,uint32_t> m;
	YAML::Node n;
	if(i==1) {
	  _serenityEncoder.configuration(m);
	  _serenityEncoder.configuration(n);
	} else if(i==2) {
	  _serenityLpgbt.configuration(m);
	} else {
	  _serenityMiniDaq.configuration(m);
	}
	
	if(_printEnable) {
	  std::cout << "Serenity configuration" << std::endl;
	  if(i==1) _serenityEncoder.print();
	  else if(i==2) _serenityLpgbt.print();
	  else _serenityMiniDaq.print();
	    
	  std::cout << "Serenity map" << std::endl;
	  for(auto i(m.begin());i!=m.end();i++) {
	    std::cout << " " << i->first << " = " << i->second << std::endl;
	  }
	  std::cout << "Serenity node" << std::endl << n << std::endl;
	  for(auto i(n.begin());i!=n.end();i++) {
	    std::cout << " " << i->first << " = " << i->second << std::endl;
	  }
	}

	if(i==1) {
	  std::ostringstream sout;
	  sout << n;
	  r->addString(sout.str());
	} else {
	  r->setConfiguration(m);
	}
	
	if(_printEnable) {
	  std::unordered_map<std::string,uint32_t> m2;
	  r->configuration(m2);
	
	  std::cout << "Record configuration unpacked" << std::endl;
	  for(auto i(m2.begin());i!=m2.end();i++) {
	    std::cout << " " << i->first << " = " << i->second << std::endl;
	  }
	}

      
	/*
	  r->addData32(_serenityEncoder.uhalRead("ctrl"));
	  r->addData32(_serenityEncoder.uhalRead("calpulse_ctrl"));
	  r->addData32(_serenityEncoder.uhalRead("ctrl_2"));
	  r->addData32(_serenityEncoder.uhalRead("ctrl_3"));
	*/
      
	if(_printEnable) r->print();
      
	ptrFifoShm2->writeIncrement();
      }
      ///////////////
      /*
	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::ConfiguredB);
	r->setType(RecordConfigured::BE);
	r->setLocation(0xbe00);
	r->print();
      
	for(unsigned i(0);i<_uhalString.size() && i<10;i++) {
	r->addString(_uhalString[i]);
	}
	r->print();
	ptrFifoShm2->writeIncrement();
      
	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::ConfiguredB);
	r->setType(RecordConfigured::BE);
	r->setLocation(0xbe01);
	r->print();


	UhalInstruction xi;

	//uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
	//uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);


	for(unsigned i(0);i<_uhalString.size();i++) {
	xi.setAddress(i);
	#ifdef ProcessorHardware
	#ifdef JUNK
	const uhal::Node& lNode = lHW.getNode("payload."+_uhalString[i]);
	uhal::ValWord<uint32_t> lReg = lNode.read();
	lHW.dispatch();
	xi.setValue(lReg.value());
	#endif
	#else
	xi.setValue(0x1000*i);
	#endif
	xi.print();
	r->addData64(xi.data());
	}
    
	if(_printEnable) r->print();
	ptrFifoShm2->writeIncrement();
      */
          
      ///////////////
	
      /* 
	 char c='1';
	 for(uint8_t i(0);i<3;i++) {
	 RecordConfiguredB h;
	 h.setHeader(_cfgSeqCounter++);
	 //h.setState(FsmState::ConfiguredB);
	 //h.setPayloadLength(1);
	 h.setSuperRunNumber(_superRunNumber);
	 h.setConfigurationCounter(_cfgSeqCounter++);
	
	 std::string s("HGROC_");
	 s+=(c+i);
	 h.setConfigurationPacketHeader(s);
	 h.setConfigurationPacketValue(0xbeefbeefcafecafe);
	
	 h.print();
	
	 assert(ptrFifoShm2->write(h.totalLength(),(uint64_t*)(&h)));
	 }
      */      
      RecordContinuing *rc((RecordContinuing*)(ptrFifoShm2->getWriteRecord()));
      rc->setHeader();
      if(_printEnable) rc->print();
      ptrFifoShm2->writeIncrement();
    }

    virtual void running() {
      _serenityEncoder.uhalWrite("ctrl.tts",1);

      _ptrFsmInterface->setProcessState(FsmState::Running);

      while(_ptrFsmInterface->systemState()==FsmState::Running) usleep(1000);
      
      _serenityEncoder.uhalWrite("ctrl.tts",0);
      
      RecordContinuing *rc((RecordContinuing*)(ptrFifoShm2->getWriteRecord()));
      rc->setHeader();
      if(_printEnable) rc->print();
      ptrFifoShm2->writeIncrement();
    }
    
    void paused() {
      _pauseCounter++;

      RecordContinuing *rc((RecordContinuing*)(ptrFifoShm2->getWriteRecord()));
      rc->setHeader();
      if(_printEnable) rc->print();
      ptrFifoShm2->writeIncrement();
    }

    
  protected:
    RelayWriterDataFifo *ptrFifoShm2;

    uint32_t _cfgSeqCounter;
    uint32_t _evtSeqCounter;
    uint32_t _pauseCounter;

    uint32_t _relayNumber;
    uint32_t _runNumber;

    uint32_t _runNumberInSuperRun;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;

    SerenityEncoder _serenityEncoder;
    SerenityLpgbt _serenityLpgbt;
    SerenityMiniDaq _serenityMiniDaq;  
  };

}

#endif
