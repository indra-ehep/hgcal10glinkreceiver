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

#include "RunFileShm.h"
#include "ShmSingleton.hh"

#include "formats/src/RunFileName.cc"



//using namespace std;

#define MAXFILEBYTES 2000000000

// Driver code
int main() {
  ShmSingleton<RunFileShm> shmU;
  RunFileShm* const ptrRunFileShm(shmU.payload());
  ptrRunFileShm->ColdStart();
  
  std::ofstream fout;

  bool doPrint(true);

  std::string openFileName;
  bool writeEnable(true);

  uint64_t cptr[2];
  uint64_t previousPH(0);

  while(true) {

  switch(ptrRunFileShm->_runControlFsmShm.fsmState()) {

    // Static states   /////////////////////
    
  case RunControlFsmShm::Initial: {

    if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Initialize) {
      
      // PreConfiguring; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Initializing,RunControlFsmShm::Good);
      
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halted,RunControlFsmShm::Good);
    }
    break;
  }

  case RunControlFsmShm::Halted: {

    if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::PreConfigure) {
      std::cout << "HERE2" << std::endl;
      
      // PreConfiguring; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfiguring,RunControlFsmShm::Good);
      
      openFileName=CfgFileName(ptrRunFileShm->_runNumber,LINKNUMBER,ptrRunFileShm->_fileNumber);
      
      if(writeEnable) {
	fout.open(openFileName.c_str(),std::ios::binary);
	if(!fout) return 1;
      }
      
      cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::PreConfiguring)<<56;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }
      
      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      
      std::cout << "Starting super run " << (cptr[1]>>32)
		<< ", UTC " << (cptr[1]&0xffffffff) << std::endl;
      
      ptrRunFileShm->_runStateActive=true;
      
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfigured,RunControlFsmShm::Good);

    } else if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Shutdown) {

      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Initial,RunControlFsmShm::Good);
      return 0;
    }
    break;
  }

  case RunControlFsmShm::PreConfigured: {
    if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Configure) {
      
      // Configuring; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configuring,RunControlFsmShm::Good);
      
      cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Configuring)<<56;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }
      
      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configured,RunControlFsmShm::Good);

    } else if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Halt) {
      
      // Halting; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halting,RunControlFsmShm::Good);
      
      cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Halting)<<56;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }
      
      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      
      if(writeEnable) {
	fout.close();
	//if(!fout) return 1;
      }
            
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halted,RunControlFsmShm::Good);
    }
    break;
  }

  case RunControlFsmShm::Configured: {
    if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Start) {
      
      // Configuring; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Starting,RunControlFsmShm::Good);
      
      cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Starting)<<56;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }
      
      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Running,RunControlFsmShm::Good);

    } else if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Unconfigure) {
      
      // Halting; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Unconfiguring,RunControlFsmShm::Good);
      
      cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Unconfiguring)<<56;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }
      
      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfigured,RunControlFsmShm::Good);
    }
    break;
  }

  case RunControlFsmShm::Running: {
    if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Pause) {
      /*      
      // Configuring; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Pausing,RunControlFsmShm::Good);
      
      cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Pausing)<<56;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }
      
      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      */      
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Paused,RunControlFsmShm::Good);

    } else if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Stop) {
      
      // Halting; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Stopping,RunControlFsmShm::Good);
      
      cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Stopping)<<56;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }

      std::cout << "HERE?" << std::endl;
      
      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configured,RunControlFsmShm::Good);
    }
    break;
  }

  case RunControlFsmShm::Paused: {
    if(ptrRunFileShm->_runControlFsmShm.fsmRequest()==RunControlFsmShm::Resume) {
      /*      
      // Configuring; set to transitional state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Starting,RunControlFsmShm::Good);
      
      cptr[0]=0x0001000000000000|uint64_t(RunControlFsmShm::Starting)<<56;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }
      
      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      */
      // Set to static state
      ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Running,RunControlFsmShm::Good);
    }
    break;
  }


    // Transitional states   /////////////////////


  case RunControlFsmShm::Initializing: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halted,RunControlFsmShm::Good);
    break;
  }
    
  case RunControlFsmShm::PreConfiguring: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfigured,RunControlFsmShm::Good);
    break;
  }
    
  case RunControlFsmShm::Halting: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Halted,RunControlFsmShm::Good);
    break;
  }

  case RunControlFsmShm::Configuring: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configured,RunControlFsmShm::Good);
    break;
  }

  case RunControlFsmShm::Unconfiguring: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::PreConfigured,RunControlFsmShm::Good);
    break;
  }

  case RunControlFsmShm::Starting: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Running,RunControlFsmShm::Good);
    break;
  }

  case RunControlFsmShm::Stopping: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Configured,RunControlFsmShm::Good);
    break;
  }

  case RunControlFsmShm::Pausing: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Paused,RunControlFsmShm::Good);
    break;
  }

  case RunControlFsmShm::Resuming: {
    
    // Set to static state
    ptrRunFileShm->_runControlFsmShm.setFsmState(RunControlFsmShm::Running,RunControlFsmShm::Good);
    break;
  }
  };

  }
  return 0;
  
  while(!ptrRunFileShm->_requestShutDown) {

    // Idle
    if(!ptrRunFileShm->_requestActive &&
       !ptrRunFileShm->_runStateActive) {
      sleep(1);
    }
    
    // Begin of run
    if(ptrRunFileShm->_requestActive &&
       !ptrRunFileShm->_runStateActive) {

      writeEnable=ptrRunFileShm->_writeEnable;

      ptrRunFileShm->_packetNumberInRun=0;
      ptrRunFileShm->_bytesInRun=0;
      
      ptrRunFileShm->_fileNumber=0;
      ptrRunFileShm->_packetNumberInFile=0;
      ptrRunFileShm->_bytesInFile=0;
      /*      
      std::ostringstream sFileName;
      sFileName << "Run" << std::setfill('0')
		<< std::setw(10) << ptrRunFileShm->_runNumber << "_File"
		<< std::setw(10) << ptrRunFileShm->_fileNumber << ".bin";
      openFileName=sFileName.str();
      */
      openFileName=RunFileName(ptrRunFileShm->_runNumber,LINKNUMBER,ptrRunFileShm->_fileNumber);

      if(writeEnable) {
	fout.open(openFileName.c_str(),std::ios::binary);
	if(!fout) return 1;
      }
      
      cptr[0]=0xbb01000000000000;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }

      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      
      std::cout << "Starting  run " << (cptr[1]>>32)
		<< ", UTC " << (cptr[1]&0xffffffff) << std::endl;
      
      ptrRunFileShm->_runStateActive=true;
    }

    // Continue running
    if((ptrRunFileShm->_requestActive &&
	ptrRunFileShm->_runStateActive) ||
       ptrRunFileShm->_readPtr<ptrRunFileShm->_writePtr) {

      if(ptrRunFileShm->_readPtr<ptrRunFileShm->_writePtr) {
	//uint64_t *ptr(ptrRunFileShm->_buffer[ptrRunFileShm->_readPtr%RunFileShm::BufferDepth]);
	uint64_t *ptr(ptrRunFileShm->_buffer[ptrRunFileShm->_readPtr&RunFileShm::BufferDepthMask]);

	if((ptr[0]>>56)!=0xac) {
	  std::cerr << "BAD HEADER = 0x"
		    << std::hex << std::setfill('0')
		    << std::setw(16) << ptr[0]
		    << std::dec << std::setfill(' ')
		    << std::endl;
	}
	
	// Check for duplicates
	if(ptr[0]==previousPH) {
	  std::cerr << "DUPLICATED PACKET HEADER = 0x"
		    << std::hex << std::setfill('0')
		    << std::setw(16) << ptr[0]
		    << std::dec << std::setfill(' ')
		    << std::endl;
	}
	previousPH=ptr[0];

	unsigned nBytes(8*(((ptr[0]>>48)&0xff)+1));
	if(writeEnable) {
	  fout.write((char*)ptr,nBytes);
	  if(!fout) return 1;
	}
	ptrRunFileShm->_readPtr++;
	
	ptrRunFileShm->_packetNumberInRun++;
	ptrRunFileShm->_bytesInRun+=nBytes;
	ptrRunFileShm->_packetNumberInFile++;
	ptrRunFileShm->_bytesInFile+=nBytes;
	
	if(ptrRunFileShm->_bytesInFile>MAXFILEBYTES) {
	
	  cptr[0]=0xcc01000000000000+ptrRunFileShm->_fileNumber;
	  cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
	  if(writeEnable) {
	    fout.write((char*)cptr,16);
	    if(!fout) return 1;
	  }
	  
	  ptrRunFileShm->_bytesInRun+=16;
	  ptrRunFileShm->_bytesInFile+=16;
	  
	  std::cout << "Ending   file for run " << (cptr[1]>>32)
		    << ", UTC " << (cptr[1]&0xffffffff)
		    << ", events in run = " << ptrRunFileShm->_packetNumberInRun
		    << ", events in file = " << ptrRunFileShm->_packetNumberInFile
		    << ", file size = " << ptrRunFileShm->_bytesInFile << " bytes"
		    << std::endl;
	  
	  if(writeEnable) {
	    fout.close();
	    if(system((std::string("chmod 444 ")+openFileName).c_str())!=0) return 1;
	  }
	
	  // Set file counters
	  ptrRunFileShm->_fileNumber++;
	  ptrRunFileShm->_packetNumberInFile=0;
	  ptrRunFileShm->_bytesInFile=0;
	  /*	  
	  std::ostringstream sFileName;
	  sFileName << "Run" << std::setfill('0')
		    << std::setw(10) << ptrRunFileShm->_runNumber << "_File"
		    << std::setw(10) << ptrRunFileShm->_fileNumber << ".bin";
    
	  openFileName=sFileName.str();
	  */
	  openFileName=RunFileName(ptrRunFileShm->_runNumber,LINKNUMBER,ptrRunFileShm->_fileNumber);
	
	  if(writeEnable) {
	    fout.open(openFileName.c_str(),std::ios::binary);
	    if(!fout) return 1;
	  }
	
	  cptr[0]=0xdd01000000000000+ptrRunFileShm->_fileNumber;
	  cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
	  if(writeEnable) {
	    fout.write((char*)cptr,16);
	    if(!fout) return 1;
	  }
	  
	  ptrRunFileShm->_bytesInRun+=16;
	  ptrRunFileShm->_bytesInFile+=16;
	  
	  std::cout << "Starting file for run " << (cptr[1]>>32)
		    << ", UTC " << (cptr[1]&0xffffffff)
		    << ", events in run = " << ptrRunFileShm->_packetNumberInRun
		    << ", events in file = " << ptrRunFileShm->_packetNumberInFile
		    << ", file size = " << ptrRunFileShm->_bytesInFile << " bytes"
		    << std::endl;
	}

      } else {
	//usleep(10);
      }
    }

    // End of run
    if((!ptrRunFileShm->_requestActive &&
	ptrRunFileShm->_runStateActive) &&
       ptrRunFileShm->_readPtr==ptrRunFileShm->_writePtr) {
      
      cptr[0]=0xee01000000000000;
      cptr[1]=(uint64_t(ptrRunFileShm->_runNumber)<<32)|time(0);
      if(writeEnable) {
	fout.write((char*)cptr,16);
	if(!fout) return 1;
      }

      ptrRunFileShm->_bytesInRun+=16;
      ptrRunFileShm->_bytesInFile+=16;
      
      std::cout << "Ending   run " << (cptr[1]>>32)
		<< ", UTC " << (cptr[1]&0xffffffff)
		<< ", events in run = " << ptrRunFileShm->_packetNumberInRun
		<< ", run size = " << ptrRunFileShm->_bytesInRun << " bytes"
		<< std::endl;
      
      if(writeEnable) {
	fout.close();
	if(system((std::string("chmod 444 ")+openFileName).c_str())!=0) return 1;
      }
      
      ptrRunFileShm->_runStateActive=false;
    }
  }
  return 0;
}

