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
#include "SerenityTrgDaq.h"
//#include "ProcessorFastControlPlusDaq.h"

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

  SerenityTrgDaq smd;
  smd.makeTable();
  smd.setDefaults();
  smd.uhalWrite("trigger_ro.SLink.source_id"  ,0xce000000|3<<4|0,true);
  if(!printEnable) smd.print();

  return 0;
}
