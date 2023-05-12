#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "ZmqSerenityProcessorFifo.h"

using namespace Hgcal10gLinkReceiver;

int main() {
  return (ZmqSerenityProcessorFifo(ProcessorFastControlDl1FifoShmKey,ProcessorFastControlDl1FifoPort)?0:1);
}
