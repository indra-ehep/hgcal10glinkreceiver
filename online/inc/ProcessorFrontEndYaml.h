#ifndef Hgcal10gLinkReceiver_ProcessorFrontEndYaml_h
#define Hgcal10gLinkReceiver_ProcessorFrontEndYaml_h

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

  class ProcessorFrontEndYaml : public ProcessorBase {
    
  public:
    ProcessorFrontEndYaml() {
      _fifoCounter=0;
      _sequenceCount=0;
      _cfgForRunStart=false;
      daqBoard=3;
    }

    virtual ~ProcessorFrontEndYaml() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      ShmSingleton<RelayWriterDataFifo> shm2;
      ptrFifoShm2=shm2.setup(fifoKey);
      //ptrFifoShm2=nullptr;
      startFsm(rcKey);
    }

    virtual bool initializing() {     
      RecordYaml *r;

      while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      r->setHeader(_sequenceCount++);
      r->setState(FsmState::Constants);

      YAML::Node n;
      n["Source"]="FE";
      n["ElectronicsId"]=123456789;
      n["HgrocVersion"]="V3";
      n["EconDVersion"]="Emulator";
      n["EconTVersion"]="Emulator";

      std::ostringstream sout0;
      sout0 << n;
      r->setString(sout0.str());
      
      if(_printEnable) r->print();
      ptrFifoShm2->writeIncrement();
      
      writeContinuing();
      return true;
    }

    bool configuring() {
      _cfgForRunStart=true;
      _configuringBCounter=0;

      return true;
    }
    
    bool reconfiguring() {
      _cfgForRunStart=true;
      _configuringBCounter++;

      return true;
    }

    bool starting() {
      unsigned slink(1);
      unsigned lpgbtPair(3);
      
      RecordYaml *r;

      while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      r->setHeader(_sequenceCount++);
      r->setState(FsmState::Configuration);

      YAML::Node n;
      n["Source"]="Engine";
      n["DaqBoard"]=daqBoard;
      n["Slink"]=slink;
      n["LpgbtPair"]=lpgbtPair;
      n["Lpgbt"]=0;
      n["ElectronicsId"]=daqBoard<<22|slink<<18|lpgbtPair<<14|0<<10|0x3ff;
	
       std::ostringstream soutl;
       soutl << "cfg/Engine" << 0 << ".yaml";
       n["Configuration"]=YAML::LoadFile(soutl.str());

       std::ostringstream sout0;
       sout0 << n;
       r->setString(sout0.str());
	
       if(_printEnable) r->print();
       ptrFifoShm2->writeIncrement();

	
       for(unsigned e(0);e<2;e++) {
       	while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
       	r->setHeader(_sequenceCount++);
	r->setState(FsmState::Configuration);

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
	
       	while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
       	r->setHeader(_sequenceCount++);
	r->setState(FsmState::Configuration);

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
	  while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
	  r->setHeader(_sequenceCount++);
	  r->setState(FsmState::Configuration);
	
	  YAML::Node n;
	  n["Source"]="HGCROC";
	  n["DaqBoard"]=daqBoard;
	  n["Slink"]=slink;
	  n["LpgbtPair"]=lpgbtPair;
	  n["EconD"]=e;
	  n["Hgcroc"]=2*h; // Label ROC as 2x halfROC
	  n["ElectronicsId"]=daqBoard<<22|slink<<18|lpgbtPair<<14|e<<10|(2*h)<<6|0x3f;
	  
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

      writeContinuing();
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
      writeContinuing();
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

    virtual void halted() {
     if(_printEnable) {
	std::cout << "ProcessorFrontEndYaml::halted()" << std::endl;
      }
    }
    
    virtual void configured() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEndYaml::configured()" << std::endl;
      }
    }

    void running() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEndYaml::running()" << std::endl;
      }
    }

    void paused() {
      if(_printEnable) {
	std::cout << "ProcessorFrontEndYaml::paused()" << std::endl;
      }
    }

    void writeContinuing() {
      if(ptrFifoShm2!=nullptr) {
	Record *r;
	while((r=ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
	r->reset(FsmState::Continuing);
	ptrFifoShm2->writeIncrement();
      }
    }
        
  protected:
    RelayWriterDataFifo *ptrFifoShm2;
      unsigned daqBoard;

    bool _cfgForRunStart;

    uint32_t _fifoCounter;
    uint32_t _sequenceCount;
    uint32_t _keyCfgA;
    uint32_t _keyCfgB;

    uint32_t _configuringBCounter;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;
  };

}

#endif
