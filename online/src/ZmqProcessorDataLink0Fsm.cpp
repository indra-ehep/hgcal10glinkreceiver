#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "ZmqProcessorFsm.h"

using namespace Hgcal10gLinkReceiver;

int main() {
  return (ZmqProcessorFsm(ProcessorDataLink0FsmShmKey,ProcessorDataLink0FsmPort)?0:1);
}