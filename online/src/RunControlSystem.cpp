#include <iostream>
#include <sstream>
#include <vector>

#include <sys/types.h>
//#include <signal.h>
#include <csignal>
#include <unistd.h>

#include "FsmInterface.h"
#include "ShmKeys.h"
#include "ShmSingleton.h"
#include "RunControlEngine.h"
#include "RecordConfiguringA.h"
#include "RecordConfiguringB.h"
#include "RecordStarting.h"
#include "RecordResetting.h"
#include "RecordInitializing.h"

using namespace Hgcal10gLinkReceiver;

bool continueSuperRun;

void RunControlSignalHandler(int signal) {
  std::cerr << "Process " << getpid() << " received signal " 
	    << signal << std::endl;
  continueSuperRun=false;
}



/*
  void request(RunControlFsmShm *p, RunControlFsmEnums::Command fr, uint32_t s=0) {
  std::cout << "WAIT" << std::endl;
  while(!p->rcLock());
  
  p->resetCommandData();
  //p->forceCommand(fr);
  p->setCommand(fr);
  std::cout << "SLEEP" << std::endl;
  sleep(2);
  }
*/



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
  
  std::vector< ShmSingleton<FsmInterface> > vShmSingleton(6);
  std::vector<FsmInterface*> vPtr;
  
  vShmSingleton[0].setup(RunControlDummyFsmShmKey);
  vShmSingleton[1].setup(RunControlFastControlFsmShmKey);
  vShmSingleton[2].setup(RunControlTcds2FsmShmKey);
  vShmSingleton[3].setup(RunControlDaqLink0FsmShmKey);
  vShmSingleton[4].setup(RunControlDaqLink1FsmShmKey);
  vShmSingleton[5].setup(RunControlDaqLink2FsmShmKey);

  RunControlEngine engine;
  engine.setPrintEnable(  printEnable);
  engine.setCheckEnable(  checkEnable);
  engine.setAssertEnable(assertEnable);

  for(unsigned i(0);i<vShmSingleton.size();i++) {
    vPtr.push_back(vShmSingleton[i].payload());
    engine.add(vShmSingleton[i].payload());
  }

  FsmCommandPacket fcp;
  Record r;
  r.setState(FsmState::Initializing);
  r.setPayloadLength(0);
  fcp.setCommand(FsmCommand::Initialize);
  fcp.setRecord(r);
  fcp.print();

  engine.coldStart();
  engine.command(fcp);

  bool continueLoop(true);

  signal(SIGINT,RunControlSignalHandler);

  while(continueLoop) {
    char x('z');
    while(x!='r' && x!='s' && x!='q') {
      std::cout << "s = Start SuperRun, r = Reset & Initialize, q = Reset & Shutdown"
		<< std::endl;
      std::cin >> x;
    }

    if(x=='r' || x=='q') {
      RecordResetting rr;
      rr.setHeader();
      fcp.setCommand(FsmCommand::Reset);
      fcp.setRecord(rr);
      fcp.print();
      engine.command(fcp);

      if(x=='r') {
	RecordInitializing ri;
	ri.setHeader();
	fcp.setCommand(FsmCommand::Initialize);
	fcp.setRecord(ri);
	fcp.print();
	engine.command(fcp);

      } else {
	RecordEnding re;
	re.setHeader();
	fcp.setCommand(FsmCommand::End);
	fcp.setRecord(re);
	fcp.print();
	engine.command(fcp);
	
	continueLoop=false;
      }
      
    } else {
      unsigned nx(0);
      while(nx==0 || nx>1000) {
	std::cout << "SuperRun number of runs"
		<< std::endl;
	std::cin >> nx;
      }

      x='z';
      while(x!='y' && x!='n') {
	std::cout << "Write to disk y/n"
		  << std::endl;
	std::cin >> x;
      }
      
      continueSuperRun=true;

      unsigned nc(0);
      unsigned nr(0);
      unsigned nrt(0);

      RecordConfiguringA rca;
      rca.setHeader();

      if(x=='y') rca.setSuperRunNumber();
      else rca.setSuperRunNumber(0xffffffff);
      uint32_t srNumber(rca.superRunNumber());

      rca.setMaxNumberOfConfigurations(nx);
      rca.setProcessKey(0xce000000,123);

      fcp.setCommand(FsmCommand::ConfigureA);
      fcp.setRecord(rca);
      std::cout  << std::endl << "HEREA!" << std::endl << std::endl;
      rca.print();
      fcp.print();
      engine.command(fcp);

      for(nc=0;nc<rca.maxNumberOfConfigurations() && continueSuperRun;nc++) {
	RecordConfiguringB rcb;
	rcb.setHeader();
	rcb.setProcessKey(0xce000000,456);
	rcb.setSuperRunNumber(srNumber);
	rcb.setConfigurationCounter(nc+1);
	rcb.setMaxNumberOfRuns(1);
    
	fcp.setCommand(FsmCommand::ConfigureB);
	fcp.setRecord(rcb);
	std::cout  << std::endl << "HEREB!" << std::endl << std::endl;
	rcb.print();
	fcp.print();
	engine.command(fcp);

	for(nr=0;nr<rcb.maxNumberOfRuns() && continueSuperRun;nr++) {
	  RecordStarting rsa;
	  rsa.setHeader();

	  if(x=='y') rsa.setRunNumber();
	  else rsa.setRunNumber(0xffffffff);

	  rsa.setMaxEvents(1000000000);
	  rsa.setMaxSeconds(30);
	  rsa.setMaxSpills(0);
      
	  fcp.setCommand(FsmCommand::Start);
	  fcp.setRecord(rsa);
	  std::cout  << std::endl << "HERES!" << std::endl << std::endl;
	  rsa.print();
	  fcp.print();
	  engine.command(fcp);

	  unsigned np(0);
	  uint32_t dt(time(0)-rsa.utc());
	  while(dt<rsa.maxSeconds() && continueSuperRun) {
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
	      engine.command(fcp);

	      RecordResuming rr;
	      rr.setHeader();
	      fcp.setCommand(FsmCommand::Resume);
	      fcp.setRecord(rr);
	      std::cout  << std::endl << "HEREP!" << std::endl << std::endl;
	      rr.print();
	      fcp.print();
	      engine.command(fcp);	  

	      np++;
	    }
	  }

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
	  engine.command(fcp);
	}

	nrt+=nr;
	
	RecordHaltingB rhb;
	rhb.setHeader();
	rhb.setSuperRunNumber(srNumber);
	rhb.setConfigurationNumber(nc+1);
	rhb.setNumberOfRuns(nr);
	rhb.setNumberOfEvents(0xffffffff);
    
	fcp.setCommand(FsmCommand::HaltB);
	fcp.setRecord(rhb);
	std::cout  << std::endl << "HEREB!" << std::endl << std::endl;
	rhb.print();
	fcp.print();
	engine.command(fcp);    
      }
    
      RecordHaltingA rha;
      rha.setHeader();
      rha.setSuperRunNumber(srNumber);
      rha.setNumberOfConfigurations(nc);
      rha.setNumberOfRuns(nrt);
      rha.setNumberOfEvents(0xffffffff);
  
      fcp.setCommand(FsmCommand::HaltA);
      fcp.setRecord(rha);
      std::cout  << std::endl << "HEREA!" << std::endl << std::endl;
      rha.print();
      fcp.print();
      engine.command(fcp);    
    }
  }
  
