#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include <yaml-cpp/yaml.h>

#include "RecordYaml.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char *argv[]) {
  uint32_t seq(0);

  while(true) {
    RecordYaml r;
    r.reset(FsmState::Configuration);
    r.setSequenceCounter(seq++);

    YAML::Node n(YAML::LoadFile("cfg/coordinates.yaml"));
    std::ostringstream sout;
    sout << n << std::endl;
    r.setString(sout.str());
    
    r.print();
    usleep(30*1000000);
  }
  
  return 0;
}
