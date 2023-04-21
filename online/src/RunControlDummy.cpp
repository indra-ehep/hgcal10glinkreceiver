#include <iostream>
#include <sstream>
#include <vector>

#include <sys/types.h>
#include <unistd.h>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

template<> const key_t ShmSingleton<RunControlFsmShm>::theKey(0xcebe00);

void request(RunControlFsmShm *p, RunControlFsmEnums::Command fr, uint32_t s=0) {
  std::cout << "WAIT" << std::endl;
  while(!p->rcLock());
  
  p->setCommandDataSize(s);
  //p->forceCommand(fr);
  p->setCommand(fr);
  std::cout << "SLEEP" << std::endl;
  sleep(2);
}

int main(int argc, char *argv[]) {
  uint16_t bits(1);
  unsigned nBits(1);
  bool isBit[8]={true,false,false,false,false,false,false,false};
  
  if(argc>1) {
    std::istringstream sin(argv[argc-1]);
    sin >> bits;

    nBits=0;
  for(unsigned i(0);i<8;i++) {
    if((bits&(1<<i))!=0) {
      nBits++;
      isBit[i]=true;
    } else {
      isBit[i]=false;
    }
  }
    
  std::cout << "Bits = 0x"
	    << std::hex << std::setfill('0')
	    << std::setw(2) << unsigned(bits)
	    << std::dec << std::setfill(' ')
	    << ", nbits = " << nBits
	    << std::endl;
  }
  
  std::vector< ShmSingleton<RunControlFsmShm> > vShmSingleton(nBits);
  std::vector<RunControlFsmShm*> vPtr;

  unsigned j(0);
  for(unsigned i(0);i<8;i++) {
    if(isBit[i]) {
      vShmSingleton[j].setup(1234560+i);
      vPtr.push_back(vShmSingleton[j].payload());
      std::cout << j << " with key "
	    << std::hex << std::setfill('0')
	    << std::setw(2) << 1234560+i
	    << std::dec << std::setfill(' ')
	    << std::endl;
      j++;
    }
  }
  assert(j==nBits);
  
  // Connect to shared memory
  //ShmSingleton<RunControlFsmShm> shmU;
  //ShmSingleton<RunControlFsmShm> &shmU(vShmSingleton[0]);  
  //RunControlFsmShm* const ptrRunFileShm(shmU.payload());
  RunControlFsmShm* const ptrRunFileShm(vPtr[0]);
  
  /*
  request(ptrRunFileShm,RunControlFsmEnums::ConfigureA);
  request(ptrRunFileShm,RunControlFsmEnums::ConfigureB);
  request(ptrRunFileShm,RunControlFsmEnums::Start);
  sleep(10);
  request(ptrRunFileShm,RunControlFsmEnums::Pause);
  request(ptrRunFileShm,RunControlFsmEnums::Resume);
  sleep(10);
  request(ptrRunFileShm,RunControlFsmEnums::Stop);
  request(ptrRunFileShm,RunControlFsmEnums::HaltB);
  request(ptrRunFileShm,RunControlFsmEnums::HaltA);
  */

  unsigned cx;

  
  
  while(true) {
  vPtr[0]->print();

  std::cout << "WAIT" << std::endl;
  for(unsigned i(0);i<vShmSingleton.size();i++) {
    while(!vPtr[i]->rcLock());
  }
  
  vPtr[0]->print();

  std::cout << std::endl;
  for(unsigned i(0);i<RunControlFsmEnums::EndOfCommandEnum;i++) {
    std::cout << "Command " << i << " = " << RunControlFsmEnums::commandName((RunControlFsmEnums::Command)i) << std::endl;
  }

  std::cout << std::endl << "Enter next command number:" << std::endl;
  std::cin >> cx;
  
  RunControlFsmEnums::Command c((RunControlFsmEnums::Command)cx);
  
  std::cout << "Command " << cx << " = " << RunControlFsmEnums::commandName(c) << std::endl;
  
  for(unsigned i(0);i<vShmSingleton.size();i++) {
    vPtr[i]->setCommand(c);
  }
  }
  return 0;
}
