#ifndef Hgcal10gLinkReceiver_ProcessorFrontEnd_h
#define Hgcal10gLinkReceiver_ProcessorFrontEnd_h

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
#include "RecordPrinter.h"
#include "RecordYaml.h"
#include "DataFifo.h"

namespace Hgcal10gLinkReceiver {

  class ProcessorFrontEnd : public ProcessorBase {
    
  public:
    ProcessorFrontEnd() {
      _fifoCounter=0;
      _sequenceCount=0;
      _cfgForRunStart=false;
    }

    virtual ~ProcessorFrontEnd() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      ShmSingleton<RelayWriterDataFifo> shm2;
      ptrFifoShm2=shm2.setup(fifoKey);
      //ptrFifoShm2=nullptr;
      startFsm(rcKey);
    }

    virtual bool initializing() {
      std::string c("cd /home/cmx/rshukla/hgc-engine-tools;python3 roc_control_PD.py --source file --filename initLD-hgcrocv3.csv --dir write --roc s2");
      if(_printEnable) std::cout << "System call = " << c << std::endl;
      system(c.c_str());

      

      return true;
    }

    bool configuring() {
      _cfgForRunStart=true;
      
      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      _keyCfgA=r.processorKey(RunControlFrontEndFsmShmKey);
	
      if(_keyCfgA<=40) {
	uint16_t add;
	uint8_t val,msk;

	// Loop over halves
	for(unsigned i(0);i<2;i++) {

	  // Digitial half: Set characterisation mode
	  add=0x564+i*0x5c0;
	  val=0x40;
	  msk=0x40;
	  i2cWriteAndRead(add,val,msk);

	  // Global analogue: unset fixed ADC values
	  add=0x52a+i*0x5c0;
	  val=0x00;
	  msk=0x08;
	  i2cWriteAndRead(add,val,msk);

	  // Halfwise: disconnect all injection capacitors
	  add=0x584+i*0x5c0;
	  val=0x00;
	  msk=0x06;
	  i2cWrite(add,val,msk);
	}
      }
	
      _configuringBCounter=0;

      return true;
    }
    
    bool reconfiguring() {
      _cfgForRunStart=true;
      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      _keyCfgB=r.processorKey(RunControlFrontEndFsmShmKey);

      if(_keyCfgA>0 && _keyCfgA<=40) {
	uint16_t dac(0),chn(0);
	unsigned dacSteps(8);
	  
	dac=(4096/dacSteps)*(_configuringBCounter%dacSteps);

	if(_keyCfgA<=39) chn=_keyCfgA-1;
	else chn=(_configuringBCounter/dacSteps);

	if(_printEnable) {
	  std::cout << "configuringB() Key A = " << _keyCfgA
		    << ", DAC = " << dac << ", Channel = "
		    << chn << std::endl;
	}
	  
	uint16_t add;
	uint8_t val,msk;

	// Loop over halves
	for(unsigned i(0);i<2;i++) {
	    
	  // Halfwise: disconnect all injection capacitors
	  add=0x584+i*0x5c0;
	  val=0x00;
	  msk=0x06;
	  i2cWrite(add,val,msk);

	  // Set individual channel low range injection capacitor
	  add=0x0004+chn*0x020+i*0x05c0;
	  val=0x02;
	  msk=0x06;
	  i2cWriteAndRead(add,val,msk);
	    
	  // Reference voltage: set DAC value and IntCtest
	  add=0x0506+i*0x05c0;
	  val=dac&0xff;
	  msk=0xff;
	  i2cWriteAndRead(add,val,msk);

	  add=0x0507+i*0x05c0;
	  val=(dac>>8|0x40);
	  msk=0x4f;
	  i2cWriteAndRead(add,val,msk);
	}	  
      }

      _configuringBCounter++;

      return true;
    }

    bool starting() {
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
      return true;
    }
    
    bool halting() {
      std::string c("cd /home/cmx/rshukla/hgc-engine-tools;python3 roc_control_PD.py --source file --filename initLD-hgcrocv3.csv --dir write --roc s2");
      if(_printEnable) std::cout << "System call = " << c << std::endl;
      system(c.c_str());
	
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

    virtual void halted() {
     if(_printEnable) {
	std::cout << "ProcessorFrontEnd::halted()" << std::endl;
      }
     
      RecordYaml *r;

      while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      r->setHeader(_sequenceCount++);
      r->setState(FsmState::Halted);

      YAML::Node n;
      n["Source"]="FE";
      n["ElectronicsId"]=123456789;

      std::ostringstream sout0;
      sout0 << n;
      r->setString(sout0.str());
      
      if(_printEnable) r->print();
      ptrFifoShm2->writeIncrement();
      
      writeContinuing();
    }
    
    virtual void configured() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEnd::configured()" << std::endl;
      }

      //if(_cfgForRunStart) {
      
      unsigned daqBoard(18);
      unsigned slink(9);
      unsigned lpgbtPair(3);
      
      RecordYaml *r;

      while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      r->setHeader(_sequenceCount++);

      YAML::Node n;
      n["Source"]="LpGBT";
      n["DaqBoard"]=daqBoard;
      n["Slink"]=slink;
      n["LpgbtPair"]=lpgbtPair;
      n["Lpgbt"]=0;
      n["ElectronicsId"]=daqBoard<<22|slink<<18|lpgbtPair<<14|0<<10|0x3ff;
	
