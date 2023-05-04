#ifndef ProcessorFastControl_h
#define ProcessorFastControl_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

//#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"



#include "I2cInstruction.h"
#include "UhalInstruction.h"
#include "RecordConfigured.h"



#include "RecordPrinter.h"
#include "ShmKeys.h"
#include "SlinkBoe.h"
#include "SlinkEoe.h"

//#undef ProcessorFastControlHardware
#define ProcessorFastControlHardware

#ifdef ProcessorFastControlHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class ProcessorFastControl : public ProcessorBase {
    
  public:
  ProcessorFastControl() : lConnectionFilePath("etc/connections.xml"),
      lDeviceId("x0"),
      lConnectionMgr("file://" + lConnectionFilePath),
      lHW(lConnectionMgr.getDevice(lDeviceId)) {
      
	uhal::setLogLevelTo(uhal::Error());  
	//lHW = lConnectionMgr.getDevice(lDeviceId);
    }

    virtual ~ProcessorFastControl() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      ShmSingleton< DataFifoT<6,1024> > shm2;
      ptrFifoShm2=shm2.setup(fifoKey);
      //ptrFifoShm2=shm2.payload();
      startFsm(rcKey);
    }

    virtual bool initializing(FsmInterface::HandshakeState s) {
      std::cout << "HEREI !!!" << std::endl;
      if(s==FsmInterface::Change) {

      std::cout << "HEREI2 !!!" << std::endl;
#ifdef ProcessorFastControlHardware
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

	_xhalString.resize(0);
	for(unsigned i(0);i<temp.size();i++) {
	  if(temp[i].substr(0,8)=="fc_ctrl.") {
	    std::cout << "Substr = " << temp[i].substr(8,17) << std::endl;
	    if(temp[i].substr(8,17)!="tcds2_emu") {
	    std::cout << "XHAL string " << std::setw(3) << " = " 
		      << temp[i] << std::endl;
	      _xhalString.push_back(temp[i]);
	    }
	  }
	}
	
	if(_printEnable) {
	  for(unsigned i(0);i<_xhalString.size();i++) {
	    std::cout << "XHAL string " << std::setw(3) << " = " 
		      << _xhalString[i] << std::endl;

	    const uhal::Node& lNode = lHW.getNode("payload."+_xhalString[i]);
	    uhal::ValWord<uint32_t> lReg = lNode.read();
	    lHW.dispatch();
	    
	    std::cout << "XHAL string " << std::setw(3) << " = "
		      << _xhalString[i] << ", initial value = " 
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
	assert(xhalWrite("fc_ctrl.fpga_fc.ctrl.tts",0));
	xhalWrite("fc_ctrl.fpga_fc.ctrl.prel1a_offset",3);
	xhalWrite("fc_ctrl.fpga_fc.ctrl.user_prel1a_off_en",1);

	uint32_t hgcrocLatencyBufferDepth(10); // ????
	uint32_t cpid(8),cped(10); // ???
	xhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_int_del",cpid);
	xhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",12+cpid+hgcrocLatencyBufferDepth);

	xhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_ext_del",cped);
	xhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",12+cped+hgcrocLatencyBufferDepth);
	
	
	/* ????
	fc_ctrl.fc_lpgbt_pair.ctrl.issue_linkrst
	  fc_ctrl.fc_lpgbt_pair.fc_cmd.linkrst
	*/
	
#else
	_xhalString.resize(0);
	_xhalString.push_back("tcds2_emu.ctrl_stat.ctrl.seq_length"); 
#endif
      }
	
      return true;
    }

    bool configuringA(FsmInterface::HandshakeState s) {
      if(_printEnable) {
	std::cout << "ConfiguringA with Handshake = "
		  << FsmInterface::handshakeName(s) << std::endl;
	RecordPrinter(_ptrFsmInterface->commandPacket().record());
	std::cout << std::endl;
      }

      if(_checkEnable) {
	if(_ptrFsmInterface->commandPacket().record().state()!=FsmState::ConfiguringA) {
	  std::cerr << "State does not match" << std::endl;
	  std::cout << "State does not match" << std::endl;
	  if(_assertEnable) assert(false);
	}
      }
      
      if(s==FsmInterface::Change) {
	//assert(_ptrFsmInterface->commandPacket().record().state()==FsmState::ConfiguringA);

	_cfgSeqCounter=1;
	_evtSeqCounter=1;


	// Do configuration; ones which could have been changed
	xhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_int_del",8);


	
	if(_printEnable) {
	  std::cout << "Waiting for space in buffer" << std::endl;
	  ptrFifoShm2->print();
	  std::cout << std::endl;
	}

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
      }

      return true;
    }
    
    bool configuringB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordConfiguringB rcb;
	rcb.deepCopy(_ptrFsmInterface->commandPacket().record());

	_eventNumberInConfiguration=0;


	rcb.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rcb.totalLength(),(uint64_t*)(&rcb)));
      }
      return true;
    }

    bool starting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_pauseCounter=0;
	_eventNumberInRun=0;
      
	RecordStarting rr;
	_runNumber=rr.runNumber();

	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      }

      return true;
    }

    bool pausing(FsmInterface::HandshakeState s) {
      /* NOT FOR Shm2
	 if(s==FsmInterface::Change) {
	 RecordPausing rr;
	 rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	 rr.print();
	 ptrFifoShm2->print();
	 assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
	 }
      */
      return true;
    }
    
    bool resuming(FsmInterface::HandshakeState s) {
      /* NOT FOR Shm2
	 if(s==FsmInterface::Change) {
	 RecordResuming rr;
	 rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	 rr.print();
	 ptrFifoShm2->print();
	 assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
	 }
      */
      return true;
    }
    
    bool stopping(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {

	_eventNumberInConfiguration+=_eventNumberInRun;

	RecordStopping rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.setNumberOfEvents(_eventNumberInRun);
	rr.setNumberOfPauses(_pauseCounter);
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      }
      return true;
    }
    
    bool haltingB(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	_eventNumberInSuperRun+=_eventNumberInConfiguration;

	RecordHaltingB rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.setNumberOfEvents(_eventNumberInConfiguration);
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      }
      return true;
    }
    
    bool haltingA(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordHaltingA rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	//rr.setNumberOfRuns(_runNumberInSuperRun);
	rr.setNumberOfEvents(_eventNumberInSuperRun);
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      }
      return true;
    }
    
    bool resetting(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	RecordResetting rr;
	rr.print();
      }
      return true;
    }

    bool ending(FsmInterface::HandshakeState s) {
      if(s==FsmInterface::Change) {
	std::cout << "Ending" << std::endl;
	ptrFifoShm2->end();
	ptrFifoShm2->print();
      }
      return true;
    }
    /*
//////////////////////////////////////////////
    
void initial() {
//sleep(1);
}
    
void halted() {
//sleep(1);
}
    
void configuredA() {
//sleep(1);
}
    */    

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
	r->print();
      
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
	r->print();
	ptrFifoShm2->writeIncrement();
      }
      
      ///////////////

      while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
      r->setHeader(_cfgSeqCounter++);
      r->setState(FsmState::ConfiguredB);
      r->setType(RecordConfigured::BE);
      r->setLocation(0xbe00);
      r->print();
      
      for(unsigned i(0);i<_xhalString.size() && i<10;i++) {
	r->addString(_xhalString[i]);
      }
      r->print();
      
      while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
      r->setHeader(_cfgSeqCounter++);
      r->setState(FsmState::ConfiguredB);
      r->setType(RecordConfigured::BE);
      r->setLocation(0xbe01);
      r->print();
      
      UhalInstruction xi;

      //uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
      //uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);


      for(unsigned i(0);i<_xhalString.size();i++) {
	xi.setAddress(i);
#ifdef ProcessorFastControlHardware
	const uhal::Node& lNode = lHW.getNode("payload."+_xhalString[i]);
	uhal::ValWord<uint32_t> lReg = lNode.read();
	lHW.dispatch();
	xi.setValue(lReg.value());
#else
	xi.setValue(0x1000*i);
#endif
	xi.print();
	r->addData64(xi.data());
      }
    
      if(_printEnable) r->print();
          
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
	 rc.print();
	 assert(ptrFifoShm2->write(rc.totalLength(),(uint64_t*)(&rc)));   

    }

