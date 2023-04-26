#ifndef ProcessorBase_h
#define ProcessorBase_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <thread>

#include "FsmInterface.h"
#include "ShmSingleton.h"
#include "ShmKeys.h"
#include "RecordConfiguringA.h"

//using namespace std::chrono_literals;

namespace Hgcal10gLinkReceiver {

  class ProcessorBase {

  public:
    ProcessorBase() {
      for(unsigned i(0);i<FsmState::EndOfStaticEnum;i++) {
	_usSleep[i]=1000;
      }
    }

    virtual ~ProcessorBase() {
    }
    
    virtual void setPrintEnable(bool p) {
      _printEnable=p;
    }

    virtual bool initializing(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool configuringA(FsmInterface::HandshakeState s) {
      RecordConfiguringA rca;
      rca.copy(ptrFsmInterface->commandPacket().recordHeader());
      rca.print();
      return true;
    }
    
    virtual bool configuringB(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool starting(FsmInterface::HandshakeState s) {
      std::cout << "********************************************************** Starting a run" << std::endl;
      return true;
    }
    
    virtual bool pausing(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool resuming(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool stopping(FsmInterface::HandshakeState s) {
      std::cout << "********************************************************** Stopping a run" << std::endl;
      return true;
    }
    
    virtual bool haltingB(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool haltingA(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool resetting(FsmInterface::HandshakeState s) {
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
      assert(false);
    }
    
    virtual void running() {
      std::cout << "********************************************************** Doing a run" << std::endl;
      unsigned n(0);
      while(ptrFsmInterface->isIdle()) {
	n++;
	usleep(_usSleep[ptrFsmInterface->processState()]);      
      }
      std::cout << "********************************************************** Done  a run " << n << std::endl;
    }
    
    virtual void paused() {
    }

    virtual void startFsm(uint32_t theKey) {
  
      // Connect to shared memory
      ShmSingleton<FsmInterface> shmU;
      shmU.setup(theKey);
      /*volatile*/ ptrFsmInterface=shmU.payload();

      // Force to Initial on startup
      /*
      if(_printEnable) ptrFsmInterface->print();
      ptrFsmInterface->forceProcessState(FsmState::Initial);
      //ptrFsmInterface->forceProcessError(FsmState::Good);
      ptrFsmInterface->setCommandHandshake(FsmInterface::Idle);
      */
      ptrFsmInterface->initialize();
      if(_printEnable) ptrFsmInterface->print();

      while(!ptrFsmInterface->pong()) usleep(1);

      /*
	ptrFsmInterface->setCommand(FsmState::Initialize);
	if(_printEnable) ptrFsmInterface->print();
      */

      /*
	bool ponged(false);
	ponged=ptrFsmInterface->pon
	if(ponged) std::cout << "PONGED1" << std::endl;
      
	while(ptrFsmInterface->isIdle() && !ponged) {
	//if(_printEnable) ptrFsmInterface->print();
	ponged=ptrFsmInterface->pong();
	if(ponged) std::cout << "PONGED2" << std::endl;
	}
	sleep(1);
      */

      //if(ptrFsmInterface->isIdle()) {
      if(_printEnable) {
	std::cout << "Waiting for system match" << std::endl;
	ptrFsmInterface->print();
      }

      //while(!ptrFsmInterface->matchingStates());

      if(_printEnable) {
	std::cout << "Got system match" << std::endl;
	ptrFsmInterface->print();
      }
      
      while(true) {
	//if(ptrFsmInterface->isIdle()) {
	if(_printEnable) {
	  std::cout << "Start processing static state" << std::endl;
	  ptrFsmInterface->print();
	}

	//assert(ptrFsmInterface->matchingStates());

	if(     ptrFsmInterface->processState()==FsmState::Initial    ) this->initial();
	else if(ptrFsmInterface->processState()==FsmState::Halted     ) this->halted();
	else if(ptrFsmInterface->processState()==FsmState::ConfiguredA) this->configuredA();
	else if(ptrFsmInterface->processState()==FsmState::ConfiguredB) this->configuredB();
	else if(ptrFsmInterface->processState()==FsmState::Running    ) this->running();
	else if(ptrFsmInterface->processState()==FsmState::Paused     ) this->paused();
	else assert(false);
	  
	if(_printEnable) {
	  std::cout << "Done processing static state" << std::endl;
	  ptrFsmInterface->print();
	}

	while(ptrFsmInterface->isIdle()) usleep(1);

	if(_printEnable) {
	  std::cout << "Now a non-static handshake" << std::endl;
	  ptrFsmInterface->print();
	}

	//} else {
	if(ptrFsmInterface->pong()) {
	  if(_printEnable) {
	    std::cout << "Did pong" << std::endl;
	    ptrFsmInterface->print();
	  }

	} else {
	  ptrFsmInterface->setCommandHandshake(FsmInterface::Propose);
	  if(_printEnable) {
	    std::cout << "Entering new command" << std::endl;
	    ptrFsmInterface->print();
	  }

	  assert(ptrFsmInterface->matchingStates());

	  // PREPARING

	  assert(ptrFsmInterface->matchingStates());

	  //while(!ptrFsmInterface->prepared(true));
	  //if(ptrFsmInterface->systemState()!=FsmState::ConfiguringB) {
	    assert(ptrFsmInterface->accepted());
	    //} else {
	    //assert(ptrFsmInterface->rejected());
	    //}

	  while(!ptrFsmInterface->isProceed()) usleep(1);///////////////////////////////////////

	  bool change(ptrFsmInterface->isChange());
			
	  if(ptrFsmInterface->isChange()) {
	    //ptrFsmInterface->forceProcessState(FsmCommand::transitionStateForCommand(ptrFsmInterface->commandPacket().command()));
	    ptrFsmInterface->changeProcessState();
	    if(_printEnable) {
	      std::cout << "Entered transient state" << std::endl;
	      ptrFsmInterface->print();
	    }
	    
	    //assert(ptrFsmInterface->matchingStates());

	    if(     ptrFsmInterface->processState()==FsmState::Initializing) this->initializing(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::ConfiguringA) this->configuringA(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::ConfiguringB) this->configuringB(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::Starting    ) this->starting(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::Pausing     ) this->pausing(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::Resuming    ) this->resuming(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::Stopping    ) this->stopping(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::HaltingB    ) this->haltingB(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::HaltingA    ) this->haltingA(ptrFsmInterface->commandHandshake());
	    else if(ptrFsmInterface->processState()==FsmState::Resetting   ) this->resetting(ptrFsmInterface->commandHandshake());
	    else assert(false);
	      
	    //assert(ptrFsmInterface->matchingStates());
	      
	    if(_printEnable) {
	      std::cout << "Done transient state" << std::endl;
	      ptrFsmInterface->print();
	    }
	      
	  } else if(ptrFsmInterface->isRepair()) {
	    if(_printEnable) {
	      std::cout << "Doing repair" << std::endl;
	      ptrFsmInterface->print();
	    }

	    assert(ptrFsmInterface->matchingStates());

	    // REPAIRING
	      
	    assert(ptrFsmInterface->matchingStates());	      
	  }
	    
	  assert(ptrFsmInterface->completed());

	  while(!ptrFsmInterface->isStartStatic()) usleep(1);///////////////////////////////////////
	    
	  if(change) ptrFsmInterface->forceProcessState(FsmCommand::staticStateAfterCommand(ptrFsmInterface->commandPacket().command()));

	  assert(ptrFsmInterface->matchingStates());

	  if(_printEnable) {
	    std::cout << "Entering static state" << std::endl;
	    ptrFsmInterface->print();
	  }
	  
	  assert(ptrFsmInterface->matchingStates());

	  assert(ptrFsmInterface->ended());
	}
	//}
      }
      
      return;
#ifdef JUNK      
      while(true) {
	std::cout << "HERE0 ";ptrFsmInterface->print();
	sleep(1);
	if(!ptrFsmInterface->rcLock()) {

	  // In transition
	  std::cout << "HERE1 ";ptrFsmInterface->print();
	  sleep(1);

	  switch(ptrFsmInterface->processState()) {

	    // Static states   /////////////////////

	    // INITIAL //
      
	  case FsmState::Initial: {

	    if(ptrFsmInterface->commandPacket().command()==FsmState::Initialize) {
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());
	
	      // Initializing; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::Initializing);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	
	      initializing();
      
	      // Set to Halted static state
	      ptrFsmInterface->setProcess(FsmState::Halted);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	      halted();
	      
	    } else if(ptrFsmInterface->commandPacket().command()==FsmState::Reset) {
	      if(_printEnable) ptrFsmInterface->print();

	      // Resetting; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::Initializing);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	
	      resetting();
      
	      // Set to Initial static state
	      ptrFsmInterface->setProcess(FsmState::Initial);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // HALTED //
      
	  case FsmState::Halted: {

	    if(ptrFsmInterface->commandPacket().command()==FsmState::ConfigureA) {
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	      // ConfiguringA; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::ConfiguringA);//,FsmState::Good);

	      configuringA();
      
	      // Set to ConfiguredA static state
	      ptrFsmInterface->setProcess(FsmState::ConfiguredA);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());


	    } else if(ptrFsmInterface->commandPacket().command()==FsmState::Reset) {

	      // Resetting; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::Resetting);//,FsmState::Good);

	      resetting();
      
	      // Set to Initial static state
	      ptrFsmInterface->setProcess(FsmState::Initial);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // CONFIGUREDA //

	  case FsmState::ConfiguredA: {

	    if(ptrFsmInterface->commandPacket().command()==FsmCommand::ConfigureB) {
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());
      
	      // ConfiguringB; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::ConfiguringB);//,FsmState::Good);

	      //configuringB();
      
	      // Set to ConfiguredB static state
	      ptrFsmInterface->setProcess(FsmState::ConfiguredB);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	
	    } else if(ptrFsmInterface->commandPacket().command()==FsmCommand::HaltA) {
      
	      // HaltingB; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::HaltingB);//,FsmState::Good);

	      haltingB();
            
	      // Set to Halted static state
	      ptrFsmInterface->setProcess(FsmState::Halted);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // CONFIGUREDB
      
	  case FsmState::ConfiguredB: {

	    if(ptrFsmInterface->commandPacket().command()==FsmCommand::Start) {
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());
      
	      // Starting; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::Starting);//,FsmState::Good);

	      starting();
      
	      // Set to Running static state
	      ptrFsmInterface->setProcess(FsmState::Running);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	
	    } else if(ptrFsmInterface->commandPacket().command()==FsmCommand::HaltB) {
      
	      // HaltingB; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::HaltingB);//,FsmState::Good);

	      haltingB();
      
	      // Set to ConfiguredA static state
	      ptrFsmInterface->setProcess(FsmState::ConfiguredA);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // RUNNING //
      
	  case FsmState::Running: {

	    if(ptrFsmInterface->commandPacket().command()==FsmCommand::Pause) {
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	      // Pausing; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::Pausing);//,FsmState::Good);

	      pausing();

	      // Set to Paused static state
	      ptrFsmInterface->setProcess(FsmState::Paused);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	
	    } else if(ptrFsmInterface->commandPacket().command()==FsmCommand::Stop) {
      
	      // Stopping; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::Stopping);//,FsmState::Good);

	      stopping();
      
	      // Set to ConfiguredB static state
	      ptrFsmInterface->setProcess(FsmState::ConfiguredB);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // PAUSED //
      
	  case FsmState::Paused: {

	    if(ptrFsmInterface->commandPacket().command()==FsmCommand::Resume) {
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	      // Starting; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::Starting);//,FsmState::Good);

	      starting();
	
	      // Set to static state
	      ptrFsmInterface->setProcess(FsmState::Running);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }
	    // NULL = Initial //
      
	  case FsmState::EndOfStateEnum: {

	    if(ptrFsmInterface->commandPacket().command()==FsmCommand::Initialize) {
	      if(_printEnable) ptrFsmInterface->print();

	      // Initializing; set to transitional state
	      ptrFsmInterface->setProcess(FsmState::Initializing);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();

	      initializing();
      
	      // Set to Halted static state
	      ptrFsmInterface->setProcess(FsmState::Halted);//,FsmState::Good);
	      if(_printEnable) ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(ptrFsmInterface->commandPacket().command())==ptrFsmInterface->processState());
	    }
	    break;
	  }

      
	    // Transitional states: should never happen   /////////////////////


	  case FsmState::Initializing: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to Halted static state
	    ptrFsmInterface->setProcess(FsmState::Halted,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }
    
	  case FsmState::ConfiguringA: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to ConfiguredA static state
	    ptrFsmInterface->setProcess(FsmState::ConfiguredA,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }
    
	  case FsmState::HaltingB: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to Halted static state
	    ptrFsmInterface->setProcess(FsmState::Halted,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::ConfiguringB: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to ConfiguredB static state
	    ptrFsmInterface->setProcess(FsmState::ConfiguredB,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::HaltingA: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to static state
	    ptrFsmInterface->setProcess(FsmState::ConfiguredA,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::Starting: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to Running static state
	    ptrFsmInterface->setProcess(FsmState::Running,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::Stopping: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to ConfiguredB static state
	    ptrFsmInterface->setProcess(FsmState::ConfiguredB,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::Pausing: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to Paused static state
	    ptrFsmInterface->setProcess(FsmState::Paused,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::Resuming: {
	    if(_printEnable) ptrFsmInterface->print();
    
	    // Set to Running static state
	    ptrFsmInterface->setProcess(FsmState::Running,FsmState::Warning);
	    if(_printEnable) ptrFsmInterface->print();
	    break;
	  }

	  default: {
	    if(_printEnable) ptrFsmInterface->print();
	    assert(false);
	    break;
	  }
	  };

	  //////////////////////////////////////////////////////////////////
	  
	} else if(ptrFsmInterface->matchingStates()) {
	  std::cout << "HERE2 ";ptrFsmInterface->print();
	  sleep(1);

	  // In static state

	  switch(ptrFsmInterface->processState()) {

	  case FsmState::Initial: {
	    initial();
	    break;
	  }
	    
	  case FsmState::Halted: {
	    halted();
	    break;
	  }

	  case FsmState::ConfiguredA: {
	    configuredA();
	    break;
	  }

	  case FsmState::ConfiguredB: {
	    configuredB();
	    break;
	  }

	  case FsmState::Running: {
	    
	    running(ptrFsmInterface);
	    break;
	  }

	  case FsmState::Paused: {
	    paused();
	    break;
	  }
	    
	  default: {
	    if(_printEnable) ptrFsmInterface->print();
	    assert(false);
	    break;
	  }
	  }

	  //while(!ptrFsmInterface->matchingStates()) {
	  while(!ptrFsmInterface->rcLock()) {
	    usleep(_usSleep[ptrFsmInterface->processState()]);
	  }
	} else {
	  std::cout << "HERE3 ";ptrFsmInterface->print();
	  sleep(1);
	}

      }
#endif

    }
   
  protected:
    bool _printEnable;
    FsmInterface *ptrFsmInterface;
    unsigned _usSleep[FsmState::EndOfStaticEnum];
  };

}

#endif
