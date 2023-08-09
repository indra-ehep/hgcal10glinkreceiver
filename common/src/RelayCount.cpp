#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <cassert>
#include <sys/types.h>
#include <getopt.h>

#include "RelayReader.h"
#include "RecordPrinter.h"
#include "RecordYaml.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char** argv) {
  if(argc<2) {
    std::cerr << argv[0] << ": no relay number specified" << std::endl;
    return 1;
  }

  unsigned relayNumber(0);
  
  // Handle relay number
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;

  std::string directory(std::string("dat/Relay")+argv[1]);
  
  FsmState::State state(FsmState::Halted);
  FsmState::State previousState(FsmState::EndOfStateEnum);

  //RecordT<4*1024-1> r;
  RecordYaml ry;
  Record &r((Record&)ry);
  
  uint32_t nRecord[FsmState::EndOfStateEnum+1];
  for(unsigned i(0);i<=FsmState::EndOfStateEnum;i++) nRecord[i]=0;
  
  RelayReader relayReader;
  relayReader.setDirectory(directory);
  assert(relayReader.openRelay(relayNumber));
  /*
  unsigned nCfgA(0);
  unsigned nCfgB(0);
  unsigned nSt(0);
  unsigned nPs(0);
  unsigned nRs(0);
  unsigned nSp(0);

  unsigned nRunSt(0);
  unsigned nRunPs(0);
  unsigned nRunEv(0);
  unsigned nRunRs(0);
  unsigned nRunSp(0);
  unsigned nSeqOff;
  
  unsigned nCfgInRelay(0);

  unsigned nRunInCfg(0);
  unsigned nRunInRelay(0);
  
  unsigned nEvtInRun(0);
  unsigned nEvtInCfg(0);
  unsigned nEvtInRelay(0);
        
  RecordHeader previousRecordHeader;

  unsigned runNumber(0);
  unsigned maxEvents(0);
  unsigned maxSeconds(0);
  unsigned maxSpills(0);
  */
  
  while(relayReader.read(&r)) {
    if(r.state()<FsmState::EndOfStateEnum) nRecord[r.state()]++;
    else nRecord[FsmState::EndOfStateEnum]++;
  }

  relayReader.close();
  
  // Print out table
  std::cout << std::endl << "Number of records seen by state" << std::endl;
  for(unsigned i(0);i<=FsmState::EndOfStateEnum;i++) {
    std::cout << " State " << FsmState::stateName((FsmState::State)i)
	      << ", number of records = " << std::setw(10)
	      << nRecord[i] << std::endl;
  }
  std::cout << std::setw(10) << relayNumber << ", "
	    << std::setw(10) << nRecord[FsmState::Running] << std::endl;
    
  return 0;
}
