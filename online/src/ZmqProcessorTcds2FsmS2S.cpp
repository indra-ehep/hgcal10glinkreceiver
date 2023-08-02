#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "ZmqProcessorFsmS2S.h"

using namespace Hgcal10gLinkReceiver;

int main() {
  return (ZmqProcessorFsmS2S(ProcessorTcds2FsmShmKey,ProcessorTcds2FsmPort)?0:1);
}
