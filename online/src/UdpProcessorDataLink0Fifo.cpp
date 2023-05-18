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

#include "SystemParameters.h"
#include "ShmSingleton.h"
#include "DataFifo.h"
#include "Record.h"

using namespace Hgcal10gLinkReceiver;

#define PORT	1234
#define MAXLINE 4096

/*
void setMemoryBuffer(RunFileShm *p) {
  for(unsigned i(0);i<RunFileShm::BufferDepth;i++) {
    p->_buffer[i][0]=0xacb0beefcafedead+(i%16)<<48;
    for(unsigned j(0);j<RunFileShm::BufferWidth;j++) {
      p->_buffer[i][j]=i*RunFileShm::BufferWidth+j;
    }
  }
}
*/

int main(int argc, char *argv[]) {
  ShmSingleton< DataFifoT<6,1024> > shmU;
  DataFifoT<6,1024> *ptrRunFileShm=shmU.setup(ProcessorDaqLink0FifoShmKey);

      //ShmSingleton<RunFileShm> shmU;
      //RunFileShm* const ptrRunFileShm(shmU.payload());

  // Define control flags
  bool dummyReader(false);
  bool dummyWriter(false);

  bool printEnable(false);
  bool checkEnable(false);
  bool assertEnable(false);

   for(int i(1);i<argc;i++) {
     if(std::string(argv[i])=="-d" ||
	std::string(argv[i])=="--dummyReader") dummyReader=true;
     if(std::string(argv[i])=="-w" ||
	std::string(argv[i])=="--dummyWriter") dummyWriter=true;
     
     if(std::string(argv[i])=="-p" ||
	std::string(argv[i])=="--printEnable") {
       std::cout << "Setting printEnable to true" << std::endl;
       printEnable=true;
     }
     
     if(std::string(argv[i])=="-c" ||
	std::string(argv[i])=="--checkEnable") {
       std::cout << "Setting checkEnable to true" << std::endl;
       checkEnable=true;
     }
     
     if(std::string(argv[i])=="-a" ||
	std::string(argv[i])=="--assertEnable") {
       std::cout << "Setting assertEnable to true" << std::endl;
       assertEnable=true;
     }     
   }
   
   
  //setMemoryBuffer(ptrRunFileShm);

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

  Record *r;

  bool continueLoop(true);

  while(continueLoop) {

    // Receive data from Serenity
    //socket.recv(request, zmq::recv_flags::none);

    
    while((r=(Record*)(ptrRunFileShm->getWriteRecord()))==nullptr) usleep(10);
    
    //uint64_t *ptr(ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm::BufferDepth]);
    n = recvfrom(sockfd, (char *)r, MAXLINE,
		 MSG_WAITALL, ( struct sockaddr *) &cliaddr,
		 &len);

    if(printEnable) std::cout  << std::endl << "************ GOT DATA ******************" << std::endl << std::endl;

    uint64_t *ptr((uint64_t*)r);

    bool valid((ptr[0]>>56)==0xac);
    if((ptr[0]>>56)!=0xac && ptr[0]!=0xdddddddddddddddd) {
      std::cerr << "BAD HEADER = 0x"
		<< std::hex << std::setfill('0')
		<< std::setw(16) << ptr[0]
		<< std::dec << std::setfill(' ')
		<< std::endl;
    } else {

      uint16_t *p16((uint16_t*)r);
      uint8_t *p8((uint8_t*)r);
      /*      
      std::cout << "p16 = 0x" << std::hex
		<< p16[0] << " " << p16[1] << " "
		<< p16[2] << " " << p16[3] << std::endl;
      std::cout << "p8 = 0x"
		<< unsigned(p8[0]) << " " << unsigned(p8[1]) << " "
		<< unsigned(p8[2]) << " " << unsigned(p8[3]) << " "
		<< unsigned(p8[4]) << " " << unsigned(p8[5]) << " "
		<< unsigned(p8[6]) << " " << unsigned(p8[7])
		<< std::dec << std::endl;
      */

      p16[2]=p8[6];
      p16[3]=0x3305;
      //r->print();
    }
    
    
    if(printEnable) std::cout << "Size = " << n << " ?= " << len << std::endl;
    
    //std::memcpy(,request.data(),request.size());
    if(printEnable) ptrRunFileShm->print();

    // Write into local PC FIFO
    //if((ptr[0]>>48)==0x3305) {
    if(valid) {
      ptrRunFileShm->writeIncrement();
      if(dummyWriter) ptrRunFileShm->readIncrement();
    }
    
    if(printEnable) ptrRunFileShm->print();

    //socket.send(zmq::buffer(&z,1), zmq::send_flags::none);

    //if(prcfs->isEnded()) continueLoop=false;
  }

  return 0;
}
