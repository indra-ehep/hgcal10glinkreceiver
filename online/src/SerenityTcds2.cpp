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

//#define ProcessorHardware

#include "SerenityHgcroc.h"
#include "SerenityMiniDaq.h"
#include "SerenityTcds2.h"
#include "ProcessorFastControlPlusDaq.h"

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

  SerenityUhal::setUhalLogLevel();

  SerenityTcds2 smd;
  smd.makeTable();
  smd.print();

  char x;

  uint32_t r,w;

  r=smd.uhalRead("ctrl_stat.ctrl1");
  std::cout << "ctrl_stat.ctrl1 read = " << r
	    << " = 0x" << std::hex << r << std::dec << std::endl;

  std::cout << "Continue (y,n)? ";
  std::cin >> x;
  if(x!='y') return 0;

  w=0x00000400;
  std::cout << "ctrl_stat.ctrl1 write = " << w
	    << " = 0x" << std::hex << w << std::dec << std::endl;
  smd.uhalWrite("ctrl_stat.ctrl1",w);

  std::cout << "Continue (y,n)? ";
  std::cin >> x;
  if(x!='y') return 0;

  r=smd.uhalRead("ctrl_stat.ctrl1");
  std::cout << "ctrl_stat.ctrl1 read = " << r
	    << " = 0x" << std::hex << r << std::dec << std::endl;

  std::cout << "Continue (y,n)? ";
  std::cin >> x;
  if(x!='y') return 0;

  w=0;
  std::cout << "ctrl_stat.ctrl1 write = " << w
	    << " = 0x" << std::hex << w << std::dec << std::endl;
  smd.uhalWrite("ctrl_stat.ctrl1",w);

  std::cout << "Continue (y,n)? ";
  std::cin >> x;
  if(x!='y') return 0;

  r=smd.uhalRead("ctrl_stat.ctrl1");
  std::cout << "ctrl_stat.ctrl1 read = " << r
	    << " = 0x" << std::hex << r << std::dec << std::endl;


  std::cout << "Continue (y,n)? ";
  std::cin >> x;
  if(x!='y') return 0;

  w=1;
  smd.uhalWrite("ctrl_stat.ctrl1.arm_ebr",w);
  std::cout << "ctrl_stat.ctrl1.arm_ebr write = " << w
	    << " = 0x" << std::hex << w << std::dec << std::endl;

  std::cout << "Continue (y,n)? ";
  std::cin >> x;
  if(x!='y') return 0;

  w=0;
  smd.uhalWrite("ctrl_stat.ctrl1.arm_ebr",w);
  std::cout << "ctrl_stat.ctrl1.arm_ebr write = " << w
	    << " = 0x" << std::hex << w << std::dec << std::endl;


  return 0;
}
