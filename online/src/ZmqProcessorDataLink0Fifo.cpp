#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "ZmqPcProcessorFifo.h"

using namespace Hgcal10gLinkReceiver;

int main() {
  return (ZmqPcProcessorFifo(ProcessorDaqLink0FifoShmKey,ProcessorDaqLink0FifoPort)?0:1);
}
