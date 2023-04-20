#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <iostream>
#include <cassert>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

template<> const key_t ShmSingleton<RunControlFsmShm>::theKey(0xcebe00);

int main(int argc, char *argv[]) {

  // Turn off printing
  bool printEnable(false);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-p" ||
       std::string(argv[i])=="--printEnable")
      std::cout << "Setting printEnable to true" << std::endl;
    printEnable=true;
  }

  key_t theKey;//(ShmSingleton<RunControlFsmShm>::theKey);
  if(argc>1) {
    std::istringstream sin(argv[argc-1]);
    std::string sKey(sin.str());
    if(sKey[0]=='0' && sKey[1]=='x') {
    } else {
      sin >> theKey;
    }
  }
  
  // Connect to shared memory
  ShmSingleton<RunControlFsmShm> shmU;
  shmU.setup(theKey);
  RunControlFsmShm* const ptrRunFileShm(shmU.payload());

  // Force to Initial on startup
  ptrRunFileShm->forceState(RunControlFsmEnums::Initial);
  ptrRunFileShm->forceErrorState(RunControlFsmEnums::Good);
  if(printEnable) ptrRunFileShm->print();

  //sleep(1);
  ptrRunFileShm->setCommand(RunControlFsmEnums::Initialize);
  
  while(true) {
    if(!ptrRunFileShm->rcLock()) {
      
      switch(ptrRunFileShm->state()) {

	// Static states   /////////////////////

	// INITIAL //
      
      case RunControlFsmEnums::Initial: {

	if(ptrRunFileShm->command()==RunControlFsmEnums::Initialize) {
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->state());
	
	  // Initializing; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::Initializing,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	
	  sleep(1);
      
	  // Set to Halted static state
	  ptrRunFileShm->setState(RunControlFsmEnums::Halted,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	} else {
	  assert(false);
	}
	break;
      }

	// HALTED //
      
      case RunControlFsmEnums::Halted: {

	if(ptrRunFileShm->command()==RunControlFsmEnums::ConfigureA) {
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	  // ConfiguringA; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::ConfiguringA,RunControlFsmEnums::Good);

	  sleep(1);
      
	  // Set to ConfiguredA static state
	  ptrRunFileShm->setState(RunControlFsmEnums::ConfiguredA,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());


	} else if(ptrRunFileShm->command()==RunControlFsmEnums::Reset) {

	  sleep(1);
      
	  // Set to Initial static state
	  ptrRunFileShm->setState(RunControlFsmEnums::Initial,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	  // Shutting down, so set restart value
	  //ptrRunFileShm->setState(RunControlFsmEnums::Initial,RunControlFsmEnums::Good);
	  //return 0;

	} else {
	  assert(false);
	}
	break;
      }

	// CONFIGUREDA //

      case RunControlFsmEnums::ConfiguredA: {

	if(ptrRunFileShm->command()==RunControlFsmEnums::ConfigureB) {
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->state());
      
	  // ConfiguringB; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::ConfiguringB,RunControlFsmEnums::Good);

	  sleep(1);
      
	  // Set to ConfiguredB static state
	  ptrRunFileShm->setState(RunControlFsmEnums::ConfiguredB,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	
	} else if(ptrRunFileShm->command()==RunControlFsmEnums::HaltA) {
      
	  // HaltingB; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::HaltingB,RunControlFsmEnums::Good);

	  sleep(1);
            
	  // Set to Halted static state
	  ptrRunFileShm->setState(RunControlFsmEnums::Halted,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	} else {
	  assert(false);
	}
	break;
      }

	// CONFIGUREDB
      
      case RunControlFsmEnums::ConfiguredB: {

	if(ptrRunFileShm->command()==RunControlFsmEnums::Start) {
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->state());
      
	  // Starting; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::Starting,RunControlFsmEnums::Good);

	  sleep(1);
      
	  // Set to Running static state
	  ptrRunFileShm->setState(RunControlFsmEnums::Running,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	
	} else if(ptrRunFileShm->command()==RunControlFsmEnums::HaltB) {
      
	  // HaltingB; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::HaltingB,RunControlFsmEnums::Good);

	  sleep(1);
      
	  // Set to ConfiguredA static state
	  ptrRunFileShm->setState(RunControlFsmEnums::ConfiguredA,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	} else {
	  assert(false);
	}
	break;
      }

	// RUNNING //
      
      case RunControlFsmEnums::Running: {

	if(ptrRunFileShm->command()==RunControlFsmEnums::Pause) {
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	  // Pausing; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::Pausing,RunControlFsmEnums::Good);

	  sleep(1);

	  // Set to Paused static state
	  ptrRunFileShm->setState(RunControlFsmEnums::Paused,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	
	} else if(ptrRunFileShm->command()==RunControlFsmEnums::Stop) {
      
	  // Stopping; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::Stopping,RunControlFsmEnums::Good);

	  sleep(1);
      
	  // Set to ConfiguredB static state
	  ptrRunFileShm->setState(RunControlFsmEnums::ConfiguredB,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	} else {
	  assert(false);
	}
	break;
      }

	// PAUSED //
      
      case RunControlFsmEnums::Paused: {

	if(ptrRunFileShm->command()==RunControlFsmEnums::Resume) {
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateBeforeCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	  // Starting; set to transitional state
	  ptrRunFileShm->setState(RunControlFsmEnums::Starting,RunControlFsmEnums::Good);

	  sleep(1);
	
	  // Set to static state
	  ptrRunFileShm->setState(RunControlFsmEnums::Running,RunControlFsmEnums::Good);
	  if(printEnable) ptrRunFileShm->print();
	  assert(RunControlFsmEnums::staticStateAfterCommand(ptrRunFileShm->command())==ptrRunFileShm->state());

	} else {
	  assert(false);
	}
	break;
      }

      
	// Transitional states: should never happen   /////////////////////


      case RunControlFsmEnums::Initializing: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to Halted static state
	ptrRunFileShm->setState(RunControlFsmEnums::Halted,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }
    
      case RunControlFsmEnums::ConfiguringA: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to ConfiguredA static state
	ptrRunFileShm->setState(RunControlFsmEnums::ConfiguredA,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }
    
      case RunControlFsmEnums::HaltingB: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to Halted static state
	ptrRunFileShm->setState(RunControlFsmEnums::Halted,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }

      case RunControlFsmEnums::ConfiguringB: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to ConfiguredB static state
	ptrRunFileShm->setState(RunControlFsmEnums::ConfiguredB,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }

      case RunControlFsmEnums::HaltingA: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to static state
	ptrRunFileShm->setState(RunControlFsmEnums::ConfiguredA,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }

      case RunControlFsmEnums::Starting: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to Running static state
	ptrRunFileShm->setState(RunControlFsmEnums::Running,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }

      case RunControlFsmEnums::Stopping: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to ConfiguredB static state
	ptrRunFileShm->setState(RunControlFsmEnums::ConfiguredB,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }

      case RunControlFsmEnums::Pausing: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to Paused static state
	ptrRunFileShm->setState(RunControlFsmEnums::Paused,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }

      case RunControlFsmEnums::Resuming: {
	if(printEnable) ptrRunFileShm->print();
    
	// Set to Running static state
	ptrRunFileShm->setState(RunControlFsmEnums::Running,RunControlFsmEnums::Warning);
	if(printEnable) ptrRunFileShm->print();
	break;
      }

      default: {
	if(printEnable) ptrRunFileShm->print();
	assert(false);
	break;
      }
      };
    }
  }
  return 0;
}
