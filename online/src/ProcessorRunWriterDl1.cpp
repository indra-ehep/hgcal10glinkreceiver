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

#include "ProcessorRunWriter.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char *argv[]) {

  // Turn off printing
  bool printEnable(false);
  bool checkEnable(false);
  bool assertEnable(false);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-p" ||
       std::string(argv[i])=="--printEnable") {
      std::cout << "Setting printEnable to true" << std::endl;
      printEnable=true;
    }
    
    if(std::string(argv[i])=="-c" ||
       std::string(argv[i])=="--checkEnable") {
      std::cout << "Setting checkEnable to true" << std::endl;
      checkEnable=true;
    }
    
    if(std::string(argv[i])=="-a" ||
       std::string(argv[i])=="--assertEnable") {
      std::cout << "Setting assertEnable to true" << std::endl;
      assertEnable=true;
    }
  }

  ProcessorRunWriter pb(1);
  pb.setPrintEnable(  printEnable);
  pb.setCheckEnable(  checkEnable);
  pb.setAssertEnable(assertEnable);

  pb.setUpAll(ProcessorDaqLink1FsmShmKey,ProcessorDaqLink1FifoShmKey);
  return 0;
}