#ifdef NOW_IN_PLUS_DAQ
    void running() {
      while(_ptrFsmInterface->isIdle()) {
	RecordT<128> buffer;

	RecordRunning &rr((RecordRunning&)buffer);
	rr.setIdentifier(RecordHeader::EventData);
	rr.setState(FsmState::Running);
	rr.setPayloadLength(4);
	rr.setUtc(_evtSeqCounter++);

	SlinkBoe *boe(rr.getSlinkBoe());
	*boe=SlinkBoe();
	boe->setSourceId(RunControlDaqLink0ShmKey);

	SlinkEoe *eoe(rr.getSlinkEoe());
	*eoe=SlinkEoe();

	eoe->setEventLength(2);
	eoe->setCrc(0xdead);
	boe->setL1aSubType(rand()%256);
	boe->setL1aType(rand()%64);
	boe->setEventId(_eventNumberInRun);
	
	eoe->setBxId((rand()%3564)+1);
	eoe->setOrbitId(rand());
	
	if(_evtSeqCounter<10) {
	  rr.print();
	}	

	if(_evtSeqCounter<10) {
	  std::cout << "HERE" << std::endl;
	  rr.print();
	}
	
	if(ptrFifoShm0!=0) {
	  ptrFifoShm0->print();
	  //assert(ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr)));
	  if(!ptrFifoShm0->write(rr.totalLength(),(uint64_t*)(&rr))) {
	    std::cerr << "Failed to write event" << std::endl;
	  }
	}

#ifdef ProcessorFastControlHardware
	std::ifstream fin("/home/cmx/pdauncey/data/rx_summary.txt");
	if(!fin) {
	  std::cout << "Failed to open file" << std::endl;
	  //return false;
	}

	uint64_t singled[128];
	uint64_t doubled[127];

	char buffer[1024];
	fin.getline(buffer,1024);
	//if(sfhDebug) 
	std::cout << buffer << std::endl;
	fin.getline(buffer,1024);
	//if(sfhDebug) 
	std::cout << buffer << std::endl;
      
	std::string str;
	fin >> str;
	///if(sfhDebug)
	std::cout << str;
	fin >> str;
	std::cout << " " << str;
	fin >> str;
	std::cout << " " << str;
	uint64_t n64;
	for(unsigned i(0);i<128 && fin;i++) {
      
	  fin >> str;
	  std::cout << " " << str;
	  fin >> str;
	  std::cout << " " << str << std::endl;

	  fin >> std::hex >> singled[i]
	    std::cout << std::hex << singled[i] << std::endl;
	  fin >> str;
	  std::cout << " " << str;
	}
	std::cout << std::dec << std::endl;

	fin.close();
#endif


	//fin >> n64;
	//std::cout << n64 << std::endl;

	/*
	  unsigned nLinks(0);
	  for(unsigned i(0);i<120 && str!="Frame";i++) {
	  nLinks++;
	
	  if(!append) {
	  v.push_back(SummaryLinkRawData());
	  v.back().link(str);
	  v.back().utc(timestamp);
	  v.back().bx(bxStart);
	  }
	
	  if(i>0 && sfhDebug) std::cout << " " << str;
	  fin >> str;
	  }
      
	  assert(nLinks==v.size());
	  if(sfhDebug) std::cout << std::endl;
	*/



	/*
	  unsigned m;
	  uint64_t d;
	  unsigned f;
      
	  for(unsigned i(0);i<2048 && fin;i++) {
	  assert(str=="Frame");
	
	  fin >> f;
	  assert(f==i);
	
	  if(sfhDebug) {
	  std::cout << "Frame " << std::setw(4) << i
	  << ", command and data = ";
	  }
	
	  for(unsigned j(0);j<v.size();j++) {
	  fin >> m >> std::hex >> d >> std::dec;
	  
	  uint8_t c(0);
	  if( (m/1000    )==1) c+=8;
	  if(((m/100 )%10)==1) c+=4;
	  if(((m/10  )%10)==1) c+=2;
	  if( (m      %10)==1) c+=1;
	  
	  if(sfhDebug) {
	  if(j>0) std::cout << ", ";
	  std::cout << " 0x" << std::hex << std::setfill('0') 
	  << std::setw(2) << unsigned(c)
	  << " 0x" << std::setw(16) << d
	  << std::dec << std::setfill(' ');
	  }
	  
	  v[j].append(c,d);
	  }
	
	  if(sfhDebug) std::cout << std::endl;
	
	  fin >> str;
	  }
      
	  if(!append) {
	  for(unsigned j(0);j<v.size();j++) v[j].setPhase();
	  }
	*/

	boe->setSourceId(RunControlDaqLink1ShmKey);
	  
	if(ptrFifoShm1!=0) {
	  assert(ptrFifoShm1->write(rr.totalLength(),(uint64_t*)(&rr)));
	}
	  
	_eventNumberInRun++;

	usleep(1000);
      }
    }
