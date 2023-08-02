#ifndef Hgcal10gLinkReceiver_RunControlEngine_h
#define Hgcal10gLinkReceiver_RunControlEngine_h

#include <iostream>
#include <iomanip>

#include "FsmState.h"
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
      unsigned ntimeout;
      unsigned n(0);
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) std::cout << "Coldstarting " << i << std::endl;

	if(_printLevel>0) _goodFsmInterface[i]->print();

	//if(_goodFsmInterface[i]->systemState() ==FsmState::Initial &&
	// _goodFsmInterface[i]->processState()==FsmState::Initial &&
	// _goodFsmInterface[i]->handshake()==FsmInterface::Idle)
	
	//_goodFsmInterface[i]->ping();
	//_goodFsmInterface[i]->setProcessState(FsmState::Shutdown);
	_goodFsmInterface[i]->getRecord().reset(FsmState::Initial);
	_goodFsmInterface[i]->getRecord().setUtc();

	if(_printLevel>0) _goodFsmInterface[i]->print();

	ntimeout=0;
	while(_goodFsmInterface[i]->processState()!=FsmState::Initial && ntimeout<(i==7?60:3)) {
	  usleep(1000000);
	  ntimeout++;
	//sleep(2);
	}
	std::cout << i << " Timeout = " << ntimeout << std::endl;
	if(_printLevel>0) _goodFsmInterface[i]->print();

	//usleep(1000);
	//if(_printLevel>0) _goodFsmInterface[i]->print();
	_alive[i]=(_goodFsmInterface[i]->processState()==FsmState::Initial);

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

      std::cout << "Vector formed of " << _goodFsmInterface.size() << " processors" << std::endl;

      return true;
    }

    //bool command(FsmState::State s, uint32_t number=time(0), uint32_t key=0) {
    bool command(const Record *r) {
      if(!FsmState::transientState(r->state())) return false;
      
      //std::cout << std::endl << "Sending Prepare" << std::endl;
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	RecordT<15> h;
	h.deepCopy(r);
	h.setUtc(0);
	//_goodFsmInterface[i]->setPrepareRecord(r->state());

	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sending pre-Transient"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}

	_goodFsmInterface[i]->setRecord(h);
	//assert(_goodFsmInterface[i]->prepare());

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
	  std::cout << std::endl << "Shm" << i << " waiting for Continuing"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}

	//while(!_goodFsmInterface[i]->isReady() &&
	while(_goodFsmInterface[i]->processState()!=FsmState::Continuing &&
	      timeout<_timeoutLimit) {
	  timeout++;
	  usleep(10);
	}

	if(timeout>=_timeoutLimit) {
	  std::cerr << "Shm" << i << " timed out waiting for Continuing"
		    << std::endl;
	  allReady=false;
	}
      
	if(_printLevel>0) {
	  if(timeout<_timeoutLimit) {
	    std::cout << std::endl << "Shm" << i << " received Continuing" << std::endl;
	  } else {
	    std::cout << std::endl << "Shm" << i << " timed out waiting for Continuing"
		      << std::endl;
	  }
	  _goodFsmInterface[i]->print();
	}
      }

      if(!allReady) return false;
      
      //std::cout << std::endl << "Sending change to transient" << std::endl;
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sending Transient" << std::endl;
	  _goodFsmInterface[i]->print();
	}
	
	_goodFsmInterface[i]->setGoToRecord(r);
	//assert(_goodFsmInterface[i]->goToTransient());
	
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sent Transient" << std::endl;
	  _goodFsmInterface[i]->print();
	}
      }
	
      //std::cout << std::endl << "Waiting for Completed" << std::endl;
      bool allCompleted(true);
      
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	unsigned timeout(0);
	
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " waiting for Transient state change" << std::endl;
	  _goodFsmInterface[i]->print();
	}
	
	//while(!_goodFsmInterface[i]->isCompleted() &&
	while(_goodFsmInterface[i]->processState()!=_goodFsmInterface[i]->systemState() &&
	      timeout<1000000) {
	  timeout++;
	  usleep(10);
	}
	
	if(timeout>=_timeoutLimit) {
	  std::cerr << "Shm" << i << " timed out waiting for Transient state change" << std::endl;
	  allCompleted=false;
	}
	
	if(_printLevel>0) {
	  if(timeout<_timeoutLimit) {
	    std::cout << std::endl << "Shm" << i << " received Transient state change" << std::endl;
	  } else {
	    std::cout << std::endl << "Shm" << i << " timed out waiting for Transient state change"
		      << std::endl;
	  }
	  _goodFsmInterface[i]->print();
	}
      }

      if(!allCompleted) return false;

      //std::cout << std::endl << "Sending change to static" << std::endl;
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sending Static"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}
	
	RecordT<15> rr;
	rr.deepCopy(r);
	rr.setState(FsmState::staticStateAfterTransient(r->state()));
	_goodFsmInterface[i]->setGoToRecord(&rr);
	//assert(_goodFsmInterface[i]->goToStatic());
	
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
	  std::cout << std::endl << "Shm" << i << " waiting for Static state change"
		    << std::endl;
	  _goodFsmInterface[i]->print();
	}
	
	//while(!_goodFsmInterface[i]->isIdle() &&
	while(_goodFsmInterface[i]->processState()!=_goodFsmInterface[i]->systemState() &&
	      timeout<_timeoutLimit) {
	  timeout++;
	  usleep(10);
	}
	
	if(timeout>=_timeoutLimit) {
	  std::cerr << "Shm" << i << " timed out waiting for Static state change" << std::endl;
	  allIdle=false;
	}
	  
	// MORE HERE
	if(_printLevel>0) {
	  if(timeout<_timeoutLimit) {
	    std::cout << std::endl << "Shm" << i << " received Static state change" << std::endl;
	  } else {
	    std::cout << std::endl << "Shm" << i << " timed out waiting for Static state change"
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