       std::ostringstream soutl;
       soutl << "cfg/LpGBT" << 0 << ".yaml";
       n["Configuration"]=YAML::LoadFile(soutl.str());

       std::ostringstream sout0;
       sout0 << n;
       r->setString(sout0.str());
	
       if(_printEnable) r->print();
       ptrFifoShm2->writeIncrement();

	
       for(unsigned e(0);e<2;e++) {
       	while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(100);
       	r->setHeader(_sequenceCount++);

       	YAML::Node n0;
       	n0["Source"]="ECOND";
	n0["DaqBoard"]=daqBoard;
	n0["Slink"]=slink;
	n0["LpgbtPair"]=lpgbtPair;
       	n0["EconD"]=e;
	n0["ElectronicsId"]=daqBoard<<22|slink<<18|lpgbtPair<<14|e<<10|0x3ff;
	
       	std::ostringstream soute0;
       	soute0 << "cfg/ECOND" << e << ".yaml";
       	n0["Configuration"]=YAML::LoadFile(soute0.str());

       	std::ostringstream sout0;
       	sout0 << n0;
       	r->setString(sout0.str());
	
       	if(_printEnable) r->print();
       	ptrFifoShm2->writeIncrement();
	
       	while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(100);
       	r->setHeader(_sequenceCount++);

       	YAML::Node n1;
       	n1["Source"]="ECONT";
	n1["DaqBoard"]=daqBoard;
	n1["Slink"]=slink;
	n1["LpgbtPair"]=lpgbtPair;
       	n1["EconT"]=e;
	n1["ElectronicsId"]=daqBoard<<22|slink<<18|lpgbtPair<<14|e<<10|0x3ff;
	
       	std::ostringstream soute1;
       	soute1 << "cfg/ECONT" << e << ".yaml";
       	n1["Configuration"]=YAML::LoadFile(soute1.str());

       	std::ostringstream sout1;
       	sout1 << n1;
	r->setString(sout1.str());
	
	if(_printEnable) r->print();
	ptrFifoShm2->writeIncrement();
	
	for(unsigned h(0);h<3;h++) {
	  while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(100);
	  r->setHeader(_sequenceCount++);

	  YAML::Node n;
	  n["Source"]="HGCROC";
	  n["DaqBoard"]=daqBoard;
	  n["Slink"]=slink;
	  n["LpgbtPair"]=lpgbtPair;
	  n["EconD"]=e;
	  n["Hgcroc"]=h;
	  n["ElectronicsId"]=daqBoard<<22|slink<<18|lpgbtPair<<14|e<<10|h<<6|0x3f;
	  
	  std::ostringstream south;
	  south << "cfg/HGCROC" << 3*e+h << ".yaml";
	  n["Configuration"]=YAML::LoadFile(south.str());

	  std::ostringstream sout;
	  sout << n;
	  r->setString(sout.str());
	  
	  if(_printEnable) r->print();
	  ptrFifoShm2->writeIncrement();
	}
      }
       //}
      /*
	RecordConfigured *r;
	while((r=(RecordConfigured*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(10);

	r->setHeader(++_fifoCounter);
	r->setState(FsmState::ConfiguredB);
      
	std::vector<uint32_t> v;
	_serenityTcds2.configuration(v);
	for(unsigned i(0);i<v.size();i++) r->addData32(v[i]);
	if(_printEnable) r->print();

	ptrFifoShm2->writeIncrement();
      */

      writeContinuing();
    }

    void running() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEnd::running()" << std::endl;
      }
      
      writeContinuing();
    }

    void paused() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEnd::paused()" << std::endl;
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
    
    void i2cRead(uint16_t add, uint8_t msk) {
      std::ostringstream sout;
      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control_PD.py"
	   << " --roc s2 --reg 0x"
	   << std::hex << std::setfill('0') << std::setw(4) << add
	   << " --mask 0x" << std::setw(2) << uint16_t(msk);

      if(_printEnable) std::cout << "Read system call = "
				 << sout.str() << std::endl;
      system(sout.str().c_str());
    }
    
    void i2cWrite(uint16_t add, uint8_t val, uint8_t msk) {
      std::ostringstream sout;
      sout << "cd /home/cmx/rshukla/hgc-engine-tools; python3 roc_control_PD.py"
	   << " --roc s2 --reg 0x"
	   << std::hex << std::setfill('0') << std::setw(4) << add
	   << " --mask 0x" << std::setw(2) << uint16_t(msk)
	   << "  --dir write --value " << uint16_t(val);

      if(_printEnable) std::cout << "Write system call = "
				 << sout.str() << std::endl;
      system(sout.str().c_str());
    }
    
    void i2cWriteAndRead(uint16_t add, uint8_t val, uint8_t msk) {
      i2cWrite(add,val,msk);
      i2cRead(add,msk);
    }
        
  protected:
    RelayWriterDataFifo *ptrFifoShm2;

    bool _cfgForRunStart;

    uint32_t _fifoCounter;
    uint32_t _sequenceCount;
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
  };

}

#endif
