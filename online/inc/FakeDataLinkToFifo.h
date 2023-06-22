#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>

#include "SystemParameters.h"
#include "DataFifo.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

bool FakeDataLinkToFifo(uint32_t key) {
  std::cout << "FakeDataLinkToFifo called with key = 0x"
            << std::hex << std::setfill('0')
            << std::setw(8) << key 
            << std::dec << std::setfill(' ')
            << std::endl;
  
  ShmSingleton<RunWriterDataFifo> shmU;
  shmU.setup(key);
  RunWriterDataFifo *prcfs=shmU.payload();
  //prcfs->print();
  
  Record *r;
  for(unsigned i(0);i<RunWriterDataFifo::BufferDepth;i++) {
    r=(Record*)((prcfs->_buffer)+i);
    r->reset(FsmState::Running);
    r->setPayloadLength(499);
  }
  
  return true;
}
