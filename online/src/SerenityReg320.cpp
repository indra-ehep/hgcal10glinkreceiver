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

#include "SerenityReg320.h"

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

  smd.setDefaults();
  /*
  YAML::Node n;
  smd.configuration(n);
  std::cout << n << std::endl;

  YAML::Node n2;
  smd.status(n2);
  std::cout << n2 << std::endl;
  */
  YAML::Node n3;
  smd.monitoring(n3);
  std::cout << n3 << std::endl;

  return 0;
}
