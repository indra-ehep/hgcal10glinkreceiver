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
#include <cstdlib>

#include "RunFileShm.h"
#include "ShmSingleton.hh"

/*
void setMemoryBuffer(RunFileShm *p) {
  for(unsigned i(0);i<RunFileShm::BufferDepth;i++) {
    p->_buffer[i][0]=0xac70beefcafedead|uint64_t(i%16)<<48;
    
    std::cout << "Buffer start " << std::setw(4) << i << " "
	      << std::hex << std::setfill('0')
	      << std::setw(16) << p->_buffer[i][0]
	      << std::dec << std::setfill(' ')
	      << std::endl;
    
    for(unsigned j(1);j<RunFileShm::BufferWidth;j++) {
      p->_buffer[i][j]=i*RunFileShm::BufferWidth+j;
    }
  }
}
*/

int main(int argc, char *argv[]) {
  ShmSingleton<RunFileShm0> shm0;
  RunFileShm0* const ptrRunFileShm0(shm0.payload());
  if(ptrRunFileShm0==0) return 1;
  
  ShmSingleton<RunFileShm1> shm1;
  RunFileShm1* const ptrRunFileShm1(shm1.payload());
  if(ptrRunFileShm1==0) return 1;
  
  // Define control flags
  bool dummyReader(false);
  bool readEnable(true);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-d" ||
       std::string(argv[i])=="--dummyReader") dummyReader=true;
  }
  
  //setMemoryBuffer(ptrRunFileShm0);
  //setMemoryBuffer(ptrRunFileShm1);
  /*
  if(dummyReader) {
    ptrRunFileShm->_writePtr=0xffffffff;
    return 0;
  }
  */
  uint64_t lh(0x0000cafebeefdead);
  
  bool doPrint(false);

  while(!ptrRunFileShm0->_requestShutDown || !ptrRunFileShm1->_requestShutDown) {

    // Active
    if(ptrRunFileShm0->_requestActive && ptrRunFileShm1->_requestActive) {

      // Check if no space to write
      if(ptrRunFileShm0->_writePtr==ptrRunFileShm0->_readPtr+RunFileShm0::BufferDepth ||
	 ptrRunFileShm1->_writePtr==ptrRunFileShm1->_readPtr+RunFileShm1::BufferDepth) {
	//usleep(1);

      } else {
	//ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm::BufferDepth][0]=0xac00000000000000;
	//ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr&RunFileShm::BufferDepthMask][0]=0xac00000000000000|(lh&0x0000ffffffffffff);
	ptrRunFileShm0->_buffer[ptrRunFileShm0->_writePtr%RunFileShm0::BufferDepth][0]=0xac00000000000000|uint64_t(rand()%256)<<48|(lh&0x0000ffffffffffff);
	ptrRunFileShm1->_buffer[ptrRunFileShm1->_writePtr%RunFileShm1::BufferDepth][0]=0xac00000000000000|uint64_t(rand()%128)<<48|(lh&0x0000ffffffffffff);
	//ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm::BufferDepth][0]=0xac00000000000000|(    lh%256)<<48|(lh&0x0000ffffffffffff);
	lh++;
	
	ptrRunFileShm0->_writePtr++;
	ptrRunFileShm1->_writePtr++;
      }

    } else {

      // Not active
      bool drained0(ptrRunFileShm0->_writePtr==ptrRunFileShm0->_readPtr);
      bool drained1(ptrRunFileShm1->_writePtr==ptrRunFileShm1->_readPtr);

      if(drained0 || drained1) {
	sleep(1);

	if(drained0 && ptrRunFileShm0->_writePtr==ptrRunFileShm0->_readPtr) {
	  ptrRunFileShm0->_writePtr=0;
	  ptrRunFileShm0->_readPtr=0;
	}

	if(drained1 && ptrRunFileShm1->_writePtr==ptrRunFileShm1->_readPtr) {
	  ptrRunFileShm1->_writePtr=0;
	  ptrRunFileShm1->_readPtr=0;
	}
	
      } else {
	sleep(1);
      }
    }
  }

  return 0;
}
