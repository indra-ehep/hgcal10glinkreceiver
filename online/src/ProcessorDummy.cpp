#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <iostream>
#include <cassert>

#include "ProcessorDummy.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char *argv[]) {

  // Turn off printing
  bool printEnable(false);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-p" ||
       std::string(argv[i])=="--printEnable")
      std::cout << "Setting printEnable to true" << std::endl;
    printEnable=true;
  }

  ProcessorDummy pb;
  pb.setPrintEnable(printEnable);
  pb.startFsm(ProcessorDummyFsmShmKey);
  return 0;
}
