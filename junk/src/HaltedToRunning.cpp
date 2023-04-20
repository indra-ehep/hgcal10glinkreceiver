#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <unistd.h>

#include "RunFileShm.h"

RunFileShm0* p0;
RunFileShm1* p1;
RunFileShm2* p2;

void request(RunControlFsmShm::FsmRequests fr, uint32_t s=0) {
  p2->_runControlFsmShm.setFsmRequestDataSize(s);
  p2->_runControlFsmShm.forceFsmRequest(fr);
  std::cout << "SLEEP " << std::endl;
  sleep(2);
}

int main(int argc, char *argv[]) {
  ShmSingleton<RunFileShm0> shm0;
  ShmSingleton<RunFileShm1> shm1;
  ShmSingleton<RunFileShm2> shm2;
  
  p0=shm0.payload();
  p1=shm1.payload();
  p2=shm2.payload();

  uint64_t *pdb(p2->_runControlFsmShm.fsmRequestDataBuffer());
  pdb[0]=time(0);
  
  request(RunControlFsmShm::PreConfigure,1);
  request(RunControlFsmShm::Configure);
  request(RunControlFsmShm::Start);

  return 0;
}
