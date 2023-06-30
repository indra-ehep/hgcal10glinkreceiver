#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <cstdlib>

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

  unsigned len=99;

  srand(time(NULL));
  
  ShmSingleton<RunWriterDataFifo> shmU;
  shmU.setup(key);
  RunWriterDataFifo *prcfs=shmU.payload();
  //prcfs->print();
  
  RecordT<1023> *r;
  
  for(unsigned i(0);i<RunWriterDataFifo::BufferDepth;i++) {
    r=(RecordT<1023>*)((prcfs->_buffer)+i);
    r->reset(FsmState::Running);
    r->setPayloadLength(len);

    uint32_t *p32((uint32_t*)(r+1));
    for(unsigned j(0);j<2*len;j++) {
      p32[j]=rand();
    }

    if(i<3) {
      std::cout << "Buffer location " << i << ": ";
      //r->print();
      r->RecordHeader::print();
    }
  }
  
  return true;
}
