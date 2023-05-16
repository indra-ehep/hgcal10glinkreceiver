#ifndef Hgcal10gLinkReceiver_RunControlEngine_h
#define Hgcal10gLinkReceiver_RunControlEngine_h

#include <iostream>
#include <iomanip>

#include "FsmCommandPacket.h"
#include "ShmSingleton.h"
#include "RecordPrinter.h"

namespace Hgcal10gLinkReceiver {

  class RunControlEngine {
  public:

    RunControlEngine() {
      _timeoutLimit=1000000;
    }

    void setPrintEnable(bool p) {
      _printEnable=p;
      _printLevel=(p?1:0);
    }

    void setCheckEnable(bool c) {
      _checkEnable=c;
    }

    void setAssertEnable(bool a) {
      _assertEnable=a;
    }

    void add(FsmInterface *p) {
      _goodFsmInterface.push_back(p);
      _alive.push_back(false);
    }

    bool coldStart() {
      unsigned n(0);
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) std::cout << "Coldstarting " << i << std::endl;

	if(_printLevel>0) _goodFsmInterface[i]->print();

	//if(_goodFsmInterface[i]->systemState() ==FsmState::Initial &&
	// _goodFsmInterface[i]->processState()==FsmState::Initial &&
	// _goodFsmInterface[i]->handshake()==FsmInterface::Idle)
	
	_goodFsmInterface[i]->ping();
	if(_printLevel>0) _goodFsmInterface[i]->print();
	usleep(200000);
	//usleep(1000);
	//if(_printLevel>0) _goodFsmInterface[i]->print();
	_alive[i]=_goodFsmInterface[i]->isIdle();

	if(_alive[i]) {
	  //_goodFsmInterface.push_back(_goodFsmInterface[i]);
	  if(_printLevel>0) std::cout << "Found " << i << " alive" << std::endl;
	  n++;
	}
      }

      std::cout << "Coldstart found " << n << " processors alive" << std::endl;

      std::vector<FsmInterface*> temp(_goodFsmInterface);
      _goodFsmInterface.resize(0);
      for(unsigned i(0);i<temp.size();i++) {
	if(_alive[i]) _goodFsmInterface.push_back(temp[i]);
      }

      std::cout << "Vecotr formed of " << _goodFsmInterface.size() << " processors" << std::endl;

      return true;
    }

    bool command(FsmState::State s, uint32_t number=time(0), uint32_t key=0) {
      if(!FsmState::transientState(s)) return false;
      
      //std::cout << std::endl << "Sending Prepare" << std::endl;
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	_goodFsmInterface[i]->setPrepareRecord(s);

	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sending Prepare"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}

	assert(_goodFsmInterface[i]->prepare());

	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sent Prepare" << std::endl;
	  _goodFsmInterface[i]->print();
	}
      }

      //std::cout << std::endl << "Waiting for Ready" << std::endl;
      bool allReady(true);
      
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	unsigned timeout(0);
      
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " waiting for Ready"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}

	while(!_goodFsmInterface[i]->isReady() &&
	      timeout<_timeoutLimit) {
	  timeout++;
	  usleep(10);
	}

	if(timeout>=_timeoutLimit) {
	  std::cerr << "Shm" << i << " timed out waiting for Ready"
		    << std::endl;
	  allReady=false;
	}
      
	if(_printLevel>0) {
	  if(timeout<_timeoutLimit) {
	    std::cout << std::endl << "Shm" << i << " received Ready" << std::endl;
	  } else {
	    std::cout << std::endl << "Shm" << i << " timed out waiting for Ready"
		      << std::endl;
	  }
	  _goodFsmInterface[i]->print();
	}
      }

      if(!allReady) return false;
      
      //std::cout << std::endl << "Sending change to transient" << std::endl;
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sending GoToTransient" << std::endl;
	  _goodFsmInterface[i]->print();
	}
	
	_goodFsmInterface[i]->setGoToRecord(s,number,key);
	assert(_goodFsmInterface[i]->goToTransient());
	
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sent GoToTransient" << std::endl;
	  _goodFsmInterface[i]->print();
	}
      }
	
      //std::cout << std::endl << "Waiting for Completed" << std::endl;
      bool allCompleted(true);
      
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	unsigned timeout(0);
	
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " waiting for Completed" << std::endl;
	  _goodFsmInterface[i]->print();
	}
	
	while(!_goodFsmInterface[i]->isCompleted() &&
	      timeout<1000000) {
	  timeout++;
	  usleep(10);
	}
	
	if(timeout>=_timeoutLimit) {
	  std::cerr << "Shm" << i << " timed out waiting for Completed" << std::endl;
	  allCompleted=false;
	}
	
	if(_printLevel>0) {
	  if(timeout<_timeoutLimit) {
	    std::cout << std::endl << "Shm" << i << " received Completed" << std::endl;
	  } else {
	    std::cout << std::endl << "Shm" << i << " timed out waiting for Completed"
		      << std::endl;
	  }
	  _goodFsmInterface[i]->print();
	}
      }

      if(!allCompleted) return false;

      //std::cout << std::endl << "Sending change to static" << std::endl;
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sending GoToStatic"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}
	
	_goodFsmInterface[i]->setGoToRecord(FsmState::staticStateAfterTransient(s));
	assert(_goodFsmInterface[i]->goToStatic());
	
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sent GoToStatic"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}
      }
      
      //std::cout << std::endl << "Waiting for Idle" << std::endl;
      bool allIdle(true);

      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	unsigned timeout(0);
	
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " waiting for Idle"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}
	
	while(!_goodFsmInterface[i]->isIdle() &&
	      timeout<_timeoutLimit) {
	  timeout++;
	  usleep(10);
	}
	
	if(timeout>=_timeoutLimit) {
	  std::cerr << "Shm" << i << " timed out waiting for Idle" << std::endl;
	  allIdle=false;
	}
	  
	// MORE HERE
	if(_printLevel>0) {
	  if(timeout<_timeoutLimit) {
	    std::cout << std::endl << "Shm" << i << " received Idle" << std::endl;
	  } else {
	    std::cout << std::endl << "Shm" << i << " timed out waiting for Idle"
		      << std::endl;
	  }
	  _goodFsmInterface[i]->print();
	}
      }
    
      if(!allIdle) return false;

      return true;
    }
    
 private:
  bool _printEnable;
  bool _checkEnable;
  bool _assertEnable;

  std::vector<bool> _alive;
  std::vector<FsmInterface*> _allFsmInterface;
  std::vector<FsmInterface*> _goodFsmInterface;

  unsigned _printLevel;
  unsigned _timeoutLimit;
};

}
#endif
