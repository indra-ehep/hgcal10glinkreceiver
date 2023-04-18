/*
g++ -I inc src/RunStart.cpp -o bin/RunStart.exe
 */

#include <iostream>
#include <sstream>

#include "RunFileShm.h"


int main(int argc, char *argv[]) {
  if(argc<2) {
    std::cerr << argv[0] << " must have a run number" << std::endl;
    return 1;
  }

  unsigned runNumber;
  bool writeEnable(true);
  bool readEnable(true);
  
  for(int i(1);i<argc-1;i++) {
    if(std::string(argv[i])=="-r" ||
       std::string(argv[i])=="--readDisable") readEnable=false;
    if(std::string(argv[i])=="-w" ||
       std::string(argv[i])=="--writeDisable") writeEnable=false;
  }
  
  std::istringstream iss(argv[argc-1]);
  iss >> runNumber;
  
  ShmSingleton<RunFileShm0> shm0;
  shm0.payload()->_writeEnable=writeEnable;
  shm0.payload()->_readEnable=readEnable;
  shm0.payload()->_runNumber=runNumber;
  shm0.payload()->_requestActive=true;
  
  ShmSingleton<RunFileShm1> shm1;
  shm1.payload()->_writeEnable=writeEnable;
  shm1.payload()->_readEnable=readEnable;
  shm1.payload()->_runNumber=runNumber;
  shm1.payload()->_requestActive=true;

  return 0;
}
