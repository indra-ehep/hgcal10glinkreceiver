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
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"



#include "I2cInstruction.h"
#include "UhalInstruction.h"
#include "RecordConfigured.h"



#include "RecordPrinter.h"

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
      _serenityUhal.makeTable();
      _serenityLpgbt.makeTable();
      

      ShmSingleton<RelayWriterDataFifo> shm2;
      ptrFifoShm2=shm2.setup(fifoKey);
      //ptrFifoShm2=shm2.payload();
      startFsm(rcKey);
    }

    virtual bool initializing() {
      std::cout << "HEREI !!!" << std::endl;

      _serenityUhal.setDefaults();
      _serenityUhal.print();
      _serenityLpgbt.setDefaults();
      _serenityLpgbt.print();

      std::cout << "HEREI2 !!!" << std::endl;
#ifdef JUNK
#ifdef ProcessorHardware
      //system("/home/cmx/pdauncey/source setFC.sh");

      std::vector<std::string> lRegisterName;
      std::vector<std::string> lRegisterNameW;
  
      lRegisterName.push_back("tcds2_emu.ctrl_stat.ctrl.seq_length");

      //uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
      //uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);
  
  
      const uhal::Node& lNode = lHW.getNode("payload.fc_ctrl." + lRegisterName[0]);
      std::cout << "Reading from register '" << lRegisterName[0] << "' ..." << std::endl;
      uhal::ValWord<uint32_t> lReg = lNode.read();
      lHW.dispatch();
      std::cout << "... success!" << std::endl << "Value = 0x" << std::hex << lReg.value() << std::endl;

      std::vector<std::string> ids;//, ids_filtered;
      std::vector<uint32_t> nds;//, ids_filtered;
      //ids = lHW.getNode("payload.fc_ctrl.tcds2_emu").getNodes();
      ids = lHW.getNode("payload.fc_ctrl").getNodes();
  
      std::cout << "payload.fc_ctrl.tcds2_emu.getNodes(): ";
      /*
	std::copy(ids.begin(),
	ids.end(),
	std::ostream_iterator<std::string>(std::cout,", "));
      */
      for(unsigned i(0);i<ids.size();i++) {
	std::cout << ids[i] << std::endl;
      }
      std::cout << std::endl << std::endl;




      for(unsigned i(0);i<ids.size();i++) {
  
	// PART 3: Reading from the register
	const uhal::Node& lNode = lHW.getNode("payload.fc_ctrl." + ids[i]);

	std::cout << "Reading from register '" << ids[i] << "' ..." << std::endl;

	uhal::ValWord<uint32_t> lReg = lNode.read();
	// dispatch method sends read request to hardware, and waits for result to return
	// N.B. Before dispatch, lReg.valid() == false, and lReg.value() will throw
	lHW.dispatch();

	std::cout << "... success!" << std::endl << "Value = 0x" << std::hex << lReg.value() << std::endl;
	nds.push_back(lReg.value());
      }

      std::vector<std::string> temp;
      temp=lHW.getNode("payload").getNodes();

      if(_printEnable) {
	for(unsigned i(0);i<temp.size();i++) {
	  std::cout << "ALL string " << temp[i] << std::endl;
	}
	std::cout << std::endl;
      }

      _uhalString.resize(0);
      for(unsigned i(0);i<temp.size();i++) {
	if(temp[i].substr(0,8)=="fc_ctrl.") {
	  std::cout << "Substr = " << temp[i].substr(8,17) << std::endl;
	  if(temp[i].substr(8,17)!="tcds2_emu") {
	    std::cout << "UHAL string " << std::setw(3) << " = " 
		      << temp[i] << std::endl;
	    _uhalString.push_back(temp[i]);
	  }
	}
      }
	
      if(_printEnable) {
	for(unsigned i(0);i<_uhalString.size();i++) {
	  std::cout << "UHAL string " << std::setw(3) << " = " 
		    << _uhalString[i] << std::endl;

	  const uhal::Node& lNode = lHW.getNode("payload."+_uhalString[i]);
	  uhal::ValWord<uint32_t> lReg = lNode.read();
	  lHW.dispatch();
	    
	  std::cout << "UHAL string " << std::setw(3) << " = "
		    << _uhalString[i] << ", initial value = " 
		    << lReg.value() << std::endl;
	}
	std::cout << std::endl;
      } 

      // Now set the initialization values
      /*

	python3 hgcal_fc.py -c test_stand/connections.xml -d x0 encoder configure -tts on = fc_ctrl.fpga_fc.ctrl.tts
	python3 hgcal_fc.py -c test_stand/connections.xml -d x0 encoder configure --set_prel1a_offset 3 = fc_ctrl.fpga_fc.ctrl.prel1a_offset
	fc_ctrl.fpga_fc.ctrl.user_prel1a_off_en

	python3 hgcal_fc.py -c test_stand/connections.xml -d x0 tcds2_emu configure --seq_disable

	fc_ctrl.fc_lpgbt_pair.ctrl.issue_linkrst
	fc_ctrl.fc_lpgbt_pair.fc_cmd.linkrst

      */


      std::cout << "SETTING SOME DEFAULTS" << std::endl;
      assert(uhalWrite("fc_ctrl.fpga_fc.ctrl.tts",0));
      uhalWrite("fc_ctrl.fpga_fc.ctrl.prel1a_offset",3);
      uhalWrite("fc_ctrl.fpga_fc.ctrl.user_prel1a_off_en",1);

      uint32_t hgcrocLatencyBufferDepth(10); // ????
      uint32_t cpid(8),cped(10); // ???
      uhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_int_del",cpid);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",12+cpid+hgcrocLatencyBufferDepth);

      uhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_ext_del",cped);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",12+cped+hgcrocLatencyBufferDepth);
	
	
      /* ????
	 fc_ctrl.fc_lpgbt_pair.ctrl.issue_linkrst
	 fc_ctrl.fc_lpgbt_pair.fc_cmd.linkrst
      */
	
