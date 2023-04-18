/*
g++ -I inc src/RunStart.cpp -o bin/RunStart.exe
 */

#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <unistd.h>

#include "RunFileShm.h"

int main(int argc, char *argv[]) {
  ShmSingleton<RunFileShm0> shm0;
  ShmSingleton<RunFileShm1> shm1;
  ShmSingleton<RunFileShm2> shm2;

  RunControlFsmShm::FsmRequests fr;

  //shm2.payload()->_runControlFsmShm.setRcLock();
  
  fr=RunControlFsmShm::PreConfigure;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(2);
  
  fr=RunControlFsmShm::Configure;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(2);

  fr=RunControlFsmShm::Start;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);

  return 0;
}
