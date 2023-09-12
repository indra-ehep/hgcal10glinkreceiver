/*
g++ -Wall -Icommon/inc src/RelayTcds2Check.cpp -lyaml-cpp -o bin/RelayTcds2Check.exe
*/

#include <iostream>
#include <iomanip>
#include <cassert>

#include <yaml-cpp/yaml.h>

#include "TH1D.h"
#include "TH2D.h"
#include "TFileHandler.h"

#include "RelayReader.h"
#include "EcontEnergies.h"
#include "RelayTcds2Check.h"


int main(int argc, char** argv) {
  if(argc<2) {
    std::cerr << argv[0] << ": No relay number specified" << std::endl;
    return 1;
  }

  uint32_t relayNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;

  if(relayNumber==0) {
    std::cerr << argv[0] << ": Relay number uninterpretable" << std::endl;
    return 2;
  }

  std::ostringstream sout;
  sout << "RelayTcds2Check_" << relayNumber;
  TFileHandler tfh(sout.str().c_str());
  
  // Create the file reader
  /*
  Hgcal10gLinkReceiver::RelayReader _relayReader;
  _relayReader.enableLink(0,true);
  _relayReader.enableLink(1,false);
  _relayReader.enableLink(2,false);
  _relayReader.openRelay(relayNumber);
  */
  
  Hgcal10gLinkReceiver::FileReader _relayReader;
  Hgcal10gLinkReceiver::FileReader runReader[3];

  RelayTcds2Check rtc;
  
  // Make the buffer space for the records and some useful casts for configuration and event records
  Hgcal10gLinkReceiver::RecordT<4095> rRun(new Hgcal10gLinkReceiver::RecordT<4*4095>[3]);
  std::vector< Hgcal10gLinkReceiver::RecordT<4095>* > vRunning;
  vRunning.push_back(new Hgcal10gLinkReceiver::RecordT<4095>);
  vRunning.push_back(new Hgcal10gLinkReceiver::RecordT<4095>);
  vRunning.push_back(new Hgcal10gLinkReceiver::RecordT<4095>);
  
  Hgcal10gLinkReceiver::RecordT<4*4095> *rxxx(new Hgcal10gLinkReceiver::RecordT<4*4095>);

  unsigned nEvents(0);
  uint64_t relayUtc(0);
  uint64_t runUtc;
  
  while(_relayReader.read(rxxx)) {
    if(rxxx->state()==Hgcal10gLinkReceiver::FsmState::Stopping) {

      if(runReader[0].read(&(vRunning[0]))) {
      }
      runReader[1].read(&(vRunning[1]));
      runReader[2].read(&(vRunning[2]));

      rtc.runningRecord(vRunning);
    }
    
    rtc.nonRunningRecord(rxxx);
  }
  
  std::cout << "Total number of events found = " << nEvents << std::endl;
  
  delete rxxx;
  
  return 0;
}
