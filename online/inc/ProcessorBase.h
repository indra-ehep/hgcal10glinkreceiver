#ifndef Hgcal10gLinkReceiver_ProcessorBase_h
#define Hgcal10gLinkReceiver_ProcessorBase_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <thread>

#include "SystemParameters.h"
#include "FsmInterface.h"
#include "ShmSingleton.h"
#include "RecordPrinter.h"

//using namespace std::chrono_literals;

namespace Hgcal10gLinkReceiver {

  class ProcessorBase {

  public:
  ProcessorBase() : _printEnable(false), _checkEnable(false), _assertEnable(false) {
      for(unsigned i(0);i<FsmState::EndOfStaticEnum;i++) {
	_usSleepWait=1000;
	_usSleep[i]=1000;
      }
    }

    virtual ~ProcessorBase() {
    }
    
    virtual void setPrintEnable(bool p) {
      _printEnable=p;
    }

    virtual void setCheckEnable(bool c) {
      _checkEnable=c;
    }

    virtual void setAssertEnable(bool a) {
      _assertEnable=a;
    }

    virtual bool initializing() {
      return true;
    }
    
    virtual bool configuringA() {
      return true;
    }
    
    virtual bool configuringB() {
      return true;
    }
    
    virtual bool starting() {
      return true;
    }
    
    virtual bool pausing() {
      return true;
    }
    
    virtual bool resuming() {
      return true;
    }
    
    virtual bool stopping() {
      return true;
    }
    
    virtual bool haltingB() {
      return true;
    }
    
      virtual bool haltingA() {
      return true;
    }
    
    virtual bool resetting() {
      return true;
    }

    virtual bool ending() {
      return true;
    }

    //////////////////////////////////////////////
    
    virtual void initial() {
    }
    
    virtual void halted() {
    }
    
    virtual void configuredA() {
    }
    
    virtual void configuredB() {
    }
    
    virtual void running() {
    }
    
    virtual void paused() {
    }

