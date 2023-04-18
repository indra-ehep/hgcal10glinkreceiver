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

#include "ShmSingleton.hh"
#include "RunFileShm.h"

//#define PORT	1234
#define MAXLINE 4096

void setMemoryBuffer(RunFileShm *p) {
  for(unsigned i(0);i<RunFileShm::BufferDepth;i++) {
    p->_buffer[i][0]=0xacb0beefcafedead+(i%16)<<48;
    for(unsigned j(0);j<RunFileShm::BufferWidth;j++) {
      p->_buffer[i][j]=i*RunFileShm::BufferWidth+j;
    }
  }
}

int main(int argc, char *argv[]) {
  ShmSingleton<RunFileShm> shmU;
  RunFileShm* const ptrRunFileShm(shmU.payload());

  // Define control flags
  bool dummyReader(false);
  bool readEnable(true);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-d" ||
       std::string(argv[i])=="--dummyReader") dummyReader=true;
  }
  
  setMemoryBuffer(ptrRunFileShm);

  if(dummyReader) {
    ptrRunFileShm->_writePtr=0xffffffff;
    return 0;
  }
  
  bool doPrint(false);
  
  int sockfd;
  //char buffer[MAXLINE];
  //uint64_t buffer64[MAXLINE/8];
  //bool b64(true);
  //unsigned nWords;
  
  const char *hello = "Hello from server";
  struct sockaddr_in servaddr, cliaddr;

  /*
  // Creating socket file descriptor
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }
  */

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));
  
  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);
  
  socklen_t len;
  int n;
  
  len = sizeof(cliaddr); //len is value/result

  // Creating socket file descriptor
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Bind the socket with the server address
  if ( bind(sockfd, (const struct sockaddr *)&servaddr,
	    sizeof(servaddr)) < 0 )
    {
      perror("bind failed");
      exit(EXIT_FAILURE);
    }


  uint64_t previousPH(0);
  
  while(!ptrRunFileShm->_requestShutDown) {

    if(ptrRunFileShm->_requestActive &&
       !ptrRunFileShm->_runStateActive) {
      
      if(ptrRunFileShm->_writePtr==ptrRunFileShm->_readPtr) {
	ptrRunFileShm->_writePtr=0;
	ptrRunFileShm->_readPtr=0;
      }
    }
    
    if(ptrRunFileShm->_requestActive) {

      if(ptrRunFileShm->_writePtr<ptrRunFileShm->_readPtr+RunFileShm::BufferDepth) {
	
	if(ptrRunFileShm->_readEnable) {
	  uint64_t *ptr(ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm::BufferDepth]);
	  n = recvfrom(sockfd, (char *)ptr, MAXLINE,
		       MSG_WAITALL, ( struct sockaddr *) &cliaddr,
		       &len);

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
	}
	
	ptrRunFileShm->_writePtr++;
      }
      
    } else {

#ifdef REAL_CASE
#else
      if(ptrRunFileShm->_readEnable) {
	uint64_t *ptr(ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm::BufferDepth]);
	n = recvfrom(sockfd, (char *)ptr, MAXLINE,
		     MSG_WAITALL, ( struct sockaddr *) &cliaddr,
		     &len);
      }
#endif
      /*
      if(ptrRunFileShm->_writePtr==ptrRunFileShm->_readPtr) {
	sleep(1);
	if(ptrRunFileShm->_writePtr==ptrRunFileShm->_readPtr) {
	  ptrRunFileShm->_writePtr=0;
	  ptrRunFileShm->_readPtr=0;
	}
	
      } else {
	sleep(1);
      }
      */
    }
  }

  return 0;
}
