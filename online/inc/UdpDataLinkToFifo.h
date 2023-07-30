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

#define MAXLINE 4096

bool UdpDataLinkToFifo(uint32_t key, uint16_t port, bool w=false) {
  std::cout << "UdpDataLinkToFifo called with key = 0x"
            << std::hex << std::setfill('0')
            << std::setw(8) << key << ", port = 0x"
            << std::setw(4) << port
            << std::dec << std::setfill(' ')
            << std::endl;

  ShmSingleton< DataFifoT<6,1024> > shmU;
  DataFifoT<6,1024> *ptrRunFileShm=shmU.setup(key);

  // Define control flags
  bool dummyWriter(w);

  bool printEnable(false);
  bool checkEnable(false);
  bool assertEnable(false);
   
  //setMemoryBuffer(ptrRunFileShm);

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
  servaddr.sin_port = htons(port);
  
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

  RecordT<1023> *rt=new RecordT<1023>;
  uint64_t *ptrt((uint64_t*)rt);
  
  Record *r;

  bool continueLoop(true);

  //printEnable=true;

  uint64_t count[20];
  memset(count,0,20*8);
  
  while(continueLoop) {

    bool goodPacket(false);
    do {
      
      n = recvfrom(sockfd, (char *)rt, MAXLINE,
		   MSG_WAITALL, ( struct sockaddr *) &cliaddr,
		   &len);

      if(printEnable) std::cout << "First word = " << std::hex << ptrt[0] << std::dec << std::endl;

      if(ptrt[0]==0xdddddddddddddddd) {
	count[0]++;
	if(n!=24) {
	  count[1]++;
	  std::cerr << "BAD HEARTBEAT n = " << n << std::endl;
	  for(unsigned i(0);i<(unsigned(n)+7)/8;i++) {
	    std::cerr << " Word " << i << " = 0x"
		      << std::hex << std::setfill('0')
		      << std::setw(16) << ptrt[i]
		      << std::dec << std::setfill(' ')
		      << std::endl;
	  }
	}

      } else if((ptrt[0]>>56)!=0xac && (ptrt[0]>>56)!=0xaa) {
	count[2]++;
	std::cerr << "BAD PACKET n = " << n << std::endl;
	for(unsigned i(0);i<(unsigned(n)+7)/8;i++) {
	    std::cerr << " Word " << i << " = 0x"
		      << std::hex << std::setfill('0')
		      << std::setw(16) << ptrt[i]
		      << std::dec << std::setfill(' ')
		      << std::endl;
	}

      } else {
	if((8*((ptrt[0]>>48)&0xff)+8)!=unsigned(n)) {
	  count[3]++;
	  std::cerr << "BAD LENGTH n = " << n << std::endl;
	  for(unsigned i(0);i<(unsigned(n)+7)/8;i++) {
	    std::cerr << " Word " << i << " = 0x"
		      << std::hex << std::setfill('0')
		      << std::setw(16) << ptrt[i]
		      << std::dec << std::setfill(' ')
		      << std::endl;
	  }

	  // Special case of first packet in run; modify header
	  if(ptrt[0]==0xaaaaaaaaaaaaaaaa) {
	    rt->setState(FsmState::Running);
	    rt->setSequenceCounter(0);
	    rt->setPayloadLength((n-1)/8);
	    rt->RecordHeader::print(std::cerr);

	    goodPacket=true;
	  }
	  
	} else {
	  count[4]++;
	  goodPacket=true;
	}
      }
      
    } while(!goodPacket);
    
    if(printEnable) std::cout  << std::endl << "************ GOT DATA ******************" << std::endl << std::endl;

    if(printEnable) {
      std::cout << "Size = " << n << " ?= " << len << " ?= " << rt->totalLengthInBytes() << std::endl;
      rt->RecordHeader::print();
    }
    
    if(printEnable) ptrRunFileShm->print();

    while((r=(Record*)(ptrRunFileShm->getWriteRecord()))==nullptr) usleep(10);

    if(printEnable) std::cout  << std::endl << "************ GOT MEMORY ******************" << std::endl << std::endl;

    memcpy(r,rt,n);
	   
    //printEnable=(len<=16 || n<=16);

    if(printEnable) ptrRunFileShm->print();
    
    

    uint64_t *ptr((uint64_t*)r);

    bool valid((ptr[0]>>56)==0xac || (ptr[0]>>56)==0xaa);

    if((ptr[0]>>56)!=0xac && (ptr[0]>>56)!=0xaa && ptr[0]!=0xdddddddddddddddd) {
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

  std::cout << std::endl << "Packet counts" << std::endl;
  for(unsigned i(0);i<10;i++) {
    std::cout << " Count " << std::setw(2) << " = " << count[i] << std::endl;
  }
    
  delete rt;
  
  return true;
}
