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

#include "ProcessorRelayWriter.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char *argv[]) {

  unsigned iBits(0x7);

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
    
    if(std::string(argv[i])=="-i") {
      std::istringstream sin(argv[i+1]);
      sin >> iBits;
      std::cout << "Setting input bits to 0x" << std::hex 
		<< iBits << std::dec << std::endl;
    }
  }
  
  ProcessorRelayWriter pb;
  pb.setPrintEnable(  printEnable);
  pb.setCheckEnable(  checkEnable);
  pb.setAssertEnable(assertEnable);
  pb.setInputFifoEnables(iBits);
  
  pb.setUpAll(ProcessorRelayFsmShmKey,
	      ProcessorRelayCl0FifoShmKey,
  	      ProcessorRelayCl1FifoShmKey,
  	      ProcessorRelayCl2FifoShmKey);

  return 0;
}
