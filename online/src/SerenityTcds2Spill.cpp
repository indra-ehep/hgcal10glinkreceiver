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

  SerenityUhal smd;
  smd.makeTable();
  //return 0;

  bool spill(true);
  
  while(true) {

    // Unthrottle
    smd.uhalWrite("reg_320.ctrl1",0x0);
    usleep(spill? 5000000:89);

    // Throttle
    smd.uhalWrite("reg_320.ctrl1",0x2);
    usleep(spill?25000000:89);
  }

  return 0;
}
