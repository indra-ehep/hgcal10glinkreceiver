#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "ZmqRunControlFsm.h"

using namespace Hgcal10gLinkReceiver;

int main() {
  return (ZmqRunControlFsm(ProcessorDummyFsmShmKey,RunControlDummyFsmPort)?0:1);
}
