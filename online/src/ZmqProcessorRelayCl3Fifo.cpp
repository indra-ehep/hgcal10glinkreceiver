#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "ZmqPcProcessorFifo.h"

using namespace Hgcal10gLinkReceiver;

int main() {
  return (ZmqPcProcessorFifo(ProcessorRelayCl3FifoShmKey,ProcessorRelayCl3FifoPort)?0:1);
}
