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

  //shm2.payload()->_runControlFsmShm.setRcLock();
  RunControlFsmShm::FsmRequests fr;

  fr=RunControlFsmShm::Pause;
  shm0.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm1.payload()->_runControlFsmShm.setFsmRequest(fr);
  shm2.payload()->_runControlFsmShm.setFsmRequest(fr);

  return 0;
}
