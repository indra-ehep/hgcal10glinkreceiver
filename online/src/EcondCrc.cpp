#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "EcondHeader.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char *argv[]) {
  if(argc<2) return 1;

  uint64_t word(0);

  std::istringstream sin(argv[1]);
  sin >> std::hex >> word;

  EcondHeader *eh((EcondHeader *)(&word));
  eh->print();
  return 0;
}
