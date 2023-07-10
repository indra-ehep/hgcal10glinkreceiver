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
  
  RecordConfiguring   &rca((RecordConfiguring&  )r);
  RecordReconfiguring &rcb((RecordReconfiguring&)r);
  RecordConfigured    &rdb((RecordConfigured&   )r);
  RecordStarting      &rst((RecordStarting&     )r);
  RecordRunning       &rrn((RecordRunning&      )r);
  RecordPausing       &rps((RecordPausing&      )r);
//RecordPaused        &rpd((RecordPaused&       )r);
  RecordResuming      &rrs((RecordResuming&     )r);
  RecordStopping      &rsp((RecordStopping&     )r);
  RecordHalting       &rha((RecordHalting&      )r);
  RecordYaml          &ryl((RecordYaml&         )r);

  uint32_t nRecord[FsmState::EndOfStateEnum+1];
  for(unsigned i(0);i<=FsmState::EndOfStateEnum;i++) nRecord[i]=0;
  
  RelayReader relayReader;
  relayReader.setDirectory(directory);
  assert(relayReader.openRelay(relayNumber));

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

  
  while(relayReader.read(&r)) {
    if(previousState!=state) {
      std::cout << std::endl << "***** Current state = "
		<< FsmState::stateName(state)
		<< " *****" << std::endl << std::endl;
      previousState=state;
    }

    if(r.state()<FsmState::EndOfStateEnum) nRecord[r.state()]++;
    else nRecord[FsmState::EndOfStateEnum]++;

    if(r.state()!=FsmState::Running &&
       r.state()!=FsmState::Configured) RecordPrinter(&r);
    if(r.state()==FsmState::Configured) ryl.print();
    if(r.state()==FsmState::Running) RecordPrinter(&r);
    
    // Check the change of state is allowed


    if(r.state()<FsmState::EndOfTransientEnum) {

      if(FsmState::staticState(state)) {
	if(state!=r.state()) {
	  assert(state==FsmState::staticStateBeforeTransient(r.state()));
	  state=FsmState::staticStateAfterTransient(r.state());
	}
      } else if(FsmState::transientState(state)) {
	assert(r.state()==FsmState::staticStateBeforeTransient(state));
	state=r.state();      
      } else {
      }
      
    }



    
    // Check values for each case
    if(r.state()==FsmState::Configuring) {
      assert(rca.valid());
      //assert(rca.utc()==relayNumber);
      //assert(rca.payloadLength()==rca.maxNumberOfPayloadWords());

      //assert(rca.relayNumber()==relayNumber);
      assert(nCfgA==0);
      nCfgA++;
    }

    else if(r.state()==FsmState::Reconfiguring) {
      assert(rcb.valid());
      assert(rcb.utc()>=previousRecordHeader.utc());
      assert(rcb.payloadLength()<=rcb.maxNumberOfPayloadWords());

      //assert(rcb.relayNumber()==relayNumber);
      //assert(rcb.configurationCounter()==(++nCfgB));
      assert(rcb.maxNumberOfRuns()==1);

      nSt=0;
      nSp=0;

      nCfgInRelay++;
      nRunInCfg=0;
      nEvtInCfg=0;
    }
    
    else if(r.state()==FsmState::Configuration) {
      assert(ry.validPattern());      
      //assert(rst.sequenceCounter()==?);
    }
    
    else if(r.state()==FsmState::Starting) {
      assert(rst.valid());
      assert(rst.utc()>=previousRecordHeader.utc());
      //assert(rst.payloadLength()==rst.maxNumberOfPayloadWords());

      //assert(rst.runNumber()==rst.utc());
      assert(nSt==0);
      assert(nSt==nSp);

      runNumber=rst.utc();
      //runNumber=rst.runNumber();
      //maxEvents=rst.maxEvents();
      //maxSeconds=rst.maxSeconds();
      //maxSpills=rst.maxSpills();

      nSt++;

      nPs=0;
      nRs=0;
      
      nRunInRelay++;
      nRunInCfg++;
      nEvtInRun=0;
    }
    
    else if(r.state()==FsmState::Running) {
      assert(rrn.valid());
      //assert(rst.sequenceCounter()==((nEvtInRelay++)/2)+1);

      nEvtInRun++;
    }
	    
    else if(r.state()==FsmState::Pausing) {
      assert(rps.valid());
      assert(rps.utc()>=previousRecordHeader.utc());
      //assert(rps.payloadLength()==rps.maxNumberOfPayloadWords());
      assert(rps.payloadLength()==0);

      assert(nPs==nRs);
      nPs++;
    }
    
    else if(r.state()==FsmState::Resuming) {
      assert(rrs.valid());
      assert(rrs.utc()>=previousRecordHeader.utc());
      //assert(rrs.payloadLength()==rrs.maxNumberOfPayloadWords());
      assert(rrs.payloadLength()==0);
      
      nRs++;
      assert(nPs==nRs);
    }
    
    else if(r.state()==FsmState::Stopping) {
      assert(rsp.valid());
      assert(rsp.utc()>=previousRecordHeader.utc());
      assert(rsp.payloadLength()==rsp.maxNumberOfPayloadWords());

      //assert(rsp.runNumber()==runNumber);
      //assert(rsp.numberOfEvents() <=maxEvents);
      //assert(rsp.numberOfSeconds()<=maxSeconds);
      //assert(rsp.numberOfSpills() <=maxSpills);

      //assert(nPs==rsp.numberOfPauses());
      //assert(nRs==rsp.numberOfPauses());

      //assert(rsp.numberOfEvents()+1==nRunEv);
      if(nRunEv<rsp.numberOfEvents()) {
	std::cerr << "Events dropped = "
		  << rsp.numberOfEvents() << " - "
		  << nRunEv << " = " << rsp.numberOfEvents()+1-nRunEv
		  << std::endl;
	    }

      nSp++;
      assert(nSp==nSt);
      assert(nPs==nRs);
    }

    else if(r.state()==FsmState::Status) { 
      assert(ry.validPattern());      
      //assert(rst.sequenceCounter()==?);
    }
    
    /*
    else if(r.state()==FsmState::HaltingB) {
      assert(rhb.valid());
      assert(rhb.utc()>=previousRecordHeader.utc());
      assert(rhb.relayNumber()==relayNumber);
      assert(rhb.payloadLength()==rhb.maxNumberOfPayloadWords());

      assert(rhb.configurationNumber()==nCfgInRelay);
      assert(rhb.numberOfRuns()==nRunInCfg);
      //assert(rhb.numberOfEvents()==nEvtInCfg);

      assert(rhb.numberOfRuns()==nSt);
      assert(rhb.numberOfRuns()==nSp);
    }
    */

    else if(r.state()==FsmState::Halting) {
      assert(rha.valid());
      assert(rha.utc()>=previousRecordHeader.utc());
      //assert(rha.payloadLength()==rha.maxNumberOfPayloadWords());

      //assert(rha.relayNumber()==relayNumber);
      //assert(rha.numberOfConfigurations()==nCfgInRelay);
      //assert(rha.numberOfRuns()==nRunInRelay);
      //assert(rha.numberOfEvents()==nEvtInRelay);
    }

    else {
      std::cerr << "Unexpected record state" << std::endl;
      std::cout << "Unexpected record state" << std::endl;
      r.print();
    }
    
    previousRecordHeader=(RecordHeader&)r;
  }

  relayReader.close();
  
  // Print out table
  std::cout << std::endl << "Number of records seen by state" << std::endl;
  for(unsigned i(0);i<=FsmState::EndOfStateEnum;i++) {

    // TEMP!!!
    if(((FsmState::State)i)!=FsmState::ConfiguredA &&
       ((FsmState::State)i)!=FsmState::HaltingB) {
      
      if(((FsmState::State)i)==FsmState::EndOfStaticEnum) std::cout << std::endl;
      if(((FsmState::State)i)==FsmState::EndOfTransientEnum) std::cout << std::endl;

      std::cout << " State " << FsmState::stateName((FsmState::State)i)
		<< ", number of records = " << std::setw(10)
		<< nRecord[i] << std::endl;
    }
  }

  return 0;
}
