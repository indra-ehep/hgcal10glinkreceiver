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

//#define PORT 10000



#define BUFFER_SIZE             (1024*1024)             /* 1 MB */
int BUFFER_SIZE_MAX=BUFFER_SIZE-1024;                                    /* What buffer size to use? Up to 1MB */

bool DthDataLinkToFifo(uint32_t key, uint16_t port, bool w=false, bool p=false) {
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

  bool printEnable(p);
  bool checkEnable(false);
  bool assertEnable(false);
   
  //setMemoryBuffer(ptrRunFileShm);

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
  server_addr.sin_port = htons(port);     
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
                        
  printf("\nListening on port %u\n", port);
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

  //std::ofstream fout;
  //if(port==10000) fout.open("temp0.bin",std::ios::binary);
  //if(port==10010) fout.open("temp1.bin",std::ios::binary);


  unsigned n64Tcp,i64Tcp,n64Packet(0),i64Packet(0);




  Hgcal10gLinkReceiver::DthHeader dthh;


  bool newEvent(true);
  uint32_t seqc(0);
  unsigned nWords=0;


  while(continueLoop) {

    // Receive data from Serenity
    //socket.recv(request, zmq::recv_flags::none);

    n=0;
    while(n<8) { // Catch no packet
    //uint64_t *ptr(ptrRunFileShm->_buffer[ptrRunFileShm->_writePtr%RunFileShm::BufferDepth]);
    n = recv(connected, buffer, BUFFER_SIZE_MAX, 0);

    n64Tcp=(n+7)/8;
    i64Tcp=0;

    if(n<8 || (n%8)!=0) std::cerr << "WARNING: number of bytes read = " << n << " gives n%8 = " << (n%8)
			   << " != 0" << std::endl;
    if(n>=BUFFER_SIZE) std::cerr << "WARNING: number of bytes read = " << n << " >= "
				      << BUFFER_SIZE << std::endl;

    //n = recvfrom(sockfd, (char *)rt, MAXLINE,
    //	   MSG_WAITALL, ( struct sockaddr *) &cliaddr,
    //	   &len);
    
    if(printEnable) std::cout  << std::endl << "************ GOT DATA ******************" << std::endl << std::endl;

    if(printEnable) {
      std::cout << "TCP size = " << n << " bytes = " << n64Tcp << " uint64_t words" << std::endl;
      if(n>= 8) std::cout << "First word        = " << std::hex << std::setw(16) << buffer[0] << std::dec << std::endl;
      if(n>=16) std::cout << "Second word       = " << std::hex << std::setw(16) << buffer[1] << std::dec << std::endl;
      if(n>=24) std::cout << "Last-but-one word = " << std::hex << std::setw(16) << buffer[n64Tcp-2] << std::dec << std::endl;
      if(n>=32) std::cout << "Last word         = " << std::hex << std::setw(16) << buffer[n64Tcp-1] << std::dec << std::endl;
      
      //std::cout << "Last-but-one word = " << std::hex << std::setw(16) << buffer[(n-9)/8] << std::dec << std::endl;
      //std::cout << "Last word         = " << std::hex << std::setw(16) << buffer[(n-1)/8] << std::dec << std::endl;
    }
    }
    
    while(i64Tcp<n64Tcp) {

    if(n64Packet==0) {
      dthh=*((Hgcal10gLinkReceiver::DthHeader*)(buffer+i64Tcp));
      i64Tcp+=2;

    if(dthh.blockStart()) {
      rt->reset(FsmState::Running);
      rt->setSequenceCounter(seqc++);
      if(printEnable) rt->RecordHeader::print();
      //rt->setPayloadLength(nWords);
      nWords=0;
    }

      n64Packet=2*dthh.length();
      i64Packet=0;
	
      if(printEnable) {
	dthh.print();
      }
    }
        
    unsigned words64=std::min(n64Packet-i64Packet,n64Tcp-i64Tcp); 
    //unsigned words64=n64Packet;
    if(printEnable) {
      std::cout << "Packet words = min(" << n64Packet-i64Packet << ", " << n64Tcp-i64Tcp
		<< ") = " << words64 << " uint64_t words" << std::endl;
    } 
  
    std::memcpy(ptrt+1+nWords,buffer+i64Tcp,8*words64);
    nWords+=words64;
    i64Tcp+=words64;
    i64Packet+=words64;

    bool cludgeStop(false);
    /*
    if((buffer[n64Tcp-1]>>56)==0xaa && !dthh.blockStop()) {
      std::cerr << "WARNING: No BlockStop but last word = " << std::hex << std::setw(16) << buffer[n64-1]
		<< std::dec << " and length = " << nWords << std::endl;
      ((const Hgcal10gLinkReceiver::SlinkEoe*)(buffer+n64-2))->print(std::cerr);
      cludgeStop=true;
    }
    */
    if(i64Packet>=n64Packet && (dthh.blockStop() || cludgeStop)) {
      if(nWords>=8*1024) std::cerr << "WARNING: number of packet words = " << nWords << " >= "
				      << 8*1024 << std::endl;

      rt->setPayloadLength(nWords);
      if(printEnable) rt->print();

      //fout.write((char*)(rt),rt->totalLengthInBytes());
      //fout.flush();

      n64Packet=0;
      
      //if(printEnable) ptrRunFileShm->print();
      
      while((r=(Record*)(ptrRunFileShm->getWriteRecord()))==nullptr) usleep(10);
      
      if(printEnable) std::cout  << std::endl << "************ GOT MEMORY ******************" << std::endl << std::endl;
      
      std::memcpy(r,rt,rt->totalLengthInBytes());
	   
      ptrRunFileShm->writeIncrement();
      if(dummyWriter) ptrRunFileShm->readIncrement();
      if(printEnable) ptrRunFileShm->print();
    }
    }
#ifdef JUNK
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