#ifdef JUNK
  std::cout << std::endl << "Checking for responding processors" << std::endl;
  bool active[100];
  for(unsigned i(0);i<vShmSingleton.size();i++) {
    vPtr[i]->_handshakeState=RunControlFsmShm::StaticState; // HACK!
    vPtr[i]->print();
    active[i]=false;
    if(vPtr[i]->ping()) active[i]=true;
    else {
      std::cout << "SHM " << i << " not in handshake Static state" << std::endl;
      vPtr[i]->print();
    }
  }

  std::cout << "SLEEP" << std::endl;
  vPtr[0]->print();
  sleep(5); // Assume all will have responded by now
  vPtr[0]->print();
  std::cout << "AWAKE" << std::endl;
  
  for(unsigned i(0);i<vShmSingleton.size();i++) {
    if(active[i]) {
      vPtr[i]->print();
      if(!vPtr[i]->isStaticState()) {
	std::cout << "SHM " << i << " did not respond to Ping" << std::endl;
	vPtr[i]->print();
	active[i]=false;
      } else {
	std::cout << "SHM " << i << " responded to Ping" << std::endl;
      }
    }
  }

  std::cout << std::endl << "Checking for well-defined static state" << std::endl;
  bool first(true);
  RunControlFsmEnums::State theState;
  bool matching(true);

  for(unsigned i(0);i<vShmSingleton.size();i++) {
    if(active[i]) {
      if(first) {
	theState=vPtr[i]->processState();
	std::cout << "Proc" << i << " setting first state: ";vPtr[i]->print();
	first=false;
      } else {
	if(theState!=vPtr[i]->processState()) {
	  std::cout << "Proc" << i << " mismatched state: ";vPtr[i]->print();
	  matching=false;
	} else {
	  std::cout << "Proc" << i << " matched state: ";vPtr[i]->print();
	}
      }
    }
  }

  if(first) {
    std::cout << "No active processors" << std::endl;
    return 1;
  }
  
  if(!matching) {
    std::cout << "Mismatch: needs reset" << std::endl;
    //RESET
    return 2;
  }

  if(!RunControlFsmEnums::staticState(theState)) {
    std::cout << "State not static" << std::endl;
    return 3;
  }
    
  std::cout << std::endl << "Forcing system to static state" << std::endl;
  for(unsigned i(0);i<vShmSingleton.size();i++) {
    if(active[i]) {
      vPtr[i]->forceSystemState(theState);
      std::cout << "Proc" << i << ": ";vPtr[i]->print();
    }
  }

  sleep(1);

  
  std::cout << std::endl << "Now starting commands" << std::endl;
  for(unsigned k(0);k<12;k++) {
    //sleep(1);
    std::cout << std::endl;
    for(unsigned j(0);j<10;j++) {
      if((k==0 && j==9) ||
	 (k== 1 && j==0) ||
	 (k>1 && k<11 && j>0 && j<9) ||
	 (k==11 && j==9)) {
	std::cout << std::endl << "Sending prepare" << std::endl;
	for(unsigned i(0);i<vShmSingleton.size();i++) {
	  if(active[i]) {
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();

	    vPtr[i]->setCommand((RunControlFsmEnums::Command)((j+1)%10));
	    if(vPtr[i]->command()==RunControlFsmEnums::ConfigureA) {
	      uint64_t srn(time(0));
	      vPtr[i]->setCommandData(1,&srn);
	    }
	    if(vPtr[i]->command()==RunControlFsmEnums::Start) {
	      uint64_t rn(time(0));
	      vPtr[i]->setCommandData(1,&rn);
	    }
	  
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    assert(vPtr[i]->prepare());
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	  }
	}
	std::cout << std::endl << "Waiting for ready" << std::endl;
	for(unsigned i(0);i<vShmSingleton.size();i++) {
	  if(active[i]) {
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    while(!vPtr[i]->isReady());
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	  }
	}
	std::cout << std::endl << "Sending proceed" << std::endl;
	for(unsigned i(0);i<vShmSingleton.size();i++) {
	  if(active[i]) {
	    //std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    //vPtr[i]->changeSystemState();
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    assert(vPtr[i]->proceed());
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	  }
	}
	std::cout << std::endl << "Waiting for completion" << std::endl;
	for(unsigned i(0);i<vShmSingleton.size();i++) {
	  if(active[i]) {
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    while(!vPtr[i]->isCompleted());
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	  }
	}
	std::cout << std::endl << "Checking transients" << std::endl;
	for(unsigned i(0);i<vShmSingleton.size();i++) {
	  if(active[i]) {
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    //while(!vPtr[i]->matchingStates());
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	  }
	}
	std::cout << std::endl << "Going back to static" << std::endl;
	for(unsigned i(0);i<vShmSingleton.size();i++) {
	  if(active[i]) {
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    assert(vPtr[i]->startStatic());
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	  }
	}
	std::cout << std::endl << "Waiting for statics" << std::endl;
	for(unsigned i(0);i<vShmSingleton.size();i++) {
	  if(active[i]) {
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    while(!vPtr[i]->isStaticState());
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	  }
	}
	std::cout << std::endl << "Checking statics" << std::endl;
	for(unsigned i(0);i<vShmSingleton.size();i++) {
	  if(active[i]) {
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	    while(!vPtr[i]->matchingStates());
	    std::cout << "Proc" << i << ": ";vPtr[i]->print();
	  }
	}
      }
    }
  }  
  std::cout << "Exiting..." << std::endl;
  return 0;

      
  unsigned cx;

  vPtr[0]->print();
  
  RunControlFsmEnums::Command c(RunControlFsmEnums::EndOfCommandEnum);
  
  while(true) {
    vPtr[0]->print();

    std::cout << "WAIT" << std::endl;

    //RunControlFsmEnums::Command req(RunControlFsmEnums::EndOfCommandEnum);

    for(unsigned i(0);i<vShmSingleton.size();i++) {
      while(!vPtr[i]->rcLock());
      std::cout << "Processor " << i << " ready" << std::endl;
      vPtr[i]->print();
      /*
      
	if(req==RunControlFsmEnums::EndOfCommandEnum) {
	if(vPtr[i]->request()!=RunControlFsmEnums::EndOfCommandEnum) {
	req=vPtr[i]->request();
	}
	}
      */

    }

    for(unsigned i(0);i<vShmSingleton.size();i++) {
      vPtr[i]->setSystemState();
      vPtr[i]->print();
    }
  
  
    std::cout << std::endl;
    for(unsigned i(0);i<RunControlFsmEnums::EndOfCommandEnum;i++) {
      std::cout << "Command " << i << " = " << RunControlFsmEnums::commandName((RunControlFsmEnums::Command)i) << std::endl;
    }

    std::cout << std::endl << "Enter next command number:" << std::endl;
    std::cin >> cx;
  
    c=(RunControlFsmEnums::Command)cx;
  
    std::cout << "Command " << cx << " = " << RunControlFsmEnums::commandName(c) << std::endl;
  
    for(unsigned i(0);i<vShmSingleton.size();i++) {
      vPtr[i]->setCommand(c);
    }
  }
#endif
  return 0;
}
