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

#include "SerenityAligner.h"

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

  SerenityAligner smd;
  smd.makeTable();

  YAML::Node n;
  smd.configuration(n);
  std::cout << n << std::endl;

  return 0;
}