#endif

    
    void paused() {
      _pauseCounter++;
    }

#ifdef ProcessorFastControlHardware

    uint32_t xhalRead(const std::string &s) {
      const uhal::Node& lNode = lHW.getNode(std::string("payload.")+s);
      uhal::ValWord<uint32_t> lReg = lNode.read();
      lHW.dispatch();
      return lReg.value();
    }
      
    bool xhalWrite(const std::string &s, uint32_t v) {
      std::cout << "xhalWrite: setting " << s << " to  0x"
		<< std::hex << std::setfill('0')
		<< std::setw(8) << v
		<< std::dec << std::setfill(' ')
		<< std::endl;

      const uhal::Node& lNode = lHW.getNode(std::string("payload.")+s);
      lNode.write(v);
      lHW.dispatch();

      return xhalRead(s)==v;
    }

#else

    uint32_t xhalRead(const std::string &s) {
      return 999;
    }

    bool xhalWrite(const std::string &s, uint32_t v) {
      std::cout << "xhalWrite: setting " << s << " to  0x"
		<< std::hex << std::setfill('0')
		<< std::setw(8) << v
		<< std::dec << std::setfill(' ')
		<< std::endl;
      return true;
    }

#endif
    
    void keyConfiguration(uint32_t key) {

      switch(key) {
	
      case 0: {
	break;
      }
	
      case 123: {
	xhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_int_del",
		  5+_runNumberInSuperRun);
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
    //DataFifoT<6,1024> *ptrFifoShm0;
    //DataFifoT<6,1024> *ptrFifoShm1;

    uint32_t _cfgSeqCounter;
    uint32_t _evtSeqCounter;
    uint32_t _pauseCounter;

    uint32_t _superRunNumber;
    uint32_t _runNumber;

    uint32_t _runNumberInSuperRun;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;

    std::vector<std::string> _xhalString;

#ifdef ProcessorFastControlHardware
    const std::string lConnectionFilePath;
    const std::string lDeviceId;
    uhal::ConnectionManager lConnectionMgr;
    uhal::HwInterface lHW;
#endif

  };

}

#endif
