#include <iostream>
#include <sstream>
#include <vector>

#include <sys/types.h>
//#include <signal.h>
#include <csignal>
#include <unistd.h>

#include "SystemParameters.h"
#include "FsmInterface.h"
#include "ShmSingleton.h"
#include "RunControlEngine.h"
#include "RecordConfiguringA.h"
#include "RecordConfiguringB.h"
#include "RecordStarting.h"
#include "RecordResetting.h"
#include "RecordInitializing.h"

using namespace Hgcal10gLinkReceiver;

bool continueRelay;

void RunControlSignalHandler(int signal) {
  std::cerr << "Process " << getpid() << " received signal " 
	    << signal << std::endl;
  continueRelay=false;
}

int main(int argc, char *argv[]) {

  // Turn off printing
  bool printEnable(false);
  bool checkEnable(false);
  bool assertEnable(false);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-p" ||
       std::string(argv[i])=="--printEnable") {
      std::cout << "Setting printEnable to true" << std::endl;
      printEnable=true;
    }
    
    if(std::string(argv[i])=="-c" ||
       std::string(argv[i])=="--checkEnable") {
      std::cout << "Setting checkEnable to true" << std::endl;
      checkEnable=true;
    }
    
    if(std::string(argv[i])=="-a" ||
       std::string(argv[i])=="--assertEnable") {
      std::cout << "Setting assertEnable to true" << std::endl;
      assertEnable=true;
    }
  }
  
  std::vector< ShmSingleton<FsmInterface> > vShmSingleton(8);
  std::vector<FsmInterface*> vPtr;
  
  vShmSingleton[0].setup(RunControlDummyFsmShmKey);
  vShmSingleton[1].setup(RunControlFrontEndFsmShmKey);
  vShmSingleton[2].setup(RunControlFastControlFsmShmKey);
  vShmSingleton[3].setup(RunControlTcds2FsmShmKey);
  vShmSingleton[4].setup(RunControlRelayFsmShmKey);
  vShmSingleton[5].setup(RunControlStageFsmShmKey);
  vShmSingleton[6].setup(RunControlDaqLink0FsmShmKey);
  vShmSingleton[7].setup(RunControlDaqLink1FsmShmKey);

  RunControlEngine engine;
  engine.setPrintEnable(  printEnable);
  engine.setCheckEnable(  checkEnable);
  engine.setAssertEnable(assertEnable);

  for(unsigned i(0);i<vShmSingleton.size();i++) {
    vPtr.push_back(vShmSingleton[i].payload());
    engine.add(vShmSingleton[i].payload());
  }

  engine.coldStart();

  FsmCommandPacket fcp;
  Record r;

  r.reset(FsmState::Initializing);
  r.setPayloadLength(0);
  r.setUtc();
  fcp.setCommand(FsmCommand::Initialize);
  fcp.setRecord(r);
  fcp.print();

  engine.command(&r);

  bool continueLoop(true);

  while(continueLoop) {
    char x('z');
    while(x!='r' && x!='s' && x!='q' && x!='x') {
      std::cout << "s = Start Relay, r = Reset & Initialize, q = Reset & Shutdown, x=Reset & Exit"
		<< std::endl;
      std::cin >> x;
    }

    if(x=='r' || x=='q' || x=='x') {
      RecordResetting rr;
      rr.setHeader();
      fcp.setCommand(FsmCommand::Reset);
      fcp.setRecord(rr);
      fcp.print();
      engine.command(&rr);
      
      if(x=='x') {
	return 0;
	
      }	else if(x=='r') {
	RecordInitializing ri;
	ri.setHeader();
	fcp.setCommand(FsmCommand::Initialize);
	fcp.setRecord(ri);
	fcp.print();
	engine.command(&ri);

      } else {
	RecordEnding re;
	re.setHeader();
	fcp.setCommand(FsmCommand::End);
	fcp.setRecord(re);
	fcp.print();
	engine.command(&re);
	
	continueLoop=false;
      }
      
    } else {
      unsigned nx(999);
      unsigned nt(1000000000);
      //while(nx==0 || nx>1000) {
      while(nx>38 && nx!=123 && nx!=124) {
	//std::cout << "Relay number of runs"
	std::cout << "Relay type (0-38,123,124)"
		<< std::endl;
	std::cin >> nx;
      }

      while(nt>1000001) {
	std::cout << "Time per run (secs)"
		<< std::endl;
	std::cin >> nt;
      }

      x='z';
      while(x!='y' && x!='n') {
	std::cout << "Write to disk y/n"
		  << std::endl;
	std::cin >> x;
      }
      
      continueRelay=true;

      unsigned nc(0);
      unsigned nr(0);
      unsigned nrt(0);

      RecordConfiguringA rca;
      rca.setHeader();

      if(x=='y') rca.setRelayNumber();
      else rca.setRelayNumber(0xffffffff);
      uint32_t srNumber(rca.relayNumber());

      if(nx==  0) rca.setMaxNumberOfConfigurations( 1);
      if(nx>0 && nx<38) rca.setMaxNumberOfConfigurations(8);
      if(nx== 38) rca.setMaxNumberOfConfigurations(8*37);
      if(nx==123) rca.setMaxNumberOfConfigurations(80);
      if(nx==124) rca.setMaxNumberOfConfigurations( 4);

      // Configuration
      rca.setProcessKey(RunControlDummyFsmShmKey,nx);
      rca.setProcessKey(RunControlFrontEndFsmShmKey,nx);
      rca.setProcessKey(RunControlFastControlFsmShmKey,0);
      rca.setProcessKey(RunControlTcds2FsmShmKey,nx);
      rca.setProcessKey(RunControlRelayFsmShmKey,0);
      rca.setProcessKey(RunControlStageFsmShmKey,0);
      rca.setProcessKey(RunControlDaqLink0FsmShmKey,0);
      rca.setProcessKey(RunControlDaqLink1FsmShmKey,0);

      fcp.setCommand(FsmCommand::ConfigureA);
      fcp.setRecord(rca);
      std::cout  << std::endl << "HEREA!" << std::endl << std::endl;
      rca.print();
      fcp.print();
      engine.command(&rca);

      for(nc=0;nc<rca.maxNumberOfConfigurations() && continueRelay;nc++) {
	RecordConfiguringB rcb;
	rcb.setHeader();
	rcb.setRelayNumber(srNumber);
	rcb.setConfigurationCounter(nc+1);
	rcb.setMaxNumberOfRuns(1);

	// Configuration
	if(nx==123) rcb.setProcessKey(RunControlTcds2FsmShmKey,20+(nc%80));
	
	fcp.setCommand(FsmCommand::ConfigureB);
	fcp.setRecord(rcb);
	std::cout  << std::endl << "HEREB!" << std::endl << std::endl;
	rcb.print();
	fcp.print();
	engine.command(&rcb);

	for(nr=0;nr<rcb.maxNumberOfRuns() && continueRelay;nr++) {
	  RecordStarting rsa;
	  rsa.setHeader();

	  if(x=='y') rsa.setRunNumber();
	  else rsa.setRunNumber(0xffffffff);

	  rsa.setMaxEvents(1000000000);
	  //rsa.setMaxSeconds(nx==0?1000000000:60);
	  rsa.setMaxSeconds(nt);
	  rsa.setMaxSpills(0);
      
	  fcp.setCommand(FsmCommand::Start);
	  fcp.setRecord(rsa);
	  std::cout  << std::endl << "HERES!" << std::endl << std::endl;
	  rsa.print();
	  fcp.print();
	  engine.command(&rsa);

	  signal(SIGINT,RunControlSignalHandler);

	  unsigned np(999);
	  //unsigned np(0);
	  uint32_t dt(time(0)-rsa.utc());
	  while(dt<rsa.maxSeconds() && continueRelay) {
	    usleep(1000);
	    dt=time(0)-rsa.utc();

	    if((dt==4 && np==0) ||
	       (dt==7 && np==1)) {

	      RecordPausing rp;
	      rp.setHeader();
	      fcp.setCommand(FsmCommand::Pause);
	      fcp.setRecord(rp);
	      std::cout  << std::endl << "HEREP!" << std::endl << std::endl;
	      rp.print();
	      fcp.print();
	      engine.command(&rp);

	      RecordResuming rr;
	      rr.setHeader();
	      fcp.setCommand(FsmCommand::Resume);
	      fcp.setRecord(rr);
	      std::cout  << std::endl << "HEREP!" << std::endl << std::endl;
	      rr.print();
	      fcp.print();
	      engine.command(&rr);

	      np++;
	    }
	  }

	  signal(SIGINT,SIG_DFL);


	  RecordStopping rsb;
	  rsb.setHeader();
	  rsb.setRunNumber(rsa.runNumber());
	  rsb.setNumberOfEvents(0xffffffff);
	  rsb.setNumberOfSeconds(rsb.utc()-rsa.utc());
	  rsb.setNumberOfSpills(0);
	  rsb.setNumberOfPauses(np);
      
	  fcp.setCommand(FsmCommand::Stop);
	  fcp.setRecord(rsb);
	  std::cout  << std::endl << "HERES!" << std::endl << std::endl;
	  rsb.print();
	  fcp.print();
	  engine.command(&rsb);
	}

	nrt+=nr;
	
	RecordHaltingB rhb;
	rhb.setHeader();
	rhb.setRelayNumber(srNumber);
	rhb.setConfigurationNumber(nc+1);
	rhb.setNumberOfRuns(nr);
	rhb.setNumberOfEvents(0xffffffff);
    
	fcp.setCommand(FsmCommand::HaltB);
	fcp.setRecord(rhb);
	std::cout  << std::endl << "HEREB!" << std::endl << std::endl;
	rhb.print();
	fcp.print();
	engine.command(&rhb);
      }
    
      RecordHaltingA rha;
      rha.setHeader();
      rha.setRelayNumber(srNumber);
      rha.setNumberOfConfigurations(nc);
      rha.setNumberOfRuns(nrt);
      rha.setNumberOfEvents(0xffffffff);
  
      fcp.setCommand(FsmCommand::HaltA);
      fcp.setRecord(rha);
      std::cout  << std::endl << "HEREA!" << std::endl << std::endl;
      rha.print();
      fcp.print();
      engine.command(&rha);
    }
  }

  return 0;
}