#else
      _uhalString.resize(0);
      _uhalString.push_back("tcds2_emu.ctrl_stat.ctrl.seq_length"); 
#endif
#endif

	
      return true;
    }

    bool configuringA() {
      if(_printEnable) {
	std::cout << "ConfiguringA" << std::endl;
	RecordPrinter(_ptrFsmInterface->record());
	std::cout << std::endl;
      }

      if(_checkEnable) {
	if(_ptrFsmInterface->record().state()!=FsmState::ConfiguringA) {
	  std::cerr << "State does not match" << std::endl;
	  std::cout << "State does not match" << std::endl;
	  if(_assertEnable) assert(false);
	}
      }
      
      //assert(_ptrFsmInterface->commandPacket().record().state()==FsmState::ConfiguringA);

      _cfgSeqCounter=1;
      _evtSeqCounter=1;


      // Do configuration; ones which could have been changed
      _serenityUhal.uhalWrite("payload.fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_int_del",8);
	
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
    
    bool configuringB() {
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
    
    bool haltingB() {
      /*
	_eventNumberInSuperRun+=_eventNumberInConfiguration;

	RecordHaltingB rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.setNumberOfEvents(_eventNumberInConfiguration);
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      */
      return true;
    }
    
    bool haltingA() {
      /*
	RecordHaltingA rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	//rr.setNumberOfRuns(_runNumberInSuperRun);
	rr.setNumberOfEvents(_eventNumberInSuperRun);
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      */

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


    virtual void configuredA() {
      RecordContinuing rc;
      rc.setHeader();
      rc.print();
      assert(ptrFifoShm2->write(rc.totalLength(),(uint64_t*)(&rc)));   
    }
    
    virtual void configuredB() {

      std::cout << "configuredB() super run = " << _superRunNumber << std::endl;

      //RecordConfiguredB *r;
      RecordConfigured *r;
      for(unsigned i(1);i<=3;i++) {

	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::ConfiguredB);
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
	    i2c.print();
	  
	    std::cout << "HGCROC cfg address 0x" << std::setw(4) << add
		      << ", mask 0x" << std::setw(2) << unsigned(mask)
		      << ", value 0x" << std::setw(2) << unsigned(val)
		      << std::endl;

	    r->addData32(i2c.data());
	    if(add<16) r->print();
	  
	    fin.getline(buffer,16);
	  }
	
	  std::cout << std::dec << std::setfill(' ');
	}
	fin.close();
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
      RecordContinuing rc;
      rc.setHeader();
      if(_printEnable) rc.print();
      assert(ptrFifoShm2->write(rc.totalLength(),(uint64_t*)(&rc)));   

    }

    virtual void running() {
      _serenityUhal.uhalWrite("payload.fc_ctrl.fpga_fc.ctrl.tts",1);

      _ptrFsmInterface->setProcessState(FsmState::Running);

      while(_ptrFsmInterface->systemState()==FsmState::Running) usleep(1000);
      
      _serenityUhal.uhalWrite("payload.fc_ctrl.fpga_fc.ctrl.tts",0);
      
      RecordContinuing rc;
      rc.setHeader();
      rc.print();
      assert(ptrFifoShm2->write(rc.totalLength(),(uint64_t*)(&rc)));   
    }
    
    void paused() {
      _pauseCounter++;

      RecordContinuing rc;
      rc.setHeader();
      rc.print();
      assert(ptrFifoShm2->write(rc.totalLength(),(uint64_t*)(&rc)));   
    }

    
  protected:
    RelayWriterDataFifo *ptrFifoShm2;

    uint32_t _cfgSeqCounter;
    uint32_t _evtSeqCounter;
    uint32_t _pauseCounter;

    uint32_t _superRunNumber;
    uint32_t _runNumber;

    uint32_t _runNumberInSuperRun;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;

    SerenityEncoder _serenityUhal;
    SerenityLpgbt _serenityLpgbt;
  
  };

}

#endif
