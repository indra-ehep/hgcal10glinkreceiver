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

#include "ShmKeys.h"
#include "ProcessorFastControl.h"

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

  uint64_t buffer[2]={0xffffffffffffffff,0xffffffffffffffff};

  char c='A';
  std::cout << "A = " << c << " = " << unsigned(c) << std::endl;
  c='\0';
  std::cout << "\0 = " << c << " = " << unsigned(c) << std::endl;

  c='A';
  std::string s("123456");
  std::cout << "String length = " << s.size() << " = " << s << " n = " << (s.size()+8)/8 << std::endl;
  s+=std::string("7");
  std::cout << "String length = " << s.size() << " = " << s << " n = " << (s.size()+8)/8 << std::endl;
  std::memcpy(buffer,s.c_str(),s.size()+1);
  std::cout << "Buffer = 0x" 
	    << std::hex << std::setfill('0')
	    << std::setw(16) << buffer[0] << ", 0x"
	    << std::setw(16) << buffer[1]
	    << std::dec << std::setfill(' ')
	    << std::endl;

  s+=std::string("8");
  std::cout << "String length = " << s.size() << " = " << s << " n = " << (s.size()+8)/8 << std::endl;
  std::memcpy(buffer,s.c_str(),s.size()+1);
  std::cout << "Buffer = 0x" 
	    << std::hex << std::setfill('0')
	    << std::setw(16) << buffer[0] << ", 0x"
	    << std::setw(16) << buffer[1]
	    << std::dec << std::setfill(' ')
	    << std::endl;

  s+=std::string("9");
  std::cout << "String length = " << s.size() << " = " << s << " n = " << (s.size()+8)/8 << std::endl;
  std::memcpy(buffer,s.c_str(),s.size()+1);
  std::cout << "Buffer = 0x" 
	    << std::hex << std::setfill('0')
	    << std::setw(16) << buffer[0] << ", 0x"
	    << std::setw(16) << buffer[1]
	    << std::dec << std::setfill(' ')
	    << std::endl;

  s+=std::string("C");

  s=(const char*)(buffer);
  std::cout << "String length = " << s.size() << " = " << s << std::endl;

  ProcessorFastControl pb;
  //ProcessorBase pb;
  pb.setPrintEnable(printEnable);
  //pb.setUpAll(RunControlFastControlShmKey,ProcessorFastControlDataShmKey);
  pb.setUpAll(RunControlFastControlShmKey,ProcessorFastControlDataShmKey,
	      ProcessorDaqLink0FifoShmKey,ProcessorDaqLink1FifoShmKey);
  return 0;
}
