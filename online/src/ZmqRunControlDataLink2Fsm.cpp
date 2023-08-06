#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "ZmqRunControlFsm.h"

using namespace Hgcal10gLinkReceiver;

int main() {
  return (ZmqRunControlFsm(ProcessorDaqLink2FsmShmKey,ProcessorDaqLink2FsmPort)?0:1);
}
