#ifndef Hgcal10gLinkReceiver_ProcessorRelay_h
#define Hgcal10gLinkReceiver_ProcessorRelay_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include <yaml-cpp/yaml.h>

#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"


#include "I2cInstruction.h"
#include "UhalInstruction.h"
#include "RecordConfigured.h"
#include "RecordHalted.h"



#include "FileWriter.h"


namespace Hgcal10gLinkReceiver {

  class ProcessorRelay : public ProcessorBase {
    
  public:

    ProcessorRelay() : _ignoreInputs(false) {
    }

    virtual ~ProcessorRelay() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey0, uint32_t fifoKey1, uint32_t fifoKey2) {
      ShmSingleton<RelayWriterDataFifo> shm0;
      _ptrFifoShm.push_back(shm0.setup(fifoKey0));

      ShmSingleton<RelayWriterDataFifo> shm1;
      _ptrFifoShm.push_back(shm1.setup(fifoKey1));

      ShmSingleton<RelayWriterDataFifo> shm2;
      _ptrFifoShm.push_back(shm2.setup(fifoKey2));
      
      _fifoShmAlive.resize(_ptrFifoShm.size());
      for(unsigned i(0);i<_ptrFifoShm.size();i++) {
	_fifoShmAlive[i]=true;
	_ptrFifoShm[i]->coldStart();
      }
      
      startFsm(rcKey);
    }
  
    virtual bool initializing() {
      const Record *r((Record*)&(_ptrFsmInterface->record()));
      _haltedUtc=r->utc();
      return true;
    }

