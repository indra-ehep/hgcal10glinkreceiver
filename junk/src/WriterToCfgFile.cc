#include <bits/stdc++.h>
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

#include "RunFileShm.h"
#include "ShmSingleton.hh"

#include "formats/src/RunFileName.cc"

#define MAXFILEBYTES 2000000000

int main(int argc, char *argv[]) {

  // Turn off file writing
  bool writeEnable(true);  

  for(int i(1);i<argc-1;i++) {
    if(std::string(argv[i])=="-w" ||
       std::string(argv[i])=="--writeDisable")
      writeEnable=false;
  }

  // Connect to shared memory
  ShmSingleton<RunFileShm> shmU;
  RunFileShm* const ptrRunFileShm(shmU.payload());
  //ptrRunFileShm->ColdStart();

  // Access to data buffer of requests
  uint64_t* const ptrFsmRequestDataBuffer(ptrRunFileShm->_runControlFsmShm.fsmRequestDataBuffer());
  
  // Variables for files
  std::ofstream fout;
  std::string openFileName;

  uint64_t cptr[2];

  while(true) {

    switch(ptrRunFileShm->_runControlFsmShm.fsmState()) {

      // Static states   /////////////////////

      // INITIAL //
      
    case RunControlFsmShm::Initial: {

      if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Initialize) {
      
	// PreConfiguring; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Initializing,RunControlFsmShm::Good);
      
	// Set to Halted static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halted,RunControlFsmShm::Good);
      }
      break;
    }

      // HALTED //
      
    case RunControlFsmShm::Halted: {

      if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::PreConfigure) {

	// PreConfiguring; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfiguring,RunControlFsmShm::Good);

	if(ptrRunFileShm->_runControlFsmShm.fsmRequestDataSize()>0) {
	  ptrRunFileShm->_runNumber=ptrFsmRequestDataBuffer[0]&0xffffffff;
	} else {
	  ptrRunFileShm->_runNumber=0;
	}
	
	openFileName=CfgFileName(ptrRunFileShm->_runNumber,LINKNUMBER,ptrRunFileShm->_fileNumber);
      
	if(writeEnable) {
	  fout.open(openFileName.c_str(),std::ios::binary);
	  if(!fout) ptrRunFileShm->_runControlFsmShm.setErrorState(RunControlFsmShm::Error);
	}
      
	cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::PreConfiguring)<<56;
	cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);

	if(writeEnable) {
	  fout.write((char*)cptr,16);
	  if(!fout) ptrRunFileShm->_runControlFsmShm.setErrorState(RunControlFsmShm::Error);
	}
      
	ptrRunFileShm->_bytesInRun+=16;
	ptrRunFileShm->_bytesInFile+=16;
      
	// Set to PreConfigured static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfigured,RunControlFsmShm::Good);


      } else if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Shutdown) {

	// Shutting down, so set restart value
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Initial,RunControlFsmShm::Good);
	return 0;
      }
      break;
    }

      // PRECONFIGURED //

    case RunControlFsmShm::PreConfigured: {
      if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Configure) {
      
	// Configuring; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configuring,RunControlFsmShm::Good);
      
	cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Configuring)<<56;
	cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
	if(writeEnable) {
	  fout.write((char*)cptr,16);
	  if(!fout) ptrRunFileShm->_runControlFsmShm.setErrorState(RunControlFsmShm::Error);
	}
      
	ptrRunFileShm->_bytesInRun+=16;
	ptrRunFileShm->_bytesInFile+=16;
      
	// Set to Configured static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configured,RunControlFsmShm::Good);

	
      } else if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Halt) {
      
	// Halting; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halting,RunControlFsmShm::Good);
      
	cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Halting)<<56;
	cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
	if(writeEnable) {
	  fout.write((char*)cptr,16);
	  if(!fout) ptrRunFileShm->_runControlFsmShm.setErrorState(RunControlFsmShm::Error);
	}
      
	ptrRunFileShm->_bytesInRun+=16;
	ptrRunFileShm->_bytesInFile+=16;
      
	if(writeEnable) {
	  fout.close();
	  // Use this later but not while debugging
	  //if(system((std::string("chmod 444 ")+openFileName).c_str())!=0) ptrRunFileShm->_runControlFsmShm.setErrorState(RunControlFsmShm::Error);
	  openFileName="";
	}
            
	// Set to Halted static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halted,RunControlFsmShm::Good);
      }
      break;
    }

      // CONFIGURED
      
    case RunControlFsmShm::Configured: {
      if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Start) {
      
	// Starting; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Starting,RunControlFsmShm::Good);
      
	cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Starting)<<56;
	cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
	if(writeEnable) {
	  fout.write((char*)cptr,16);
	  if(!fout) ptrRunFileShm->_runControlFsmShm.setErrorState(RunControlFsmShm::Error);
	}
      
	ptrRunFileShm->_bytesInRun+=16;
	ptrRunFileShm->_bytesInFile+=16;
      
	// Set to Running static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Running,RunControlFsmShm::Good);

	
      } else if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Unconfigure) {
      
	// Unconfiguring; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Unconfiguring,RunControlFsmShm::Good);
      
	cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Unconfiguring)<<56;
	cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
	if(writeEnable) {
	  fout.write((char*)cptr,16);
	  if(!fout) ptrRunFileShm->_runControlFsmShm.setErrorState(RunControlFsmShm::Error);
	}
      
	ptrRunFileShm->_bytesInRun+=16;
	ptrRunFileShm->_bytesInFile+=16;
      
	// Set to PreConfigured static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfigured,RunControlFsmShm::Good);
      }
      break;
    }

      // RUNNING //
      
    case RunControlFsmShm::Running: {
      if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Pause) {

	// Pausing; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Pausing,RunControlFsmShm::Good);

	// Set to Paused static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Paused,RunControlFsmShm::Good);

	
      } else if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Stop) {
      
	// Stopping; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Stopping,RunControlFsmShm::Good);
      
	cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Stopping)<<56;
	cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
	if(writeEnable) {
	  fout.write((char*)cptr,16);
	  if(!fout) ptrRunFileShm->_runControlFsmShm.setErrorState(RunControlFsmShm::Error);
	}
      
	ptrRunFileShm->_bytesInRun+=16;
	ptrRunFileShm->_bytesInFile+=16;
      
	// Set to Configured static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configured,RunControlFsmShm::Good);
      }
      break;
    }

      // PAUSED //
      
    case RunControlFsmShm::Paused: {
      if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Resume) {

	// Starting; set to transitional state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Starting,RunControlFsmShm::Good);

	// Set to static state
	ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Running,RunControlFsmShm::Good);
      }
      break;
    }


      // Transitional states: should never happen   /////////////////////


    case RunControlFsmShm::Initializing: {
    
      // Set to Halted static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halted,RunControlFsmShm::Warning);
      break;
    }
    
    case RunControlFsmShm::PreConfiguring: {
    
      // Set to PreConfigured static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfigured,RunControlFsmShm::Warning);
      break;
    }
    
    case RunControlFsmShm::Halting: {
    
      // Set to Halted static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halted,RunControlFsmShm::Warning);
      break;
    }

    case RunControlFsmShm::Configuring: {
    
      // Set to Configured static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configured,RunControlFsmShm::Warning);
      break;
    }

    case RunControlFsmShm::Unconfiguring: {
    
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfigured,RunControlFsmShm::Warning);
      break;
    }

    case RunControlFsmShm::Starting: {
    
      // Set to Running static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Running,RunControlFsmShm::Warning);
      break;
    }

    case RunControlFsmShm::Stopping: {
    
      // Set to Configured static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configured,RunControlFsmShm::Warning);
      break;
    }

    case RunControlFsmShm::Pausing: {
    
      // Set to Paused static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Paused,RunControlFsmShm::Warning);
      break;
    }

    case RunControlFsmShm::Resuming: {
    
      // Set to Running static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Running,RunControlFsmShm::Warning);
      break;
    }

    default: {
      assert(false);
      break;
    }
    };

  }
  return 0;
}
