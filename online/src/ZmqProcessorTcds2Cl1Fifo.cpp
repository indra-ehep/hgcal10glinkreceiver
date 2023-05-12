#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "ZmqSerenityProcessorFifo.h"

using namespace Hgcal10gLinkReceiver;

int main() {
  return (ZmqSerenityProcessorFifo(ProcessorTcds2Cl1FifoShmKey,ProcessorTcds2Cl1FifoPort)?0:1);
}
