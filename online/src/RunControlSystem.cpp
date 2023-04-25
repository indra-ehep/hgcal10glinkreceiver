#include <iostream>
#include <sstream>
#include <vector>

#include <sys/types.h>
#include <unistd.h>

#include "RunControlFsmShm.h"
#include "ShmKeys.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

void request(RunControlFsmShm *p, RunControlFsmEnums::Command fr, uint32_t s=0) {
  std::cout << "WAIT" << std::endl;
  while(!p->rcLock());
  
  p->resetCommandData();
  //p->forceCommand(fr);
  p->setCommand(fr);
  std::cout << "SLEEP" << std::endl;
  sleep(2);
}




int main(int argc, char *argv[]) {
  uint16_t bits(1);
  unsigned nBits(1);
  bool isBit[8]={true,false,false,false,false,false,false,false};
  
  if(argc>1) {
    std::istringstream sin(argv[argc-1]);
    sin >> bits;

    nBits=0;
    for(unsigned i(0);i<8;i++) {
      if((bits&(1<<i))!=0) {
	nBits++;
	isBit[i]=true;
      } else {
	isBit[i]=false;
      }
    }
    
    std::cout << "Bits = 0x"
	      << std::hex << std::setfill('0')
	      << std::setw(2) << unsigned(bits)
	      << std::dec << std::setfill(' ')
	      << ", nbits = " << nBits
	      << std::endl;
  }
  
  //std::vector< ShmSingleton<RunControlFsmShm> > vShmSingleton(nBits);
  std::vector< ShmSingleton<RunControlFsmShm> > vShmSingleton(6);
  std::vector<RunControlFsmShm*> vPtr;
  /*
    unsigned j(0);
    for(unsigned i(0);i<8;i++) {
    if(isBit[i]) {
    vShmSingleton[j].setup(1234560+i);
    vPtr.push_back(vShmSingleton[j].payload());
    std::cout << j << " with key "
    << std::hex << std::setfill('0')
    << std::setw(2) << 1234560+i
    << std::dec << std::setfill(' ')
    << std::endl;
    j++;
    }
    }
    assert(j==nBits);
  */
  
  vShmSingleton[0].setup(RunControlTestingShmKey);
  vShmSingleton[1].setup(RunControlFastControlShmKey);
  vShmSingleton[2].setup(RunControlTcds2ShmKey);
  vShmSingleton[3].setup(RunControlDaqLink0ShmKey);
  vShmSingleton[4].setup(RunControlDaqLink1ShmKey);
  vShmSingleton[5].setup(RunControlDaqLink2ShmKey);

  for(unsigned i(0);i<vShmSingleton.size();i++) {
    vPtr.push_back(vShmSingleton[i].payload());
  }

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
      return 0;
    }