    bool configuring() {
      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));

      _relayNumber=r.relayNumber();

      if(r.relayNumber()<0xffffffff) {
	std::ostringstream sout;
	sout << "dat/Relay" << std::setfill('0')
	     << std::setw(10) << r.relayNumber();

	_relayDirectory=sout.str();
	system((std::string("mkdir ")+_relayDirectory).c_str());	
	_fileWriter.setDirectory(sout.str());
      }
      
      _fileWriter.openRelay(r.relayNumber());
      _fileWriter.write(&(_ptrFsmInterface->record()));
      
      _cfgSeqCounter=1;
      _evtSeqCounter=1;
      
      return true;
    }
    
    bool reconfiguring() {
	_fileWriter.write(&(_ptrFsmInterface->record()));

	_eventNumberInConfiguration=0;

      return true;
    }

    bool starting() {
      _fileWriter.write(&(_ptrFsmInterface->record()));
      _runNumber=((const RecordStarting*)&(_ptrFsmInterface->record()))->runNumber();
      
      const Record *r;

      for(unsigned i(2);i<_ptrFifoShm.size() && !_ignoreInputs;i++) {
	bool done(false);
	while(!done) {
	  std::cout << "i = " << i << std::endl;

	  while((r=_ptrFifoShm[i]->readRecord())!=nullptr) {

	    std::string s;
	    if(r->state()==FsmState::Configured) {
	      const RecordYaml *rc((const RecordYaml*)r);
	      if(_printEnable) rc->print();
	      s=rc->string();
	    }
	    /*
	    if(r->state()==FsmState::Halted) {
	      const RecordYaml *rh((const RecordYaml*)r);
	      if(_printEnable) rh->print();
	      s=rh->string();
	    }
	    */
	    if(s!="") {
	      if(_relayNumber<0xffffffff) {
		YAML::Node n(YAML::Load(s));

		std::ostringstream sout;
		sout << std::setfill('0')
		     << "Run" << _runNumber << "_";

		std::cout << "HERE0" << std::endl;
		if(n["Source"].as<std::string>()=="LpGBT" )
		  sout << "Lpgbt"  << std::setw(3) << n["Lpgbt" ].as<unsigned>();
		std::cout << "HERE1" << std::endl;
		if(n["Source"].as<std::string>()=="ECOND" )
		  sout << "EconD"  << std::setw(3) << n["EconD" ].as<int>();
		std::cout << "HERE2" << std::endl;
		if(n["Source"].as<std::string>()=="ECONT" )
		  sout << "EconT"  << std::setw(3) << n["EconT" ].as<int>();
		std::cout << "HERE3" << std::endl;
		if(n["Source"].as<std::string>()=="HGCROC")
		  sout << "Hgcroc" << std::setw(3) << 3*n["EconD" ].as<int>()+n["Hgcroc"].as<int>();
		std::cout << "HERE4" << std::endl;

		sout << ".yaml";

		std::ofstream fout((_relayDirectory+"/"+sout.str()).c_str());
		fout << n << std::endl;
		fout.close();
	      }
	    }

	    if(r->state()!=FsmState::Continuing) {
	      _fileWriter.write(r);
	    } else {
	      done=true;
	    }
	    _ptrFifoShm[i]->readIncrement();
	  }
	}
      }

	_pauseCounter=0;
	_eventNumberInRun=0;


      return true;
    }

    bool pausing() {
	_fileWriter.write(&(_ptrFsmInterface->record()));

      return true;
    }
    
    bool resuming() {
	_fileWriter.write(&(_ptrFsmInterface->record()));
      return true;
    }
    
    bool stopping() {
	RecordStopping r;
	r.deepCopy((_ptrFsmInterface->record()));

	_eventNumberInConfiguration+=_eventNumberInRun;

	r.setNumberOfEvents(_eventNumberInRun);
	r.setNumberOfPauses(_pauseCounter);
	if(_printEnable) r.print();

	_fileWriter.write(&r);

      return true;
    }
    
    bool halting() {
      const Record *r((Record*)&(_ptrFsmInterface->record()));
      _haltedUtc=r->utc();
	
      _eventNumberInRelay+=_eventNumberInConfiguration;
      _fileWriter.write(&(_ptrFsmInterface->record()));
      _fileWriter.close();
      return true;
    }
    
    bool resetting() {
      _fileWriter.close();
      return true;
    }

    bool ending() {
      _fileWriter.close();
      return true;
    }

    //////////////////////////////////////////////
    
    virtual void halted() {
      _ptrFsmInterface->setProcessState(FsmState::Halted);
      while(_ptrFsmInterface->systemState()==FsmState::Halted) usleep(1000);

      std::cout << "*** HALTED ***" << std::endl;

      const RecordYaml *r;
 
      for(unsigned i(0);i<_ptrFifoShm.size() && !_ignoreInputs;i++) {
	//bool done(false);
	//while(!done) {
	while((r=(RecordYaml*)_ptrFifoShm[i]->readRecord())!=nullptr) {
	    if(_printEnable) r->print();

	    if(r->state()==FsmState::Halted) {
	      std::ostringstream sout;
	      sout << "dat/Serenity0Constants" << std::setfill('0')
		   << std::setw(9) << _haltedUtc << ".yaml";
	      
	      YAML::Node n(YAML::Load(std::string(r->string())));
	      std::ofstream fout(sout.str().c_str());
	      fout << n << std::endl;
	      fout.close();
	    }

	    if(r->state()!=FsmState::Continuing) {
	      _fileWriter.write(r);
	    } else {
	      //done=true;
	    }
	    _ptrFifoShm[i]->readIncrement();
	  }
	  //}
      }

    }
    
    virtual void configured() {
      if(_printEnable) std::cout << "ProcessRelay::configured()" << std::endl;

      _ptrFsmInterface->setProcessState(FsmState::Configured);

      while(_ptrFsmInterface->systemState()==FsmState::Configured) usleep(1000);


#ifdef NOT_YET


      for(unsigned i(0);i<_ptrFifoShm.size() && !_ignoreInputs;i++) {

	std::cout << "configured() relay = " << _relayNumber << std::endl;

	RecordConfigured *r;
	for(unsigned i(1);i<=3;i++) {

	  while((r=(RecordConfigured*)_ptrFifoShm[i]->getWriteRecord())==nullptr) usleep(10);
	  r->setHeader(_cfgSeqCounter++);
	  r->setState(FsmState::Configured);
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
	  _ptrFifoShm[i]->writeIncrement();
	}

      
	///////////////
	/*
	  while((r=(RecordConfigured*)_ptrFifoShm[i]->getWriteRecord())==nullptr) usleep(10);
	  r->setHeader(_cfgSeqCounter++);
	  r->setState(FsmState::ConfiguredB);
	  r->setType(RecordConfigured::BE);
	  r->setLocation(0xbe00);
	  r->print();
      
	  for(unsigned i(0);i<_uhalString.size() && i<10;i++) {
	  r->addString(_uhalString[i]);
	  }
	  r->print();
	  _ptrFifoShm[i]->writeIncrement();
      
	  while((r=(RecordConfigured*)_ptrFifoShm[]->getWriteRecord())==nullptr) usleep(10);
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
	  _ptrFifoShm[i]->writeIncrement();
	*/
          
	///////////////
	
	/* 
	   char c='1';
	   for(uint8_t i(0);i<3;i++) {
	   RecordConfiguredB h;
	   h.setHeader(_cfgSeqCounter++);
	   //h.setState(FsmState::ConfiguredB);
	   //h.setPayloadLength(1);
	   h.setRelayNumber(_RelayNumber);
	   h.setConfigurationCounter(_cfgSeqCounter++);
	
	   std::string s("HGROC_");
	   s+=(c+i);
	   h.setConfigurationPacketHeader(s);
	   h.setConfigurationPacketValue(0xbeefbeefcafecafe);
	
	   h.print();
	
	   assert(_ptrFifoShm[i]->write(h.totalLength(),(uint64_t*)(&h)));
	   }
	*/      
      }
#endif
    }

    virtual void running() {
      if(_printEnable) std::cout << "ProcessRelay::running()" << std::endl;

      _ptrFsmInterface->setProcessState(FsmState::Running);

      while(_ptrFsmInterface->systemState()==FsmState::Running) usleep(1000);

      const Record *r;

      for(unsigned i(0);i<_ptrFifoShm.size() && !_ignoreInputs;i++) {
	//bool done(false);
	//while(!done) {
	while((r=_ptrFifoShm[i]->readRecord())!=nullptr) {
	  if(_printEnable) r->print();
	  
	  if(r->state()!=FsmState::Continuing) {
	    _fileWriter.write(r);
	  } else {
	    //done=true;
	  }
	  _ptrFifoShm[i]->readIncrement();
	}
	//}
      }
    }

    void paused() {
      if(_printEnable) std::cout << "ProcessRelay::paused()" << std::endl;

      _ptrFsmInterface->setProcessState(FsmState::Paused);

      while(_ptrFsmInterface->systemState()==FsmState::Paused) usleep(1000);

      const Record *r;

      for(unsigned i(0);i<_ptrFifoShm.size() && !_ignoreInputs;i++) {
	//bool done(false);
	//while(!done) {
	while((r=_ptrFifoShm[i]->readRecord())==nullptr) {
	  if(_printEnable) r->print();
	  
	  if(r->state()!=FsmState::Continuing) {
	    _fileWriter.write(r);
	  } else {
	    //done=true;
	  }
	  _ptrFifoShm[i]->readIncrement();
	}
	//}
      }
      _pauseCounter++;
    }
        
  protected:
    std::vector<RelayWriterDataFifo*> _ptrFifoShm;
    std::vector<bool> _fifoShmAlive;

    FileWriter _fileWriter;

    bool _ignoreInputs;
    
    uint32_t _cfgSeqCounter;
    uint32_t _evtSeqCounter;
    uint32_t _pauseCounter;

    uint32_t _haltedUtc;
    
    uint32_t _relayNumber;
    uint32_t _runNumber;
    std::string _relayDirectory;

    uint32_t _runNumberInRelay;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInRelay;
  };

}

#endif
