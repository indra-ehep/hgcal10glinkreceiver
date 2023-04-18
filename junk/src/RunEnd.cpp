/*
g++ -I inc src/RunEnd.cpp -o bin/RunEnd.exe
 */

#include <iostream>

#include "RunFileShm.h"

int main(int argc, char *argv[]) {
  ShmSingleton<RunFileShm0> shm0;
  shm0.payload()->_requestActive=false;

  ShmSingleton<RunFileShm1> shm1;
  shm1.payload()->_requestActive=false;

  return 0;
}
