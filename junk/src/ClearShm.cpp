#include <iostream>
#include <sstream>
#include <vector>

#include "SystemParameters.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char *argv[]) {
  system("ipcs -m");

  std::vector<uint32_t> v;
  v.push_back(RunControlDummyFsmShmKey);
  v.push_back(RunControlFrontEndFsmShmKey);
  v.push_back(RunControlFastControlFsmShmKey);
  v.push_back(RunControlTcds2FsmShmKey);
  v.push_back(RunControlRelayFsmShmKey);
  v.push_back(RunControlStageFsmShmKey);
  v.push_back(RunControlDaqLink0FsmShmKey);
  v.push_back(RunControlDaqLink1FsmShmKey);
  
  for(unsigned i(0);i<v.size();i++) {
    std::ostringstream s0;
    s0 << "ipcrm -M " << v[i];
    system(s0.str().c_str());
  }

  // Obsolete keys
  /*
    system("ipcrm -M 0x075bcd15");
    system("ipcrm -M 0xced0cf00");
    system("ipcrm -M 0xced0cf01");
    system("ipcrm -M 0xced0cf02");
    system("ipcrm -M 0xced0da00");
    system("ipcrm -M 0xced0da01");
  */
  
  system("ipcs -m");
  return 0;
}
