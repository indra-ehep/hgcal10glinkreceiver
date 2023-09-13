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
  */
  
  Hgcal10gLinkReceiver::FileReader _relayReader;
  Hgcal10gLinkReceiver::FileReader runReader[3];

  _relayReader.openRelay(relayNumber);

  std::ostringstream srun;
  srun << "dat/Relay" << relayNumber;

  runReader[0].setDirectory(srun.str());
  runReader[1].setDirectory(srun.str());
  runReader[2].setDirectory(srun.str());
  
  Hgcal10gLinkReceiver::RelayTcds2Check rtc;
  
  // Make the buffer space for the records and some useful casts for configuration and event records
  Hgcal10gLinkReceiver::RecordT<4095> *rRun(new Hgcal10gLinkReceiver::RecordT<4095>[3]);

  //std::vector< Hgcal10gLinkReceiver::RecordT<4095>* > vRunning;
  std::vector<const Hgcal10gLinkReceiver::Record*> vRunning;
  vRunning.resize(3);
  //vRunning.push_back(new Hgcal10gLinkReceiver::RecordT<4095>);
  //vRunning.push_back(new Hgcal10gLinkReceiver::RecordT<4095>);
  //vRunning.push_back(new Hgcal10gLinkReceiver::RecordT<4095>);
  
  Hgcal10gLinkReceiver::RecordT<4*4095> *rxxx(new Hgcal10gLinkReceiver::RecordT<4*4095>);

  unsigned nEvents(0);
  uint64_t relayUtc(0);
  uint64_t runUtc;
  
  while(_relayReader.read(rxxx)) {
    if(rxxx->state()==Hgcal10gLinkReceiver::FsmState::Starting) {
      Hgcal10gLinkReceiver::RecordYaml *rCfg((Hgcal10gLinkReceiver::RecordYaml*)rxxx);
      YAML::Node n(YAML::Load(rCfg->string()));

      unsigned runNumber(n["RunNumber"].as<unsigned>());
      runReader[0].openRun(runNumber,0);
      runReader[1].openRun(runNumber,1);
      runReader[2].openRun(runNumber,2);

      runReader[0].read(rRun  );
      runReader[1].read(rRun+1);
      runReader[2].read(rRun+2);
    }
    
    if(rxxx->state()==Hgcal10gLinkReceiver::FsmState::Stopping) {
      bool done(false);

      while(!done) {
	done=true;

	for(unsigned l(0);l<3;l++) {
	  if(runReader[l].read(rRun+l)) {
	    done=false;
	    vRunning[l]=(Hgcal10gLinkReceiver::Record*)(rRun+l);
	  } else {
	    std::cout << "WARNING: read failed for link " << l << std::endl;
	    vRunning[l]=nullptr;
	  }
	}
	
	rtc.runningRecord(vRunning);
      }
    }
    
    rtc.nonRunningRecord(rxxx);
  }
  
  std::cout << "Total number of events found = " << nEvents << std::endl;
  
  delete rxxx;
  
  return 0;
}
