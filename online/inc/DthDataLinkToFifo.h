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


#include "DthHeader.h"
#include "SlinkBoe.h"
#include "SlinkEoe.h"


#include "SystemParameters.h"
#include "ShmSingleton.h"
#include "DataFifo.h"
#include "Record.h"

using namespace Hgcal10gLinkReceiver;

#define MAXLINE 4096

#define PORT 10000



#define BUFFER_SIZE             (1024*1024)             /* 1 MB */
int BUFFER_SIZE_MAX=BUFFER_SIZE;                                    /* What buffer size to use? Up to 1MB */

bool DthDataLinkToFifo(uint32_t key, uint16_t port, bool w=false) {
  std::cout << "DthDataLinkToFifo called with key = 0x"
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

  int sock, connected, n;
    int istrue = 1;          
  struct sockaddr_in server_addr, client_addr;    
  unsigned int sin_size;

  int i, closed, used;
        
  int client_count = 0;
  //pthread_t thread;       
  //cpu_set_t cpuset;
  int cpu;

  char client_description[100];

  uint64_t *buffer=new uint64_t[BUFFER_SIZE/8];

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

        
  /* set SO_REUSEADDR on a socket to bypass TIME_WAIT state */

  if(false) {
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &istrue, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }
  }

  //if (options.net_if) {
    /* Bind socket to a device */
  /*
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, options.net_if, strlen(options.net_if)) == -1) {
      perror("setsockopt");
      exit(1);
    }
  }
  */    
        
  server_addr.sin_family = AF_INET;         
  server_addr.sin_port = htons(PORT);     
  server_addr.sin_addr.s_addr = INADDR_ANY; 
  bzero(&(server_addr.sin_zero), 8); 

  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
    perror("Unable to bind");
    exit(1);
  }

  if (listen(sock, 1) == -1) {
    perror("Listen");
    exit(1);
  }
                        
  printf("\nListening on port %u\n", PORT);
  /*
  memset(&clients, 0, sizeof(clients));
  if (options.cpu != -1) {
    cpu = options.cpu;
  } else {
    cpu = 1;
  }
  */

  sin_size = sizeof(struct sockaddr_in);
  connected = accept(sock, (struct sockaddr *)&client_addr,&sin_size);

  printf("\nConnected...\n");


  RecordT<8*1024-1> *rt=new RecordT<8*1024-1>;
  uint64_t *ptrt((uint64_t*)rt);
  
  Record *r;

  bool continueLoop(true);

  printEnable=true;
  

  std::ofstream fout;
  fout.open("temp.bin",std::ios::binary);







  Hgcal10gLinkReceiver::DthHeader *dthh(0);


  bool newEvent(true);
  uint32_t seqc(0);
  unsigned nWords=0;


  while(continueLoop) {

    // Receive data from Serenity
    //socket.recv(request, zmq::recv_flags::none);

    
    //uint64_t *ptr(ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm::BufferDepth]);
    n = recv(connected, buffer, BUFFER_SIZE_MAX, 0);

    unsigned n64(n+7/8);
    if((n%8)!=0) std::cerr << "WARNING: number of bytes read = " << n << " gives n%8 = " << (n%8)
			   << " != 0" << std::endl;

    //n = recvfrom(sockfd, (char *)rt, MAXLINE,
    //	   MSG_WAITALL, ( struct sockaddr *) &cliaddr,
    //	   &len);
    
    if(printEnable) std::cout  << std::endl << "************ GOT DATA ******************" << std::endl << std::endl;

    if(printEnable) {
      std::cout << "Size = " << n << " bytes = " << n64 << " uint64_t words" << std::endl;
      std::cout << "First word        = " << std::hex << std::setw(16) << buffer[0] << std::dec << std::endl;
      std::cout << "Second word       = " << std::hex << std::setw(16) << buffer[1] << std::dec << std::endl;
      std::cout << "Last-but-one word = " << std::hex << std::setw(16) << buffer[(n-9)/8] << std::dec << std::endl;
      std::cout << "Last word         = " << std::hex << std::setw(16) << buffer[(n-1)/8] << std::dec << std::endl;
    }

    dthh=(Hgcal10gLinkReceiver::DthHeader*)buffer;
    if(printEnable) {
      dthh->print();
    }

    //if(newEvent) {

    if(dthh->blockStart()) {
	rt->reset(FsmState::Running);
	rt->setSequenceCounter(seqc++);
	rt->RecordHeader::print();
	//rt->setPayloadLength(nWords);
	nWords=0;
    }

    std::memcpy(ptrt+1+nWords,buffer+2,n64-2);
    nWords+=n64-2;

    if(dthh->blockStop()) {
      rt->setPayloadLength(nWords);
      rt->print();

      fout.write((char*)(rt),rt->totalLengthInBytes());
      fout.flush();
    }
      
    if(printEnable) ptrRunFileShm->print();

#ifdef JUNK
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

#endif
  }

  delete rt;
  
  return true;
}



/*


  std::ofstream fout;
  fout.open("temp.bin",std::ios::binary);

  while(true) {
  int32_t rc = recv(connected, buffer, BUFFER_SIZE_MAX, 0);
  int32_t z(0);
  std::cout << "Recv returns rc = " << rc << std::endl;
  if(rc>0) {
    fout.write((char*)(&rc),4);
    fout.write((char*)(&z),4);
    fout.write((char*)buffer,rc);
    fout.flush();

    if(dthh->blockStart()) {
      Hgcal10gLinkReceiver::SlinkBoe *boe((Hgcal10gLinkReceiver::SlinkBoe*)(buffer+2));
      boe->print();
    }
    //if(dthh->blockStop()) {
      Hgcal10gLinkReceiver::SlinkEoe *eoe((Hgcal10gLinkReceiver::SlinkEoe*)(buffer+2*dthh->length()));
      eoe->print();
      //}

    for(unsigned i(0);i<(rc+7)/8;i++) {
      std::cout << " Word " << std::setw(6) << i << " = 0x"
		<< std::hex << std::setfill('0')
		<< std::setw(16) << buffer[i]
		<< std::dec << std::setfill(' ') << std::endl;
    }
  } else {
    perror("recv");
  }
  }
  return 0;
}
*/
