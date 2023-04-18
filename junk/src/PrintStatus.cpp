/*
g++ -I inc src/PrintStatus.cpp -o bin/PrintStatus.exe
 */

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include "ShmSingleton.hh"
#include "RunFileShm.h"


int main(int argc, char *argv[]) {
  ShmSingleton<RunFileShm0> shm0;
  ShmSingleton<RunFileShm1> shm1;
  ShmSingleton<RunFileShm2> shm2;
  RunFileShm0* const p0(shm0.payload());
  RunFileShm1* const p1(shm1.payload());
  RunFileShm2* const p2(shm2.payload());

  uint64_t pOld[3]={0,0,0};
  uint64_t bOld[3]={0,0,0};
  uint64_t pNew,bNew,pDiff,bDiff;

  while(true) {
    uint64_t t(time(0));
    std::cout << ctime((time_t*)(&t)) << std::endl;

    std::cout << "Link0: ";
    p0->print();
    std::cout << " Number of processes attached = "
	      << shm0.numberAttached() << std::endl;
    
    // Get new packet count
    pNew=p0->_packetNumberInRun;
    pDiff=pNew-pOld[0];
    std::cout << " Packet rate = " << std::setw(10) << pDiff << " packets/sec" << std::endl;
    
    // Get new byte count
    bNew=p0->_bytesInRun;
    bDiff=8*(bNew-bOld[0]);
    std::cout << " Data rate = "
	      << std::setw(3) <<  bDiff/1000000000 << "," << std::setfill('0')
	      << std::setw(3) << (bDiff/1000000)%1000 << ","
	      << std::setw(3) << (bDiff/1000)%1000 << ","
	      << std::setw(3) <<  bDiff%1000 << std::setfill(' ')
	      << " bits/sec" << std::endl;
    
    // Update previous values
    pOld[0]=pNew;
    bOld[0]=bNew;

    
    std::cout << std::endl << "Link1: ";
    p1->print();
    std::cout << " Number of processes attached = "
	      << shm1.numberAttached() << std::endl;
    
    // Get new packet count
    pNew=p1->_packetNumberInRun;
    pDiff=pNew-pOld[1];
    std::cout << " Packet rate = " << std::setw(10) << pDiff << " packets/sec" << std::endl;
    
    // Get new byte count
    bNew=p1->_bytesInRun;
    bDiff=8*(bNew-bOld[1]);
    std::cout << " Data rate = "
	      << std::setw(3) <<  bDiff/1000000000 << "," << std::setfill('0')
	      << std::setw(3) << (bDiff/1000000)%1000 << ","
	      << std::setw(3) << (bDiff/1000)%1000 << ","
	      << std::setw(3) <<  bDiff%1000 << std::setfill(' ')
	      << " bits/sec" << std::endl;
    
    // Update previous values
    pOld[1]=pNew;
    bOld[1]=bNew;

    
    std::cout << std::endl << "Link2: ";
    p2->print();
    std::cout << " Number of processes attached = "
	      << shm2.numberAttached() << std::endl;
    
    // Get new packet count
    pNew=p2->_packetNumberInRun;
    pDiff=pNew-pOld[2];
    std::cout << " Packet rate = " << std::setw(10) << pDiff << " packets/sec" << std::endl;
    
    // Get new byte count
    bNew=p2->_bytesInRun;
    bDiff=8*(bNew-bOld[2]);
    std::cout << " Data rate = "
	      << std::setw(3) <<  bDiff/1000000000 << "," << std::setfill('0')
	      << std::setw(3) << (bDiff/1000000)%1000 << ","
	      << std::setw(3) << (bDiff/1000)%1000 << ","
	      << std::setw(3) <<  bDiff%1000 << std::setfill(' ')
	      << " bits/sec" << std::endl;
    
    // Update previous values
    pOld[2]=pNew;
    bOld[2]=bNew;

    sleep(1);    
  }

  return 0;
}
