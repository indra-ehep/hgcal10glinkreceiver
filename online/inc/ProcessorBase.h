#ifndef ProcessorBase_h
#define ProcessorBase_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"
#include "ShmKeys.h"

namespace Hgcal10gLinkReceiver {

  class ProcessorBase {

  public:
    ProcessorBase() {
      for(unsigned i(0);i<RunControlFsmEnums::EndOfStaticEnum;i++) {
	_usSleep[i]=1000;
      }
    }

    virtual ~ProcessorBase() {
    }
    
    virtual void setPrintEnable(bool p) {
      _printEnable=p;
    }

    virtual void initializing() {
    }
    
    virtual void configuringA() {
    }
    
    virtual void configuringB() {
    }
    
    virtual void starting() {
      std::cout << "********************************************************** Starting a run" << std::endl;
    }
    
    virtual void pausing() {
    }
    
    virtual void resuming() {
    }
    
    virtual void stopping() {
      std::cout << "********************************************************** Stopping a run" << std::endl;
    }
    
    virtual void haltingB() {
    }
    
    virtual void haltingA() {
    }
    
    virtual void resetting() {
    }

    //////////////////////////////////////////////
    
    virtual void initial() {
    }
    
    virtual void halted() {
    }
    
    virtual void configuredA() {
    }
    
    virtual void configuredB() {
      assert(false);
    }
    
    virtual void running(RunControlFsmShm* const p) {
      std::cout << "********************************************************** Doing a run" << std::endl;
      unsigned n(0);
      while(p->isStaticState()) {
	n++;
	usleep(_usSleep[p->processState()]);      

	p->pong();
      }
      std::cout << "********************************************************** Done  a run " << n << std::endl;
    }
    
    virtual void paused() {
    }

