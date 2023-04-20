#include <iostream>
#include <sstream>

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

  // Connect to shared memory
  ShmSingleton<RunControlFsmShm> shmU;
  RunControlFsmShm* const ptrRunFileShm(shmU.payload());
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

  unsigned iOld(999);
  unsigned cx;
  
  while(true) {
  ptrRunFileShm->print();

  std::cout << "WAIT" << std::endl;
  while(!ptrRunFileShm->rcLock());

  ptrRunFileShm->print();

  std::cout << std::endl;
  for(unsigned i(0);i<RunControlFsmEnums::EndOfCommandEnum;i++) {
    std::cout << "Command " << i << " = " << RunControlFsmEnums::commandName((RunControlFsmEnums::Command)i) << std::endl;
  }

  std::cout << std::endl << "Enter next command number:" << std::endl;
  std::cin >> cx;
  if(iOld==999) iOld=cx+1;
  
  if(iOld==9 && cx==0) {
  } else if(iOld>cx) {
    if((iOld-cx)>1) {
      std::cout << std::endl << "SURE??? Re-enter command number:" << std::endl;
      std::cin >> cx;
    }
  } else if(iOld==cx) {
      std::cout << std::endl << "SURE??? Re-enter command number:" << std::endl;
      std::cin >> cx;    
  } else if(iOld<cx) {
    if((cx-iOld)>1) {
      std::cout << std::endl << "SURE??? Re-enter command number:" << std::endl;
      std::cin >> cx;
    }
  }
  iOld=cx;
  
  RunControlFsmEnums::Command c((RunControlFsmEnums::Command)cx);
  
  std::cout << "Command " << cx << " = " << RunControlFsmEnums::commandName(c) << std::endl;
  ptrRunFileShm->setCommand(c);
  }
  
  return 0;
}
