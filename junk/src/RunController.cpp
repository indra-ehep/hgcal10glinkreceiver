/*
g++ -I inc src/RunStart.cpp -o bin/RunStart.exe
 */

#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <unistd.h>

#include "RunFileShm.h"

int main(int argc, char *argv[]) {
  if(argc<2) {
    std::cerr << argv[0] << " must have a request" << std::endl;

    for(unsigned i(0);i<RunControlFsmShm::EndOfFsmRequestsEnum;i++) {
      std::cerr << RunControlFsmShm::fsmRequestName((RunControlFsmShm::FsmRequests)i) << std::endl;
    }
    return 1;
  }

  RunControlFsmShm rsfs;
  
  ShmSingleton<RunFileShm0> shm0;
  ShmSingleton<RunFileShm1> shm1;
  ShmSingleton<RunFileShm2> shm2;
  /*
  unsigned ifr;
  std::istringstream sin(argv[1]);
  sin >> ifr;
  if(ifr>=RunControlFsmShm::EndOfFsmRequestsEnum) return 2;
  
  RunControlFsmShm::FsmRequests fr((RunControlFsmShm::FsmRequests)ifr);
  */
  RunControlFsmShm::FsmRequests fr;

  shm2.payload()->_runControlFsmShm.setRcLock();

  fr=RunControlFsmShm::Initialize;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(3);
  
  fr=RunControlFsmShm::PreConfigure;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(3);
  
  fr=RunControlFsmShm::Configure;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(3);

  fr=RunControlFsmShm::Start;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(3);

  fr=RunControlFsmShm::Pause;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(3);

  fr=RunControlFsmShm::Resume;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(3);
  
  fr=RunControlFsmShm::Stop;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(3);

  fr=RunControlFsmShm::Unconfigure;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(3);

  fr=RunControlFsmShm::Halt;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;

  /*
  sleep(3);
  
  fr=RunControlFsmShm::Shutdown;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SHUTDOWN" << std::endl;

  fr=RunControlFsmShm::Initialize;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  */
  /*
  std::string frn(argv[1]);
  std::cout << frn << std::endl;
  
  bool found(false);
  for(unsigned i(0);i<RunControlFsmShm::EndOfFsmRequestsEnum;i++) {
    std::cout << frn << " ?= " << RunControlFsmShm::fsmRequestName((RunControlFsmShm::FsmRequests)i) << std::endl;
    if(frn==RunControlFsmShm::fsmRequestName((RunControlFsmShm::FsmRequests)i)) {
      fr=(RunControlFsmShm::FsmRequests)i;
      found=true;
    }
  }

  if(!found) {
    std::cerr << "Unknown request" << std::endl;
    return 2;
  }
  */

  

  return 0;
}
