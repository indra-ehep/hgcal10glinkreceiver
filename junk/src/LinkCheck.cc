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

#include "LinkCheckShm.h"

//#define PORT	1234
#define MAXLINE 4096

int main(int argc, char *argv[]) {
  ShmSingleton<LinkCheckShm> shm;
  shm.setup(theKey);
  LinkCheckShm* const p(shm.payload());

  p->reset();

  bool fakeSocket(false);
  bool countEnable(false);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-f") fakeSocket=true;
    if(std::string(argv[i])=="-s") countEnable=true;
  }

  // Fake socket 
  const uint64_t nWords(245);
  
  int sockfd;
  uint64_t buffer64[MAXLINE];
  buffer64[0]=0xac00000000000000|nWords<<48;
  
  const char *hello = "Hello from server";
  struct sockaddr_in servaddr, cliaddr;

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));
  
  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);
  
  socklen_t len;
  uint64_t n;
  
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

  uint64_t predictedPH(0);
  uint64_t maskPH(0xff00ffffffffffff);

  unsigned ip(0);
  unsigned np;

  uint64_t minBytes(0xffffffff),maxBytes(0);
  
  while(true) {
    if(!fakeSocket) {
      n = recvfrom(sockfd, (char *)buffer64, MAXLINE,
		   MSG_WAITALL, ( struct sockaddr *) &cliaddr,
		   &len);
      //std::cout << "Received " << n << std::endl;
    } else {
      n=8*nWords;
      buffer64[0]++;
    }

    np=8*(((buffer64[0]>>48)&0xff)+1);
    
    if(countEnable) {
      p->_array[LinkCheckShm::NumberOfSocketPackets]++;
      p->_array[LinkCheckShm::BytesOfSocketPackets]+=n;
      if(minBytes>n) {
	minBytes=n;
	p->_array[LinkCheckShm::MinBytesOfSocketPackets]=n;
      }
      if(maxBytes<n) {
	maxBytes=n;
	p->_array[LinkCheckShm::MaxBytesOfSocketPackets]=n;
      }
    }
    
    if(n==0) {
      if(countEnable) p->_array[LinkCheckShm::NumberOfSocketSizeEq0k]++;

    } else if(n>MAXLINE) {
      if(countEnable) p->_array[LinkCheckShm::NumberOfSocketSizeGt4k]++;

    } else {
      if((n%8)!=0) {if(countEnable) p->_array[LinkCheckShm::NumberOfSocketSizeMod8]++;}
      if(n>np) {if(countEnable) p->_array[LinkCheckShm::NumberOfSocketSizeGtPacket]++;}
      if(n<np) {if(countEnable) p->_array[LinkCheckShm::NumberOfSocketSizeLtPacket]++;}
      if(n==np) {if(countEnable) p->_array[LinkCheckShm::NumberOfSocketSizeEqPacket]++;}
    
      //std::cerr << "Lengths " << n << ", " << np << std::endl;

      ip=0;
      while(ip<(n/8)) {
	if(ip>0) {if(countEnable) p->_array[LinkCheckShm::NumberOfSocketsGtOnePacket]++;}
	
	/*
	  if(n>np) {
	  if(countEnable) (*p)[3]++;
	  ip=(np+7)/8;
	  np=8*(((buffer64[ip]>>48)&0xff)+1);
	  if((n-np)>np) {
	  if(countEnable) (*p)[4]++;
	  }
	  } else if(n!=8*(((buffer64[0]>>48)&0xff)+1)) {
	  if(countEnable) (*p)[5]++;
	  
	  
	  std::cerr << "n = " << n << ", buffer = " << 8*(((buffer64[0]>>48)&0xff)+1) << std::endl;
	  } else {
	*/
	if((buffer64[ip]>>56)!=0xac) {
	  if(countEnable) p->_array[LinkCheckShm::NumberOfPacketBadHeaders]++;
	  //if(ip>0) {if(countEnable) p->_array[]++;;}
	  /*
	  std::cerr << "BAD HEADER, ip = " << ip << " = 0x"
		    << std::hex << std::setfill('0')
		    << std::setw(16) << buffer64[ip]
		    << std::dec << std::setfill(' ')
		    << std::endl;
	  */
	} else {
	  if((buffer64[ip]&maskPH)!=(predictedPH&maskPH)) {
	    if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkips]++;
	    //if(ip>0) {if(countEnable) p->_array[]++;}

	    if((buffer64[ip]&maskPH)>(predictedPH&maskPH)) {
	      if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkipsGt0]++;
	      if((buffer64[ip]&maskPH)-(predictedPH&maskPH)<10) {
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedLtP10]++;
	      } else if((buffer64[ip]&maskPH)-(predictedPH&maskPH)<100) {
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedLtP100]++;
	      } else if((buffer64[ip]&maskPH)-(predictedPH&maskPH)<1000) {
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedLtP1000]++;
	      } else if((buffer64[ip]&maskPH)-(predictedPH&maskPH)<10000) {
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedLtP10000]++;
	      } else if((buffer64[ip]&maskPH)-(predictedPH&maskPH)<100000) {
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedLtP100000]++;
	      } else {
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedGeP100000]++;
	      }
	      if(countEnable) p->_array[LinkCheckShm::NumberOfTotalSkippedPacketsGt0]+=(buffer64[ip]&maskPH)-(predictedPH&maskPH);
	    }

	    if((buffer64[ip]&maskPH)<(predictedPH&maskPH)) {
	      if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkipsLt0]++;
	      if((predictedPH&maskPH)-(buffer64[ip]&maskPH)<10) {		
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedGtM10]++;
	      } else if((predictedPH&maskPH)-(buffer64[ip]&maskPH)<100) {		
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedGtM100]++;
	      } else if((predictedPH&maskPH)-(buffer64[ip]&maskPH)<1000) {		
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedGtM1000]++;
	      } else if((predictedPH&maskPH)-(buffer64[ip]&maskPH)<10000) {		
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedGtM10000]++;
	      } else if((predictedPH&maskPH)-(buffer64[ip]&maskPH)<100000) {
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedGtM100000]++;
	      } else {
		if(countEnable) p->_array[LinkCheckShm::NumberOfPacketSkippedLeM100000]++;
	      }
	      if(countEnable) p->_array[LinkCheckShm::NumberOfTotalSkippedPacketsLt0]+=(predictedPH&maskPH)-(buffer64[ip]&maskPH);
	    }
	    /*
	    std::cerr << "NOT PREDICTED HEADER, ip = " << ip << " = 0x"
		      << std::hex << std::setfill('0')
		      << std::setw(16) << buffer64[ip]
		      << std::dec << std::setfill(' ')
		      << std::endl;
	    */
	  } else {
	    countEnable=true;
	  }      
	  predictedPH=buffer64[ip]+1;
	}
	ip+=((buffer64[ip]>>48)&0xff)+1;
      }
    }
  }

  return 0;
}