    virtual void startFsm(uint32_t theKey) {
  
      // Connect to shared memory
      ShmSingleton<FsmInterface> shmU;
      _ptrFsmInterface=shmU.setup(theKey);
      // /*volatile*/ _ptrFsmInterface=shmU.payload();

      // Put interface into a good state
      if(_printEnable) {
	std::cout << "Sending coldStart to FsmInterface" << std::endl;
      }

      _ptrFsmInterface->coldStart();

      if(_printEnable) {
	_ptrFsmInterface->print();
      }

      // Wait for Ping from Run Control
      if(_printEnable) {
	std::cout << "Waiting for initial Ping" << std::endl;
      }

      unsigned counter(0);
      //while(!_ptrFsmInterface->pong()) {
      while(_ptrFsmInterface->systemState()!=FsmState::Initial) {
	usleep(100000);
	if(_printEnable) {
	  std::cout << "Initial Ping not seen after " << 0.1*(++counter)
		    << " seconds, sleeping" << std::endl;
	  _ptrFsmInterface->print();
	}
      }

      // Do initial tasks
      this->initial();      

      // Reply if not already done
      if(_ptrFsmInterface->processState()==FsmState::Resetting) {
	_ptrFsmInterface->setProcessState(FsmState::Initial);
      }

      if(_printEnable) {
	std::cout << "Handled initial Ping" << std::endl;
	_ptrFsmInterface->print();
      }


      /////////////////////////////////////////////////////////////////////

      
      bool continueLoop(true);
      
      while(continueLoop) {
	if(_printEnable) {
	  std::cout << "At top of processing loop: checking if still in static state" << std::endl;
	  _ptrFsmInterface->print();
	}

	if(FsmState::staticState(_ptrFsmInterface->processState())) {
	
	  if(_printEnable) {
	    std::cout << "Waiting for pre-transient system state" << std::endl;
	    _ptrFsmInterface->print();
	  }
	  
	  while(!FsmState::transientState(_ptrFsmInterface->systemState())) usleep(_usSleepWait);

	  if(_checkEnable) {
	    if(_ptrFsmInterface->processState()!=FsmState::staticStateBeforeTransient(_ptrFsmInterface->systemState())) {
	      std::cout << "Processor state is not state before system transient" << std::endl;
	      _ptrFsmInterface->print();
	      if(_assertEnable) assert(false);
	    }
	  }

	  if(_printEnable) {
	    std::cout << "Pre-transition state seen, immediately setting Continuing" << std::endl;
	    _ptrFsmInterface->print();
	  }

	  _ptrFsmInterface->setProcessState(FsmState::Continuing);
	}
      	  
	if(_printEnable) {
	  std::cout << "Check or wait for Transient" << std::endl;
	  _ptrFsmInterface->print();
	}

	while(_ptrFsmInterface->record().utc()==0) usleep(_usSleepWait);///////////////////////////////////////

	if(_checkEnable) {
	  if(!FsmState::transientState(_ptrFsmInterface->systemState())) {
	    std::cout << "System is not in a transient state" << std::endl;
	    _ptrFsmInterface->print();
	    if(_assertEnable) assert(false);
	  }

	  if(_ptrFsmInterface->processState()!=FsmState::Continuing) {
	    std::cout << "Processor is not in Continuing state" << std::endl;
	    _ptrFsmInterface->print();
	    if(_assertEnable) assert(false);
	  }
	}

	if(_printEnable) {
	  std::cout << "Transient seen, starting processing transient state" << std::endl;
	  _ptrFsmInterface->print();
	}
	    
	if(     _ptrFsmInterface->systemState()==FsmState::Initializing) this->initializing();
	else if(_ptrFsmInterface->systemState()==FsmState::ConfiguringA) this->configuringA();
	else if(_ptrFsmInterface->systemState()==FsmState::ConfiguringB) this->configuringB();
	else if(_ptrFsmInterface->systemState()==FsmState::Starting    ) this->starting();
	else if(_ptrFsmInterface->systemState()==FsmState::Pausing     ) this->pausing();
	else if(_ptrFsmInterface->systemState()==FsmState::Resuming    ) this->resuming();
	else if(_ptrFsmInterface->systemState()==FsmState::Stopping    ) this->stopping();
	else if(_ptrFsmInterface->systemState()==FsmState::HaltingB    ) this->haltingB();
	else if(_ptrFsmInterface->systemState()==FsmState::HaltingA    ) this->haltingA();
	else if(_ptrFsmInterface->systemState()==FsmState::Resetting   ) this->resetting();
	else if(_ptrFsmInterface->systemState()==FsmState::Ending      ) this->ending();
	else {
	  if(_printEnable) {
	    std::cout << "Unknown transient state" << std::endl;
	    _ptrFsmInterface->print();
	  }
	  
	  if(_assertEnable) assert(false);
	}
	      
	if(_printEnable) {
	  std::cout << "Finished processing transient state, change to Trnasient state" << std::endl;
	  _ptrFsmInterface->print();
	}
	  
	_ptrFsmInterface->changeProcessState();

	if(_printEnable) {
	  std::cout << "Waiting to move to static state" << std::endl;
	  _ptrFsmInterface->print();
	}
      
	while(!FsmState::staticState(_ptrFsmInterface->systemState())) usleep(_usSleepWait);
	    
	if(_printEnable) {
	  std::cout << "Seen static state, starting static processing" << std::endl;
	  _ptrFsmInterface->print();
	}

	if(     _ptrFsmInterface->systemState()==FsmState::Initial    ) this->initial();
	else if(_ptrFsmInterface->systemState()==FsmState::Halted     ) this->halted();
	else if(_ptrFsmInterface->systemState()==FsmState::ConfiguredA) this->configuredA();
	else if(_ptrFsmInterface->systemState()==FsmState::ConfiguredB) this->configuredB();
	else if(_ptrFsmInterface->systemState()==FsmState::Running    ) this->running();
	else if(_ptrFsmInterface->systemState()==FsmState::Paused     ) this->paused();
	else if(_ptrFsmInterface->systemState()==FsmState::Shutdown   );
	else {
	  if(_printEnable) {
	    std::cout << "Unknown static state" << std::endl;
	    _ptrFsmInterface->print();
	  }
	  
	  if(_assertEnable) assert(false);
	}
	
	if(_printEnable) {
	  std::cout << "Finished processing of static state, change to Static state if not already" << std::endl;
	  _ptrFsmInterface->print();
	}

	if(FsmState::staticStateAfterTransient(_ptrFsmInterface->processState())==_ptrFsmInterface->systemState()) {
	  _ptrFsmInterface->setProcessState(_ptrFsmInterface->systemState());
	  
	  if(_checkEnable) {
	    if(!FsmState::staticState(_ptrFsmInterface->systemState())) {
	      std::cout << "System is not in a static state" << std::endl;
	      _ptrFsmInterface->print();
	      if(_assertEnable) assert(false);
	    }
	    
	    if(!FsmState::staticState(_ptrFsmInterface->processState())) {
	      std::cout << "Processor is not in a static state" << std::endl;
	      _ptrFsmInterface->print();
	      if(_assertEnable) assert(false);
	    }
	    
	    if(!_ptrFsmInterface->matchingStates()) {
	      std::cout << "System and processor state do not match" << std::endl;
	      _ptrFsmInterface->print();
	      if(_assertEnable) assert(false);
	    }
	  }
	}
		
	if(_ptrFsmInterface->processState()==FsmState::Shutdown) continueLoop=false;
	
	if(_printEnable) {
	  std::cout << "At end of processing loop" << std::endl;
	  _ptrFsmInterface->print();
	}
      }

      

      return;
    }
    
    protected:
      bool _printEnable;
      bool _checkEnable;
      bool _assertEnable;

      FsmInterface *_ptrFsmInterface;
      unsigned _usSleep[FsmState::EndOfStaticEnum];
      unsigned _usSleepWait;
  };

  }

#endif
