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
  bool writeEnable;

  uint64_t cptr[2];
  uint64_t previousPH(0);
      
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

