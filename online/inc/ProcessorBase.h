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
    
    virtual bool configuringA(FsmInterface::Handshake s) {
      return true;
    }
    
    virtual bool configuringB(FsmInterface::Handshake s) {
      return true;
    }
    
    virtual bool starting(FsmInterface::Handshake s) {
      return true;
    }
    
    virtual bool pausing(FsmInterface::Handshake s) {
      return true;
    }
    
    virtual bool resuming(FsmInterface::Handshake s) {
      return true;
    }
    
    virtual bool stopping(FsmInterface::Handshake s) {
      return true;
    }
    
    virtual bool haltingB(FsmInterface::Handshake s) {
      return true;
    }
    
    virtual bool haltingA(FsmInterface::Handshake s) {
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
      while(!_ptrFsmInterface->pong()) {
      usleep(100000);
      if(_printEnable) {
	std::cout << "Initial Ping not seen after " << 0.1*(++counter)
		  << " seconds, sleeping" << std::endl;
	_ptrFsmInterface->print();
      }
      }

      if(_printEnable) {
      std::cout << "Handled initial Ping" << std::endl;
      _ptrFsmInterface->print();
      }
      
      bool continueLoop(true);
      
      while(continueLoop) {
	if(_printEnable) {
	  std::cout << "At top of processing loop" << std::endl;
	  _ptrFsmInterface->print();
	}

	// Check if left in Ready
	if(!_ptrFsmInterface->ready()) {

	  // If not left in Prepare, wait for Prepare
	
	  if(_printEnable) {
	    std::cout << "Waiting for handshake to be Prepare" << std::endl;
	  }
	  
	  while(!_ptrFsmInterface->isPrepare()) usleep(_usSleepWait);
	  
	  if(_checkEnable) {
	    if(_ptrFsmInterface->handshake()!=FsmInterface::Prepare) {
	      std::cout << "Handshake is not Prepare" << std::endl;
	      _ptrFsmInterface->print();
	      if(_assertEnable) assert(false);
	    }
	  }
	  
	  if(_printEnable) {
	    std::cout << "Handshake is Prepare; entering transition" << std::endl;
	    _ptrFsmInterface->print();
	  }

	  if(_printEnable) {
	    std::cout << "Setting handshake to Ready" << std::endl;
	    _ptrFsmInterface->print();
	  }
	  
	  if(!_ptrFsmInterface->ready()) {
	    if(_checkEnable) {
	      std::cout << "Ready return false" << std::endl;
	      _ptrFsmInterface->print();
	      if(_assertEnable) assert(false);
	    }
	  }
	}
      	  
	if(_printEnable) {
	  std::cout << "Waiting for handshake to be GoToTransient" << std::endl;
	  _ptrFsmInterface->print();
	}

	while(!_ptrFsmInterface->isGoToTransient()) usleep(_usSleepWait);///////////////////////////////////////
			
	_ptrFsmInterface->changeProcessState();

	if(_checkEnable) {
	  if(!FsmState::transientState(_ptrFsmInterface->systemState())) {
	    std::cout << "System is not in a transient state" << std::endl;
	    _ptrFsmInterface->print();
	    if(_assertEnable) assert(false);
	  }

	  if(!FsmState::transientState(_ptrFsmInterface->processState())) {
	    std::cout << "Processor is not in a transient state" << std::endl;
	    _ptrFsmInterface->print();
	    if(_assertEnable) assert(false);
	  }
	    
	  if(!_ptrFsmInterface->matchingStates()) {
	    std::cout << "System and processor state do not match" << std::endl;
	    _ptrFsmInterface->print();
	    if(_assertEnable) assert(false);
	  }
	}

	if(_printEnable) {
	  std::cout << "Starting processing transient state" << std::endl;
	  _ptrFsmInterface->print();
	}
	    
	//assert(_ptrFsmInterface->matchingStates());

	if(     _ptrFsmInterface->processState()==FsmState::Initializing) this->initializing();
	else if(_ptrFsmInterface->processState()==FsmState::ConfiguringA) this->configuringA(_ptrFsmInterface->handshake());
	else if(_ptrFsmInterface->processState()==FsmState::ConfiguringB) this->configuringB(_ptrFsmInterface->handshake());
	else if(_ptrFsmInterface->processState()==FsmState::Starting    ) this->starting(_ptrFsmInterface->handshake());
	else if(_ptrFsmInterface->processState()==FsmState::Pausing     ) this->pausing(_ptrFsmInterface->handshake());
	else if(_ptrFsmInterface->processState()==FsmState::Resuming    ) this->resuming(_ptrFsmInterface->handshake());
	else if(_ptrFsmInterface->processState()==FsmState::Stopping    ) this->stopping(_ptrFsmInterface->handshake());
	else if(_ptrFsmInterface->processState()==FsmState::HaltingB    ) this->haltingB(_ptrFsmInterface->handshake());
	else if(_ptrFsmInterface->processState()==FsmState::HaltingA    ) this->haltingA(_ptrFsmInterface->handshake());
	else if(_ptrFsmInterface->processState()==FsmState::Resetting   ) this->resetting();
	else if(_ptrFsmInterface->processState()==FsmState::Ending      ) this->ending();
	else {
	  if(_printEnable) {
	    std::cout << "Unknown transient state" << std::endl;
	    _ptrFsmInterface->print();
	  }
	  
	  if(_assertEnable) assert(false);
	}
	      
	//assert(_ptrFsmInterface->matchingStates());
	      
	if(_printEnable) {
	  std::cout << "Finished processing transient state" << std::endl;
	  _ptrFsmInterface->print();
	}
	  
	if(!_ptrFsmInterface->completed()) {
	  if(_checkEnable) {
	    std::cout << "Completed return false" << std::endl;
	    _ptrFsmInterface->print();
	    if(_assertEnable) assert(false);
	  }
	}
	  
	if(_printEnable) {
	  std::cout << "Waiting for handshake to be GoToStatic" << std::endl;
	}
      
	while(!_ptrFsmInterface->isGoToStatic()) usleep(_usSleepWait);///////////////////////////////////////
	    
	_ptrFsmInterface->changeProcessState();

	  
	assert(_ptrFsmInterface->matchingStates());
	  
	if(_printEnable) {
	  std::cout << "Entering static state" << std::endl;
	  _ptrFsmInterface->print();
	}
	  
	assert(_ptrFsmInterface->matchingStates());
	if(_checkEnable) {
	  //if(!FsmState::staticState(_ptrFsmInterface->systemState())) {
	  //std::cout << "System is not in a static state" << std::endl;
	  //_ptrFsmInterface->print();
	  //if(_assertEnable) assert(false);
	  //}

	  if(!FsmState::staticState(_ptrFsmInterface->processState())) {
	    std::cout << "Processor is not in a static state" << std::endl;
	    _ptrFsmInterface->print();
	    if(_assertEnable) assert(false);
	  }

	  //if(!_ptrFsmInterface->matchingStates()) {
	  //std::cout << "System and processor state do not match" << std::endl;
	  //_ptrFsmInterface->print();
	  //if(_assertEnable) assert(false);
	  //}
	}

	if(_printEnable) {
	  std::cout << "Starting processing static state" << std::endl;
	  _ptrFsmInterface->print();
	}
	
	if(     _ptrFsmInterface->processState()==FsmState::Initial    ) this->initial();
	else if(_ptrFsmInterface->processState()==FsmState::Halted     ) this->halted();
	else if(_ptrFsmInterface->processState()==FsmState::ConfiguredA) this->configuredA();
	else if(_ptrFsmInterface->processState()==FsmState::ConfiguredB) this->configuredB();
	else if(_ptrFsmInterface->processState()==FsmState::Running    ) this->running();
	else if(_ptrFsmInterface->processState()==FsmState::Paused     ) this->paused();
	else if(_ptrFsmInterface->processState()==FsmState::Shutdown   );
	else {
	  if(_printEnable) {
	    std::cout << "Unknown static state" << std::endl;
	    _ptrFsmInterface->print();
	  }
	  
	  if(_assertEnable) assert(false);
	}
	
	if(_printEnable) {
	  std::cout << "Finished processing of static state" << std::endl;
	  _ptrFsmInterface->print();
	}

	// If left in GoToStatic, bring to Idle
	_ptrFsmInterface->idle();
	
	
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
