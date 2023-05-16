#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include "SystemParameters.h"
#include "DataFifo.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

int main() {

  ShmSingleton< DataFifoT<6,1024> > shmU[4];
  DataFifoT<6,1024> *ptr[4];
  
  ptr[0]=shmU[0].setup(ProcessorFastControlDl0FifoShmKey);
  ptr[1]=shmU[1].setup(ProcessorFastControlDl1FifoShmKey);
  ptr[2]=shmU[2].setup(ProcessorFastControlCl0FifoShmKey);
  ptr[3]=shmU[3].setup(ProcessorTcds2Cl1FifoShmKey);

  while(true) {
    for(unsigned i(0);i<4;i++) {
      ptr[i]->print();
    }
    system("df -h dat");
    usleep(1000000);
  }
  
  return 0;
}