    virtual void startFsm(uint32_t theKey) {
  
      // Connect to shared memory
      ShmSingleton<RunControlFsmShm> shmU;
      shmU.setup(theKey);
      RunControlFsmShm* const ptrRunFileShm(shmU.payload());

      // Force to Initial on startup
      if(_printEnable) ptrRunFileShm->print();
      ptrRunFileShm->forceProcessState(RunControlFsmEnums::Initial);
      ptrRunFileShm->forceProcessError(RunControlFsmEnums::Good);
      ptrRunFileShm->_handshakeState=RunControlFsmShm::StaticState;
      if(_printEnable) ptrRunFileShm->print();

      while(!ptrRunFileShm->pong());

      /*
	ptrRunFileShm->setCommand(RunControlFsmEnums::Initialize);
	if(_printEnable) ptrRunFileShm->print();
      */

      /*
	bool ponged(false);
	ponged=ptrRunFileShm->pon
	if(ponged) std::cout << "PONGED1" << std::endl;
      
	while(ptrRunFileShm->isStaticState() && !ponged) {
	//if(_printEnable) ptrRunFileShm->print();
	ponged=ptrRunFileShm->pong();
	if(ponged) std::cout << "PONGED2" << std::endl;
	}
	sleep(1);
      */

      //if(ptrRunFileShm->isStaticState()) {
      if(_printEnable) {
	std::cout << "Waiting for system match" << std::endl;
	ptrRunFileShm->print();
      }

      //while(!ptrRunFileShm->matchingStates());

      if(_printEnable) {
	std::cout << "Got system match" << std::endl;
	ptrRunFileShm->print();
      }
      
      while(true) {
	//if(ptrRunFileShm->isStaticState()) {
	if(_printEnable) {
	  std::cout << "Start processing static state" << std::endl;
	  ptrRunFileShm->print();
	}

	//assert(ptrRunFileShm->matchingStates());

	if(     ptrRunFileShm->processState()==RunControlFsmEnums::Initial    ) this->initial();
	else if(ptrRunFileShm->processState()==RunControlFsmEnums::Halted     ) this->halted();
	else if(ptrRunFileShm->processState()==RunControlFsmEnums::ConfiguredA) this->configuredA();
	else if(ptrRunFileShm->processState()==RunControlFsmEnums::ConfiguredB) this->configuredB();
	else if(ptrRunFileShm->processState()==RunControlFsmEnums::Running    ) this->running(ptrRunFileShm);
	else if(ptrRunFileShm->processState()==RunControlFsmEnums::Paused     ) this->paused();
	else assert(false);
	  
	if(_printEnable) {
	  std::cout << "Done processing static state" << std::endl;
	  ptrRunFileShm->print();
	}

	while(ptrRunFileShm->isStaticState());

	if(_printEnable) {
	  std::cout << "Now a non-static handshake" << std::endl;
	  ptrRunFileShm->print();
	}

	//} else {
	if(ptrRunFileShm->pong()) {
	  if(_printEnable) {
	    std::cout << "Did pong" << std::endl;
	    ptrRunFileShm->print();
	  }

	} else {
	  assert(ptrRunFileShm->_handshakeState==RunControlFsmShm::Prepare);
	  if(_printEnable) {
	    std::cout << "Entering new command" << std::endl;
	    ptrRunFileShm->print();
	  }

	  assert(ptrRunFileShm->matchingStates());

	  // PREPARING

	  assert(ptrRunFileShm->matchingStates());

	  //while(!ptrRunFileShm->prepared(true));
	  if(ptrRunFileShm->systemState()!=RunControlFsmEnums::ConfiguringB) {
	    assert(ptrRunFileShm->accepted());
	  } else {
	    assert(ptrRunFileShm->rejected());
	  }

	  while(!ptrRunFileShm->isProceed());///////////////////////////////////////

	  bool change(ptrRunFileShm->isChange());
			
	  if(ptrRunFileShm->isChange()) {
	    ptrRunFileShm->forceProcessState(RunControlFsmEnums::transitionStateForCommand(ptrRunFileShm->command()));
	    if(_printEnable) {
	      std::cout << "Entered transient state" << std::endl;
	      ptrRunFileShm->print();
	    }
	    
	    assert(ptrRunFileShm->matchingStates());

	    if(     ptrRunFileShm->processState()==RunControlFsmEnums::Initializing) this->initializing();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::ConfiguringA) this->configuringA();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::ConfiguringB) this->configuringB();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::Starting    ) this->starting();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::Pausing     ) this->pausing();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::Resuming    ) this->resuming();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::Stopping    ) this->stopping();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::HaltingB    ) this->haltingB();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::HaltingA    ) this->haltingA();
	    else if(ptrRunFileShm->processState()==RunControlFsmEnums::Resetting   ) this->resetting();
	    else assert(false);
	      
	    assert(ptrRunFileShm->matchingStates());
	      
	    if(_printEnable) {
	      std::cout << "Done transient state" << std::endl;
	      ptrRunFileShm->print();
	    }
	      
	  } else if(ptrRunFileShm->isRepair()) {
	    if(_printEnable) {
	      std::cout << "Doing repair" << std::endl;
	      ptrRunFileShm->print();
	    }

	    assert(ptrRunFileShm->matchingStates());

	    // REPAIRING
	      
	    assert(ptrRunFileShm->matchingStates());	      
	  }
	    
	  assert(ptrRunFileShm->completed());

	  while(!ptrRunFileShm->isStartStatic());///////////////////////////////////////
	    
	  if(change) ptrRunFileShm->forceProcessState(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command()));

	  assert(ptrRunFileShm->matchingStates());

	  if(_printEnable) {
	    std::cout << "Entering static state" << std::endl;
	    ptrRunFileShm->print();
	  }
	  
	  assert(ptrRunFileShm->matchingStates());

	  assert(ptrRunFileShm->ended());
	}
	//}
      }
      
      return;
      
      while(true) {
	std::cout << "HERE0 ";ptrRunFileShm->print();
	sleep(1);
	if(!ptrRunFileShm->rcLock()) {

	  // In transition
	  std::cout << "HERE1 ";ptrRunFileShm->print();
	  sleep(1);

	  switch(ptrRunFileShm->processState()) {

	    // Static states   /////////////////////

	    // INITIAL //
      
	  case RunControlFsmEnums::Initial: {

	    if(ptrRunFileShm->command()==RunControlFsmEnums::Initialize) {
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());
	
	      // Initializing; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Initializing,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	
	      initializing();
      
	      // Set to Halted static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Halted,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	      halted();
	      
	    } else if(ptrRunFileShm->command()==RunControlFsmEnums::Reset) {
	      if(_printEnable) ptrRunFileShm->print();

	      // Resetting; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Initializing,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	
	      resetting();
      
	      // Set to Initial static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Initial,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // HALTED //
      
	  case RunControlFsmEnums::Halted: {

	    if(ptrRunFileShm->command()==RunControlFsmEnums::ConfigureA) {
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	      // ConfiguringA; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguringA,RunControlFsmEnums::Good);

	      configuringA();
      
	      // Set to ConfiguredA static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguredA,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());


	    } else if(ptrRunFileShm->command()==RunControlFsmEnums::Reset) {

	      // Resetting; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Resetting,RunControlFsmEnums::Good);

	      resetting();
      
	      // Set to Initial static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Initial,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // CONFIGUREDA //

	  case RunControlFsmEnums::ConfiguredA: {

	    if(ptrRunFileShm->command()==RunControlFsmEnums::ConfigureB) {
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());
      
	      // ConfiguringB; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguringB,RunControlFsmEnums::Good);

	      //configuringB();
      
	      // Set to ConfiguredB static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguredB,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	
	    } else if(ptrRunFileShm->command()==RunControlFsmEnums::HaltA) {
      
	      // HaltingB; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::HaltingB,RunControlFsmEnums::Good);

	      haltingB();
            
	      // Set to Halted static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Halted,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // CONFIGUREDB
      
	  case RunControlFsmEnums::ConfiguredB: {

	    if(ptrRunFileShm->command()==RunControlFsmEnums::Start) {
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());
      
	      // Starting; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Starting,RunControlFsmEnums::Good);

	      starting();
      
	      // Set to Running static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Running,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	
	    } else if(ptrRunFileShm->command()==RunControlFsmEnums::HaltB) {
      
	      // HaltingB; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::HaltingB,RunControlFsmEnums::Good);

	      haltingB();
      
	      // Set to ConfiguredA static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguredA,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // RUNNING //
      
	  case RunControlFsmEnums::Running: {

	    if(ptrRunFileShm->command()==RunControlFsmEnums::Pause) {
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	      // Pausing; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Pausing,RunControlFsmEnums::Good);

	      pausing();

	      // Set to Paused static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Paused,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	
	    } else if(ptrRunFileShm->command()==RunControlFsmEnums::Stop) {
      
	      // Stopping; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Stopping,RunControlFsmEnums::Good);

	      stopping();
      
	      // Set to ConfiguredB static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguredB,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // PAUSED //
      
	  case RunControlFsmEnums::Paused: {

	    if(ptrRunFileShm->command()==RunControlFsmEnums::Resume) {
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	      // Starting; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Starting,RunControlFsmEnums::Good);

	      starting();
	
	      // Set to static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Running,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }
	    // NULL = Initial //
      
	  case RunControlFsmEnums::EndOfStateEnum: {

	    if(ptrRunFileShm->command()==RunControlFsmEnums::Initialize) {
	      if(_printEnable) ptrRunFileShm->print();

	      // Initializing; set to transitional state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Initializing,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();

	      initializing();
      
	      // Set to Halted static state
	      ptrRunFileShm->setProcess(RunControlFsmEnums::Halted,RunControlFsmEnums::Good);
	      if(_printEnable) ptrRunFileShm->print();
	      assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->processState());
	    }
	    break;
	  }

      
	    // Transitional states: should never happen   /////////////////////


	  case RunControlFsmEnums::Initializing: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to Halted static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::Halted,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }
    
	  case RunControlFsmEnums::ConfiguringA: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to ConfiguredA static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguredA,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }
    
	  case RunControlFsmEnums::HaltingB: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to Halted static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::Halted,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }

	  case RunControlFsmEnums::ConfiguringB: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to ConfiguredB static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguredB,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }

	  case RunControlFsmEnums::HaltingA: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguredA,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }

	  case RunControlFsmEnums::Starting: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to Running static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::Running,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }

	  case RunControlFsmEnums::Stopping: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to ConfiguredB static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::ConfiguredB,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }

	  case RunControlFsmEnums::Pausing: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to Paused static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::Paused,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }

	  case RunControlFsmEnums::Resuming: {
	    if(_printEnable) ptrRunFileShm->print();
    
	    // Set to Running static state
	    ptrRunFileShm->setProcess(RunControlFsmEnums::Running,RunControlFsmEnums::Warning);
	    if(_printEnable) ptrRunFileShm->print();
	    break;
	  }

	  default: {
	    if(_printEnable) ptrRunFileShm->print();
	    assert(false);
	    break;
	  }
	  };

	  //////////////////////////////////////////////////////////////////
	  
	} else if(ptrRunFileShm->matchingStates()) {
	  std::cout << "HERE2 ";ptrRunFileShm->print();
	  sleep(1);

	  // In static state

	  switch(ptrRunFileShm->processState()) {

	  case RunControlFsmEnums::Initial: {
	    initial();
	    break;
	  }
	    
	  case RunControlFsmEnums::Halted: {
	    halted();
	    break;
	  }

	  case RunControlFsmEnums::ConfiguredA: {
	    configuredA();
	    break;
	  }

	  case RunControlFsmEnums::ConfiguredB: {
	    configuredB();
	    break;
	  }

	  case RunControlFsmEnums::Running: {
	    
	    running(ptrRunFileShm);
	    break;
	  }

	  case RunControlFsmEnums::Paused: {
	    paused();
	    break;
	  }
	    
	  default: {
	    if(_printEnable) ptrRunFileShm->print();
	    assert(false);
	    break;
	  }
	  }

	  //while(!ptrRunFileShm->matchingStates()) {
	  while(!ptrRunFileShm->rcLock()) {
	    usleep(_usSleep[ptrRunFileShm->processState()]);
	  }
	} else {
	  std::cout << "HERE3 ";ptrRunFileShm->print();
	  sleep(1);
	}

      }
    }
   
  protected:
    bool _printEnable;
    unsigned _usSleep[RunControlFsmEnums::EndOfStaticEnum];
  };

}

#endif
