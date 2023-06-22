#include <iostream>
#include <iomanip>
#include <array>

#include <sys/types.h>
#include <unistd.h>

#include "LinkCheckShm.h"

int main(int argc, char *argv[]) {
  unsigned sleepSecs(1);

  if(argc>1) {
    std::istringstream sin(argv[1]);
    sin >> sleepSecs;
    std::cout << "Setting sleepSecs = " << sleepSecs << std::endl;
  }
  
  ShmSingleton<LinkCheckShm> shm;
  shm.setup(0xce2302);
  LinkCheckShm* const p(shm.payload());

  uint64_t pOld[2]={0,0};
  uint64_t bOld[2]={0,0};
  uint64_t pNew,bNew,pDiff,bDiff;

  unsigned i(0),j(0);
  LinkCheckShm lcs[2];
  lcs[1]=*p;
  
  uint64_t t(time(0));
  std::cout << ctime((time_t*)(&t)) << std::endl;
  lcs[1].print();
  
  while(true) {
    /*
    for(unsigned i(0);i<30;i++) {
      std::cout << "Value " << std::setw(2) << i << " = " << std::setw(20) << (*p)[i] << std::endl;
    }
    */
    lcs[i]=*p;

    j=(i+1)%2;
    if(sleepSecs>0 || lcs[i]._array[LinkCheckShm::NumberOfPacketSkips]!=lcs[j]._array[LinkCheckShm::NumberOfPacketSkips]) {

    uint64_t t(time(0));
    std::cout << ctime((time_t*)(&t)) << std::endl;
    lcs[i].print();
       
    std::cout << std::endl;
    std::cout << " Rates:" << std::endl;
    std::cout << "  Socket packet rate = " << std::setw(9) << std::setprecision(6)
	      << 0.001*(lcs[i]._array[LinkCheckShm::NumberOfSocketPackets]-lcs[j]._array[LinkCheckShm::NumberOfSocketPackets])
	      << " kHz" << std::endl;
    std::cout << "  Socket data rate   = " << std::setw(9) << std::setprecision(6)
	      << 0.000000008*(lcs[i]._array[LinkCheckShm::BytesOfSocketPackets]-lcs[j]._array[LinkCheckShm::BytesOfSocketPackets])
	      << " Gbit/s" << std::endl;

    i=j;

    /*
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
    */
    }
    if(sleepSecs>0) sleep(std::max(sleepSecs,1U));
  }

  return 0;
}
