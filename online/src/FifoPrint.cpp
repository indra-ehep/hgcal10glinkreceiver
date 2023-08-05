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

  ShmSingleton<RunWriterDataFifo> shmD[2];
  RunWriterDataFifo *ptrD[2];
  ShmSingleton<RelayWriterDataFifo> shmC[3];
  RelayWriterDataFifo *ptrC[3];
  
  ptrD[0]=shmD[0].setup(ProcessorFastControlDl0FifoShmKey);
  ptrD[1]=shmD[1].setup(ProcessorFastControlDl1FifoShmKey);

  ptrC[0]=shmC[0].setup(ProcessorFastControlCl0FifoShmKey);
  ptrC[1]=shmC[1].setup(ProcessorTcds2Cl1FifoShmKey);
  ptrC[2]=shmC[2].setup(ProcessorFrontEndCl2FifoShmKey);

  uint32_t w[2],wOld[2]={0,0},wDiff[2];
  
  while(true) {
    system("clear");
    for(unsigned i(0);i<2;i++) {
      ptrD[i]->print();
      w[i]=ptrD[i]->writePtr();
    }

    wDiff[0]=(w[0]>=wOld[0]?w[0]-wOld[0]:0);
    wDiff[1]=(w[1]>=wOld[1]?w[1]-wOld[1]:0);
    
    std::cout << std::endl
	      << "Rates: Link 0 = " << std::setw(8) << 0.001*wDiff[0]
	      << " kHz, Link 1 = " << std::setw(8) << 0.001*wDiff[1]
	      << " kHz" << std::endl << std::endl;
    wOld[0]=w[0];
    wOld[1]=w[1];
					     
    for(unsigned i(0);i<3;i++) {
      ptrC[i]->print();
    }
    system("df -h dat");
    usleep(1000000);
  }
  
  return 0;
}
