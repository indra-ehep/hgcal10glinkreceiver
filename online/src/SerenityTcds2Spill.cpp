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
#include "SerenityReg320.h"
#include "ProcessorFastControlPlusDaq.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char *argv[]) {
  /*
  // Turn off printing
  bool printEnable(false);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-p" ||
       std::string(argv[i])=="--printEnable")
      std::cout << "Setting printEnable to true" << std::endl;
    printEnable=true;
  }
  */
  SerenityUhal::setUhalLogLevel();

  SerenityReg320 smd;
  smd.makeTable();
  //return 0;

  while(true) {

    // Unthrottle
    smd.uhalWrite("ctrl1.l1a_throttle_user",0x0);
    usleep( 5000000);

    // Throttle
    smd.uhalWrite("ctrl1.l1a_throttle_user",0x1);
    usleep(10000000);
  }

  return 0;
}
