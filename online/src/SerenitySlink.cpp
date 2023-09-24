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

#include "SerenitySlink.h"

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

  SerenitySlink smd;
  smd.makeTable();

  smd.resetGlobalCore();

  return 0;


  smd.setSourceId(0,0xce001234);
  smd.setSourceId(1,0xce001235);

  YAML::Node nc;
  smd.configuration(nc);
  std::cout << std::endl << nc << std::endl;

  smd.channelReset();
  
  YAML::Node ns;
  smd.status(ns);
  std::cout << std::endl << ns << std::endl;

  std::ofstream fout("SerenitySlinkStatus.txt");

  while(true) {
    YAML::Node ns;
    smd.status(ns);
    fout << std::endl << ns << std::endl;
    
    usleep(1000000);
  }

  return 0;
}
