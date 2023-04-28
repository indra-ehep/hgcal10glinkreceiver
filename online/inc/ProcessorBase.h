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
      //RecordConfiguringA rca;
      //rca.deepCopy(_ptrFsmInterface->commandPacket().record());
      //rca.print();
      return true;
    }
    
    virtual bool configuringB(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool starting(FsmInterface::HandshakeState s) {
      //std::cout << "********************************************************** Starting a run" << std::endl;
      return true;
    }
    
    virtual bool pausing(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool resuming(FsmInterface::HandshakeState s) {
      return true;
    }
    
    virtual bool stopping(FsmInterface::HandshakeState s) {
      //std::cout << "********************************************************** Stopping a run" << std::endl;
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
    }
    
    virtual void running() {
      /*
      std::cout << "********************************************************** Doing a run" << std::endl;
      unsigned n(0);
      while(_ptrFsmInterface->isIdle()) {
	n++;
	usleep(_usSleep[_ptrFsmInterface->processState()]);      
      }
      std::cout << "********************************************************** Done  a run " << n << std::endl;
      */
    }
    
    virtual void paused() {
    }

    virtual void startFsm(uint32_t theKey) {
  
      // Connect to shared memory
      ShmSingleton<FsmInterface> shmU;
      shmU.setup(theKey);
      /*volatile*/ _ptrFsmInterface=shmU.payload();

      // Force to Initial on startup
      /*
      if(_printEnable) _ptrFsmInterface->print();
      _ptrFsmInterface->forceProcessState(FsmState::Initial);
      //_ptrFsmInterface->forceProcessError(FsmState::Good);
      _ptrFsmInterface->setCommandHandshake(FsmInterface::Idle);
      */
      _ptrFsmInterface->initialize();
      if(_printEnable) _ptrFsmInterface->print();

      std::cout << "TRY PONG" << std::endl;
      while(!_ptrFsmInterface->pong()) {
	sleep(1);
	if(_printEnable) _ptrFsmInterface->print();
	//usleep(10);
      }
      std::cout << "PONGED" << std::endl;
      if(_printEnable) _ptrFsmInterface->print();

      /*
	_ptrFsmInterface->setCommand(FsmState::Initialize);
	if(_printEnable) _ptrFsmInterface->print();
      */

      /*
	bool ponged(false);
	ponged=_ptrFsmInterface->pon
	if(ponged) std::cout << "PONGED1" << std::endl;
      
	while(_ptrFsmInterface->isIdle() && !ponged) {
	//if(_printEnable) _ptrFsmInterface->print();
	ponged=_ptrFsmInterface->pong();
	if(ponged) std::cout << "PONGED2" << std::endl;
	}
	//sleep(1);
      */

      //if(_ptrFsmInterface->isIdle()) {
      if(_printEnable) {
	std::cout << "Waiting for system match" << std::endl;
	_ptrFsmInterface->print();
      }

      //while(!_ptrFsmInterface->matchingStates());

      if(_printEnable) {
	std::cout << "Got system match" << std::endl;
	_ptrFsmInterface->print();
      }
      
      while(true) {
	//if(_ptrFsmInterface->isIdle()) {
	if(_printEnable) {
	  std::cout << "Start processing static state" << std::endl;
	  _ptrFsmInterface->print();
	}

	//assert(_ptrFsmInterface->matchingStates());

	if(     _ptrFsmInterface->processState()==FsmState::Initial    ) this->initial();
	else if(_ptrFsmInterface->processState()==FsmState::Halted     ) this->halted();
	else if(_ptrFsmInterface->processState()==FsmState::ConfiguredA) this->configuredA();
	else if(_ptrFsmInterface->processState()==FsmState::ConfiguredB) this->configuredB();
	else if(_ptrFsmInterface->processState()==FsmState::Running    ) this->running();
	else if(_ptrFsmInterface->processState()==FsmState::Paused     ) this->paused();
	else assert(false);
	  
	if(_printEnable) {
	  std::cout << "Done processing static state" << std::endl;
	  _ptrFsmInterface->print();
	}

	while(_ptrFsmInterface->isIdle()) usleep(10);

	if(_printEnable) {
	  std::cout << "Now a non-static handshake" << std::endl;
	  _ptrFsmInterface->print();
	}

	//} else {
	if(_ptrFsmInterface->pong()) {
	  if(_printEnable) {
	    std::cout << "Did pong" << std::endl;
	    _ptrFsmInterface->print();
	  }

	} else {
	  _ptrFsmInterface->setCommandHandshake(FsmInterface::Propose);
	  if(_printEnable) {
	    std::cout << "Entering new command" << std::endl;
	    _ptrFsmInterface->print();
	  }

	  assert(_ptrFsmInterface->matchingStates());

	  // PREPARING

	  assert(_ptrFsmInterface->matchingStates());

	  //while(!_ptrFsmInterface->prepared(true));
	  //if(_ptrFsmInterface->systemState()!=FsmState::ConfiguringB) {
	    assert(_ptrFsmInterface->accepted());
	    //} else {
	    //assert(_ptrFsmInterface->rejected());
	    //}

	  while(!_ptrFsmInterface->isProceed()) usleep(10);///////////////////////////////////////

	  bool change(_ptrFsmInterface->isChange());
			
	  if(_ptrFsmInterface->isChange()) {
	    //_ptrFsmInterface->forceProcessState(FsmCommand::transitionStateForCommand(_ptrFsmInterface->commandPacket().command()));
	    _ptrFsmInterface->changeProcessState();
	    if(_printEnable) {
	      std::cout << "Entered transient state" << std::endl;
	      _ptrFsmInterface->print();
	    }
	    
	    //assert(_ptrFsmInterface->matchingStates());

	    if(     _ptrFsmInterface->processState()==FsmState::Initializing) this->initializing(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::ConfiguringA) this->configuringA(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::ConfiguringB) this->configuringB(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::Starting    ) this->starting(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::Pausing     ) this->pausing(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::Resuming    ) this->resuming(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::Stopping    ) this->stopping(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::HaltingB    ) this->haltingB(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::HaltingA    ) this->haltingA(_ptrFsmInterface->commandHandshake());
	    else if(_ptrFsmInterface->processState()==FsmState::Resetting   ) this->resetting(_ptrFsmInterface->commandHandshake());
	    else assert(false);
	      
	    //assert(_ptrFsmInterface->matchingStates());
	      
	    if(_printEnable) {
	      std::cout << "Done transient state" << std::endl;
	      _ptrFsmInterface->print();
	    }
	      
	  } else if(_ptrFsmInterface->isRepair()) {
	    if(_printEnable) {
	      std::cout << "Doing repair" << std::endl;
	      _ptrFsmInterface->print();
	    }

	    assert(_ptrFsmInterface->matchingStates());

	    // REPAIRING
	      
	    assert(_ptrFsmInterface->matchingStates());	      
	  }
	    
	  assert(_ptrFsmInterface->completed());

	  while(!_ptrFsmInterface->isStartStatic()) usleep(10);///////////////////////////////////////
	    
	  if(change) _ptrFsmInterface->forceProcessState(FsmCommand::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command()));

	  assert(_ptrFsmInterface->matchingStates());

	  if(_printEnable) {
	    std::cout << "Entering static state" << std::endl;
	    _ptrFsmInterface->print();
	  }
	  
	  assert(_ptrFsmInterface->matchingStates());

	  assert(_ptrFsmInterface->ended());
	}
	//}
      }
      
      return;
#ifdef JUNK      
      while(true) {
	std::cout << "HERE0 ";_ptrFsmInterface->print();
	//sleep(1);
	if(!_ptrFsmInterface->rcLock()) {

	  // In transition
	  std::cout << "HERE1 ";_ptrFsmInterface->print();
	  //sleep(1);

	  switch(_ptrFsmInterface->processState()) {

	    // Static states   /////////////////////

	    // INITIAL //
      
	  case FsmState::Initial: {

	    if(_ptrFsmInterface->commandPacket().command()==FsmState::Initialize) {
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());
	
	      // Initializing; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::Initializing);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	
	      initializing();
      
	      // Set to Halted static state
	      _ptrFsmInterface->setProcess(FsmState::Halted);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	      halted();
	      
	    } else if(_ptrFsmInterface->commandPacket().command()==FsmState::Reset) {
	      if(_printEnable) _ptrFsmInterface->print();

	      // Resetting; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::Initializing);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	
	      resetting();
      
	      // Set to Initial static state
	      _ptrFsmInterface->setProcess(FsmState::Initial);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // HALTED //
      
	  case FsmState::Halted: {

	    if(_ptrFsmInterface->commandPacket().command()==FsmState::ConfigureA) {
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	      // ConfiguringA; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::ConfiguringA);//,FsmState::Good);

	      configuringA();
      
	      // Set to ConfiguredA static state
	      _ptrFsmInterface->setProcess(FsmState::ConfiguredA);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());


	    } else if(_ptrFsmInterface->commandPacket().command()==FsmState::Reset) {

	      // Resetting; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::Resetting);//,FsmState::Good);

	      resetting();
      
	      // Set to Initial static state
	      _ptrFsmInterface->setProcess(FsmState::Initial);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // CONFIGUREDA //

	  case FsmState::ConfiguredA: {

	    if(_ptrFsmInterface->commandPacket().command()==FsmCommand::ConfigureB) {
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());
      
	      // ConfiguringB; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::ConfiguringB);//,FsmState::Good);

	      //configuringB();
      
	      // Set to ConfiguredB static state
	      _ptrFsmInterface->setProcess(FsmState::ConfiguredB);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	
	    } else if(_ptrFsmInterface->commandPacket().command()==FsmCommand::HaltA) {
      
	      // HaltingB; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::HaltingB);//,FsmState::Good);

	      haltingB();
            
	      // Set to Halted static state
	      _ptrFsmInterface->setProcess(FsmState::Halted);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // CONFIGUREDB
      
	  case FsmState::ConfiguredB: {

	    if(_ptrFsmInterface->commandPacket().command()==FsmCommand::Start) {
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());
      
	      // Starting; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::Starting);//,FsmState::Good);

	      starting();
      
	      // Set to Running static state
	      _ptrFsmInterface->setProcess(FsmState::Running);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	
	    } else if(_ptrFsmInterface->commandPacket().command()==FsmCommand::HaltB) {
      
	      // HaltingB; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::HaltingB);//,FsmState::Good);

	      haltingB();
      
	      // Set to ConfiguredA static state
	      _ptrFsmInterface->setProcess(FsmState::ConfiguredA);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // RUNNING //
      
	  case FsmState::Running: {

	    if(_ptrFsmInterface->commandPacket().command()==FsmCommand::Pause) {
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	      // Pausing; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::Pausing);//,FsmState::Good);

	      pausing();

	      // Set to Paused static state
	      _ptrFsmInterface->setProcess(FsmState::Paused);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	
	    } else if(_ptrFsmInterface->commandPacket().command()==FsmCommand::Stop) {
      
	      // Stopping; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::Stopping);//,FsmState::Good);

	      stopping();
      
	      // Set to ConfiguredB static state
	      _ptrFsmInterface->setProcess(FsmState::ConfiguredB);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }

	    // PAUSED //
      
	  case FsmState::Paused: {

	    if(_ptrFsmInterface->commandPacket().command()==FsmCommand::Resume) {
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmCommand::staticStateBeforeCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	      // Starting; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::Starting);//,FsmState::Good);

	      starting();
	
	      // Set to static state
	      _ptrFsmInterface->setProcess(FsmState::Running);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());

	    } else {
	      assert(false);
	    }
	    break;
	  }
	    // NULL = Initial //
      
	  case FsmState::EndOfStateEnum: {

	    if(_ptrFsmInterface->commandPacket().command()==FsmCommand::Initialize) {
	      if(_printEnable) _ptrFsmInterface->print();

	      // Initializing; set to transitional state
	      _ptrFsmInterface->setProcess(FsmState::Initializing);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();

	      initializing();
      
	      // Set to Halted static state
	      _ptrFsmInterface->setProcess(FsmState::Halted);//,FsmState::Good);
	      if(_printEnable) _ptrFsmInterface->print();
	      assert(FsmState::staticStateAfterCommand(_ptrFsmInterface->commandPacket().command())==_ptrFsmInterface->processState());
	    }
	    break;
	  }

      
	    // Transitional states: should never happen   /////////////////////


	  case FsmState::Initializing: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to Halted static state
	    _ptrFsmInterface->setProcess(FsmState::Halted,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }
    
	  case FsmState::ConfiguringA: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to ConfiguredA static state
	    _ptrFsmInterface->setProcess(FsmState::ConfiguredA,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }
    
	  case FsmState::HaltingB: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to Halted static state
	    _ptrFsmInterface->setProcess(FsmState::Halted,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::ConfiguringB: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to ConfiguredB static state
	    _ptrFsmInterface->setProcess(FsmState::ConfiguredB,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::HaltingA: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to static state
	    _ptrFsmInterface->setProcess(FsmState::ConfiguredA,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::Starting: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to Running static state
	    _ptrFsmInterface->setProcess(FsmState::Running,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::Stopping: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to ConfiguredB static state
	    _ptrFsmInterface->setProcess(FsmState::ConfiguredB,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::Pausing: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to Paused static state
	    _ptrFsmInterface->setProcess(FsmState::Paused,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }

	  case FsmState::Resuming: {
	    if(_printEnable) _ptrFsmInterface->print();
    
	    // Set to Running static state
	    _ptrFsmInterface->setProcess(FsmState::Running,FsmState::Warning);
	    if(_printEnable) _ptrFsmInterface->print();
	    break;
	  }

	  default: {
	    if(_printEnable) _ptrFsmInterface->print();
	    assert(false);
	    break;
	  }
	  };

	  //////////////////////////////////////////////////////////////////
	  
	} else if(_ptrFsmInterface->matchingStates()) {
	  std::cout << "HERE2 ";_ptrFsmInterface->print();
	  //sleep(1);

	  // In static state

	  switch(_ptrFsmInterface->processState()) {

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
	    
	    running(_ptrFsmInterface);
	    break;
	  }

	  case FsmState::Paused: {
	    paused();
	    break;
	  }
	    
	  default: {
	    if(_printEnable) _ptrFsmInterface->print();
	    assert(false);
	    break;
	  }
	  }

	  //while(!_ptrFsmInterface->matchingStates()) {
	  while(!_ptrFsmInterface->rcLock()) {
	    usleep(_usSleep[_ptrFsmInterface->processState()]);
	  }
	} else {
	  std::cout << "HERE3 ";_ptrFsmInterface->print();
	  //sleep(1);
	}

      }
#endif

    }
   
  protected:
    bool _printEnable;
    FsmInterface *_ptrFsmInterface;
    unsigned _usSleep[FsmState::EndOfStaticEnum];
  };

}

#endif
