#ifndef Hgcal10gLinkReceiver_ProcessorStage_h
#define Hgcal10gLinkReceiver_ProcessorStage_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

//#include <yaml-cpp/yaml.h>

#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "RecordPrinter.h"
#include "RecordYaml.h"
#include "DataFifo.h"

namespace Hgcal10gLinkReceiver {

  class ProcessorStage : public ProcessorBase {
    
  public:
    ProcessorStage() {
      std::cout << "STAGE!" << std::endl;

      _fifoCounter=0;
      _sequenceCount=0;
      _cfgForRunStart=false;
    }

    virtual ~ProcessorStage() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      ShmSingleton<RelayWriterDataFifo> shm2;
      ptrFifoShm2=shm2.setup(fifoKey);

      startFsm(rcKey);
    }

    virtual bool initializing() {     
      RecordYaml *r;

      while((r=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      r->setHeader(_sequenceCount++);
      r->setState(FsmState::Constants);

      std::ostringstream sout0;
      /*
      YAML::Node n;
      n["Source"]="FE";
      n["ElectronicsId"]=123456789;
      n["HgrocVersion"]="V3";
      n["EconDVersion"]="Emulator";
      n["EconTVersion"]="Emulator";

      sout0 << n;
      */
      sout0 << "Source: Stage" << std::endl
	    << "ElectronicsId: " << 0xffffffff;
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

      std::system("touch cfg/coordinates.txt");
      
      bool readOk(false);
      double x(0.0),y(0.0),a(0.0);

      std::ifstream fin("cfg/coordinates.txt");

      char c;
      if(fin) {
	fin >> x;
	fin >> c;
	//std::cout << "c = " << c << " = " << unsigned(c) << std::endl;
	fin >> y;
	fin >> c;
	//std::cout << "c = " << c << " = " << unsigned(c) << std::endl;
	fin >> a;

	if(fin) readOk=true;
      }
      
      std::ostringstream sout;
      sout << "Source: Stage" << std::endl
	   << "ElectronicsId: " << 0xffffffff << std::endl
	   << "Configuration:" << std::endl
	   << "  ReadOK: " << (readOk?"True":"False") << std::endl
	   << "  StageXmm: " << x << std::endl
	   << "  StageYmm: " << y << std::endl
	   << "  StageAdeg: " << a;
	
      r->setString(sout.str());
      
      if(_printEnable) r->print();
      ptrFifoShm2->writeIncrement();
      

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
	std::cout << "ProcessorStage::halted()" << std::endl;
      }
    }
    
    virtual void configured() {
      if(_printEnable) {
	std::cout << "ProcessorStage::configured()" << std::endl;
      }
    }

    void running() {
      if(_printEnable) {
	std::cout << "ProcessorStage::running()" << std::endl;
      }
    }

    void paused() {
      if(_printEnable) {
	std::cout << "ProcessorStage::paused()" << std::endl;
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
