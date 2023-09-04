#include <iostream>
#include <sstream>
#include <vector>

#include <sys/types.h>
//#include <signal.h>
#include <csignal>
#include <unistd.h>

#include <yaml-cpp/yaml.h>

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
  
  std::vector< ShmSingleton<FsmInterface> > vShmSingleton(9);
  std::vector<FsmInterface*> vPtr;
  
  vShmSingleton[0].setup(RunControlDummyFsmShmKey);
  vShmSingleton[1].setup(RunControlFrontEndFsmShmKey);
  vShmSingleton[2].setup(RunControlFastControlFsmShmKey);
  vShmSingleton[3].setup(RunControlTcds2FsmShmKey);
  vShmSingleton[4].setup(RunControlRelayFsmShmKey);
  vShmSingleton[5].setup(RunControlStageFsmShmKey);
  vShmSingleton[6].setup(RunControlDaqLink0FsmShmKey);
  vShmSingleton[7].setup(RunControlDaqLink1FsmShmKey);
  vShmSingleton[8].setup(RunControlDaqLink2FsmShmKey);

  RunControlEngine engine;
  engine.setPrintEnable(  printEnable);
  engine.setCheckEnable(  checkEnable);
  engine.setAssertEnable(assertEnable);

  for(unsigned i(0);i<vShmSingleton.size();i++) {
    vPtr.push_back(vShmSingleton[i].payload());
    engine.add(vShmSingleton[i].payload());
  }

  engine.coldStart();

  //FsmCommandPacket fcp;
  Record r;

  r.reset(FsmState::Initializing);
  r.setPayloadLength(0);
  r.setUtc();
  //fcp.setCommand(FsmCommand::Initialize);
  //fcp.setRecord(r);
  //fcp.print();

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
      //fcp.setCommand(FsmCommand::Reset);
      //fcp.setRecord(rr);
      //fcp.print();
      engine.command(&rr);
      
      if(x=='x') {
	return 0;
	
      }	else if(x=='r') {
	RecordInitializing ri;
	ri.setHeader();
	//fcp.setCommand(FsmCommand::Initialize);
	//fcp.setRecord(ri);
	//fcp.print();
	engine.command(&ri);

      } else {
	RecordEnding re;
	re.setHeader();
	//fcp.setCommand(FsmCommand::End);
	//fcp.setRecord(re);
	//fcp.print();
	engine.command(&re);
	
	continueLoop=false;
      }
      
    } else {
      unsigned nx(999999);
      unsigned numberOfSecs(1000000000);
      //while(nx==0 || nx>1000) {
      while(nx>38 && 
	    nx!=123 && nx!=124 && nx!=125 && nx!=126 && nx!=127 && nx!=128 &&
	    nx!=131 && nx!=132 && nx!=133 && nx!=134 && nx!=135 &&
	    nx!=200 && nx!=201 && nx!=202 && nx!=203 && nx!=204 && nx!=205 && nx!=209 &&
	    nx!=996 && nx!=997 && nx!=998 && nx!=999) {
	//std::cout << "Relay number of runs"
	std::cout << "Relay type (0-38,123,124,125,126,127,128,131,132,133,134,135,200,201,202,203,204,205,209,997,998,999)"
		<< std::endl;
	std::cin >> nx;
      }

      while(numberOfSecs>1000001) {
	std::cout << "Time per run (secs)"
		<< std::endl;
	std::cin >> numberOfSecs;
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

      RecordConfiguring rca;
      rca.setHeader();

      //if(x=='y') rca.setRelayNumber();
      //else rca.setRelayNumber(0xffffffff);
      uint32_t srNumber(x=='y'?uint32_t(time(0)):0xffffffff);

      YAML::Node nRca;
      nRca["RelayNumber"]=srNumber;

      nRca["RunType"]="Default";
      unsigned maxNumberOfConfigurations(1);

      if(nx==  0) maxNumberOfConfigurations=1;
      if(nx>0 && nx<38) maxNumberOfConfigurations=8;
      if(nx== 38) maxNumberOfConfigurations=8*37;
      if(nx==123) maxNumberOfConfigurations=2;
      if(nx==124) maxNumberOfConfigurations=4;
      if(nx==125) {
	nRca["RunType"]="L1aBxScan";
	maxNumberOfConfigurations=64;
      }
      if(nx==126) {
	nRca["RunType"]="BxResetCheck";
	maxNumberOfConfigurations=1;
      }
      if(nx==127) {
	nRca["RunType"]="BeGenericTest";
	maxNumberOfConfigurations=1;
      }
      if(nx==128) {
	nRca["RunType"]="Pedestal";
	maxNumberOfConfigurations=1;
      }

      if(nx==131) {
	nRca["RunType"]="HgcrocBufferTest";
	maxNumberOfConfigurations=4;
      }
      if(nx==132) {
	nRca["RunType"]="CalPulseIntTimeScan";
	maxNumberOfConfigurations=70;
      }
      if(nx==133) {
	nRca["RunType"]="BeamRam1TimeScan";
	maxNumberOfConfigurations=80;
      }
      if(nx==134) {
	nRca["RunType"]="BeamRam1TimeScan2";
	maxNumberOfConfigurations=1000;
      }
      if(nx==135) {
	nRca["RunType"]="BeamScintTimeScan";
	maxNumberOfConfigurations=50;
      }

      if(nx==200) {
	nRca["RunType"]="NoTriggerTest";
	maxNumberOfConfigurations=1;
      }
      if(nx==201) {
	nRca["RunType"]="PhysicsTriggerTest";
	maxNumberOfConfigurations=1;
      }
      if(nx==202) {
	nRca["RunType"]="RandomTriggerTest";
	nRca["RandomRateKhz"]=100.0;
	maxNumberOfConfigurations=1;
      }
      if(nx==203) {
	nRca["RunType"]="SoftwareTriggerTest";
	maxNumberOfConfigurations=1;
      }
      if(nx==204) {
	nRca["RunType"]="RegularTriggerTest";
	maxNumberOfConfigurations=1;
      }
      if(nx==205) {
	nRca["RunType"]="MixedTriggerTest";
	maxNumberOfConfigurations=1;
      }
      if(nx==209) {
	nRca["RunType"]="DisabledTriggerTest";
	maxNumberOfConfigurations=1;
      }

      if(nx==996) {
	nRca["RunType"]="FakeBeamRun";
	maxNumberOfConfigurations=1000000;
      }

      if(nx==997) {
	nRca["RunType"]="EcontTriggerBeamRun";
	maxNumberOfConfigurations=1000000;
      }

      if(nx==998) {
	nRca["RunType"]="BeamAndRandomRun";
	maxNumberOfConfigurations=1000000;
      }

      if(nx==999) {
	nRca["RunType"]="ElectronBeamRun";
	maxNumberOfConfigurations=1000000;
      }

      //rca.setMaxNumberOfConfigurations(maxNumberOfConfigurations);

      nRca["MaxNumberOfConfigurations"]=maxNumberOfConfigurations;

      // Configuration
      /*
      rca.setProcessKey(RunControlDummyFsmShmKey,nx);
      rca.setProcessKey(RunControlFrontEndFsmShmKey,nx);
      rca.setProcessKey(RunControlFastControlFsmShmKey,0);
      rca.setProcessKey(RunControlTcds2FsmShmKey,nx);
      rca.setProcessKey(RunControlRelayFsmShmKey,0);
      rca.setProcessKey(RunControlStageFsmShmKey,0);
      rca.setProcessKey(RunControlDaqLink0FsmShmKey,0);
      rca.setProcessKey(RunControlDaqLink1FsmShmKey,0);
      */
      
      nRca["ProcessorKey"]=nx;

      std::ostringstream sRca;
      sRca << nRca;
      rca.setString(sRca.str());

      //fcp.setCommand(FsmCommand::ConfigureA);
      //fcp.setRecord(rca);
      std::cout  << std::endl << "HEREA!" << std::endl << std::endl;
      rca.print();
      //fcp.print();
      engine.command(&rca);

      for(nc=0;nc<maxNumberOfConfigurations && continueRelay;nc++) {
	if(nc>0) {
	  RecordReconfiguring rcb;
	  rcb.setHeader();
	  rcb.setRelayNumber(srNumber);
	  rcb.setConfigurationCounter(nc+1);
	  rcb.setMaxNumberOfRuns(1);
	  
	  // Configuration
	  if(nx==123) rcb.setProcessKey(RunControlTcds2FsmShmKey,20+(nc%80));
	  
	  //fcp.setCommand(FsmCommand::ConfigureB);
	  //fcp.setRecord(rcb);
	  std::cout  << std::endl << "HEREB!" << std::endl << std::endl;
	  rcb.print();
	  //fcp.print();
	  engine.command(&rcb);
	}

	for(nr=0;nr<1 && continueRelay;nr++) {
	  RecordStarting rsa;
	  rsa.setHeader();

	  //rsa.setMaxEvents(1000000000);
	  //rsa.setMaxSeconds(nx==0?1000000000:60);
	  //rsa.setMaxSeconds(numberOfSecs);
	  //rsa.setMaxSpills(0);
      
	  YAML::Node nRsa;
	  nRsa["RunNumber"]=(x=='y'?uint32_t(time(0)):0xffffffff);

	  std::ostringstream sRsa;
	  sRsa << nRsa;
	  rsa.setString(sRsa.str());
	  
	  //if(x=='y') rsa.setRunNumber();
	  //else rsa.setRunNumber(0xffffffff);

	  //fcp.setCommand(FsmCommand::Start);
	  //fcp.setRecord(rsa);
	  std::cout  << std::endl << "HERES!" << std::endl << std::endl;
	  rsa.print();
	  //fcp.print();
	  engine.command(&rsa);

	  signal(SIGINT,RunControlSignalHandler);

	  unsigned np(999);
	  //unsigned np(0);
	  uint32_t dt(time(0)-rsa.utc());
	  while(dt<numberOfSecs && continueRelay) {
	    usleep(1000);
	    dt=time(0)-rsa.utc();

	    if((dt==4 && np==0) ||
	       (dt==7 && np==1)) {

	      RecordPausing rp;
	      rp.setHeader();
	      //fcp.setCommand(FsmCommand::Pause);
	      //fcp.setRecord(rp);
	      std::cout  << std::endl << "HEREP!" << std::endl << std::endl;
	      rp.print();
	      //fcp.print();
	      engine.command(&rp);

	      RecordResuming rr;
	      rr.setHeader();
	      //fcp.setCommand(FsmCommand::Resume);
	      //fcp.setRecord(rr);
	      std::cout  << std::endl << "HEREP!" << std::endl << std::endl;
	      rr.print();
	      //fcp.print();
	      engine.command(&rr);

	      np++;
	    }
	  }

	  signal(SIGINT,SIG_DFL);


	  RecordStopping rsb;
	  rsb.setHeader();
	  //rsb.setRunNumber(rsa.runNumber());
	  rsb.setNumberOfEvents(0xffffffff);
	  rsb.setNumberOfSeconds(rsb.utc()-rsa.utc());
	  rsb.setNumberOfSpills(0);
	  rsb.setNumberOfPauses(np);
      
	  //fcp.setCommand(FsmCommand::Stop);
	  //fcp.setRecord(rsb);
	  std::cout  << std::endl << "HERES!" << std::endl << std::endl;
	  rsb.print();
	  //fcp.print();
	  engine.command(&rsb);
	}

	nrt+=nr;
	/*	
	RecordHaltingB rhb;
	rhb.setHeader();
	rhb.setRelayNumber(srNumber);
	rhb.setConfigurationNumber(nc+1);
	rhb.setNumberOfRuns(nr);
	rhb.setNumberOfEvents(0xffffffff);
    
	//fcp.setCommand(FsmCommand::HaltB);
	//fcp.setRecord(rhb);
	std::cout  << std::endl << "HEREB!" << std::endl << std::endl;
	rhb.print();
	//fcp.print();
	engine.command(&rhb);
	*/
      }
    
      RecordHalting rha;
      rha.setHeader();
      rha.setRelayNumber(srNumber);
      rha.setNumberOfConfigurations(nc);
      rha.setNumberOfRuns(nrt);
      rha.setNumberOfEvents(0xffffffff);
  
      //fcp.setCommand(FsmCommand::HaltA);
      //fcp.setRecord(rha);
      std::cout  << std::endl << "HEREA!" << std::endl << std::endl;
      rha.print();
      //fcp.print();
      engine.command(&rha);
    }
  }

  return 0;
}
