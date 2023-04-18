/*
g++ -I inc src/ColdStart.cpp -o bin/ColdStart.exe
*/

#include "ShmSingleton.hh"
#include "RunFileShm.h"

int main(int argc, char *argv[]) {
  ShmSingleton<RunFileShm0> shm0;
  ShmSingleton<RunFileShm1> shm1;
  
  shm0.payload()->_requestActive=false;
  shm0.payload()->_requestShutDown=false;
  shm0.payload()->_runStateActive=false;
  
  shm1.payload()->_requestActive=false;
  shm1.payload()->_requestShutDown=false;
  shm1.payload()->_runStateActive=false;
  
  return 0;
}
