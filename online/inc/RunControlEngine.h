#ifndef RunControlEngine_h
#define RunControlEngine_h

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
    }

    void setCheckEnable(bool c) {
      _checkEnable=c;
    }

    void setAssertEnable(bool a) {
      _assertEnable=a;
    }

    void add(FsmInterface *p) {
      _allFsmInterface.push_back(p);
      _alive.push_back(false);
    }

    bool coldStart() {
      unsigned n(0);
      std::cout << "Coldstarting " << n << std::endl;
      for(unsigned i(0);i<_allFsmInterface.size();i++) {
	//_allFsmInterface[i]->initialize(); // DONE BY DOWNSTREAM
	_allFsmInterface[i]->print();
      
	_allFsmInterface[i]->ping();
	_allFsmInterface[i]->print();
	sleep(1);
	_allFsmInterface[i]->print();
	_alive[i]=_allFsmInterface[i]->isIdle();

	if(_alive[i]) {
	  _goodFsmInterface.push_back(_allFsmInterface[i]);
	  n++;
	}
      }
      for(unsigned i(0);i<_goodFsmInterface.size() && i<1;i++) {
	/*	
		unsigned timeout(0);
		_goodFsmInterface[i]->commandPacket().setCommand(FsmCommand::Reset);

		_goodFsmInterface[i]->print();
		if(command(_goodFsmInterface[i]->commandPacket())) n++;
		_goodFsmInterface[i]->print();

		assert(_goodFsmInterface[i]->propose());

		while(!_goodFsmInterface[i]->isAccepted() &&
		!_goodFsmInterface[i]->isRejected() &&
		timeout<_timeoutLimit) {
		timeout++;
		usleep(1);
		}

		if(_goodFsmInterface[i]->isAccepted()) n++;
	*/
      }
      std::cout << "Coldstart found " << n << std::endl;
      return true;
    }

    bool command(FsmCommandPacket c) {
      c.print();
      //uint64_t srn(time(0));
      //_goodFsmInterface[i]->setCommandData(1,&srn);
          
      //std::cout << std::endl << "Sending propose" << std::endl;
      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	_goodFsmInterface[i]->setCommandPacket(c);

	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sending propose" << std::endl;
	  _goodFsmInterface[i]->print();
	}

	assert(_goodFsmInterface[i]->propose());

	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " sent propose" << std::endl;
	  _goodFsmInterface[i]->print();
	}
      }

      //std::cout << std::endl << "Waiting for ready" << std::endl;
      std::vector<bool> accepted(_goodFsmInterface.size());
      bool allAccepted(true);

      for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	unsigned timeout(0);
	accepted[i]=false;
      
	if(_printLevel>0) {
	  std::cout << std::endl << "Shm" << i << " waiting for accept" << std::endl;
	  _goodFsmInterface[i]->print();
	}

	while(!_goodFsmInterface[i]->isAccepted() &&
	      !_goodFsmInterface[i]->isRejected() &&
	      timeout<_timeoutLimit) {
	  timeout++;
	  usleep(10);
	}

	accepted[i]=_goodFsmInterface[i]->isAccepted();
	if(!accepted[i]) allAccepted=false;
      
	if(timeout>=_timeoutLimit) {
	  std::cerr << "Shm" << i << " timed out waiting for accepted/rejected" << std::endl;
	} else {
	  assert(accepted[i]!=_goodFsmInterface[i]->isRejected());
	  assert(_goodFsmInterface[i]->matchingStates());
	}
      
	if(_printLevel>0) {
	  if(accepted[i]) {
	    std::cout << std::endl << "Shm" << i << " received accepted" << std::endl;
	  } else if(timeout<_timeoutLimit) {
	    std::cout << std::endl << "Shm" << i << " received rejected" << std::endl;
	  } else {
	    std::cout << std::endl << "Shm" << i << " timed out waiting for accepted/rejected" << std::endl;
	  }
	  _goodFsmInterface[i]->print();
	}
      }

      if(!allAccepted) {
	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " sending repair" << std::endl;
	    _goodFsmInterface[i]->print();
	  }

	  // May fail if timed out???
	  assert(_goodFsmInterface[i]->repair());

	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " sent repair" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	}

	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	  unsigned timeout(0);

	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " waiting for idle" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	
	  while(!_goodFsmInterface[i]->isIdle() &&
		timeout<_timeoutLimit) {
	    timeout++;
	    usleep(10);
	  }
	
	  if(timeout>=_timeoutLimit) {
	    std::cerr << "Shm" << i << " timed out waiting for idle" << std::endl;
	  } else {
	    assert(_goodFsmInterface[i]->matchingStates());
	  }
      
	  // MORE HERE
	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " sent repair" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	}

      } else {
	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " sending change" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	
	  //_goodFsmInterface[i]->changeSystemState();
	  assert(_goodFsmInterface[i]->change());
	
	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " sent change" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	}

	//std::cout << std::endl << "Waiting for changed" << std::endl;
	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	  unsigned timeout(0);
	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " waiting for changed" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	
	  while(!_goodFsmInterface[i]->isChanged() &&
		timeout<1000000) {
	    timeout++;
	    usleep(10);
	  }

	  if(timeout>=_timeoutLimit) {
	    std::cerr << "Shm" << i << " timed out waiting for changed" << std::endl;
	  } else {
	    _goodFsmInterface[i]->print();
	    assert(_goodFsmInterface[i]->matchingStates());
	  }
      
	  // MORE HERE
	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " received changed" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	}

	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " sending startstatic" << std::endl;
	    _goodFsmInterface[i]->print();
	  }

	  // May fail if timed out???
	  assert(_goodFsmInterface[i]->startStatic());

	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " sent startstatic" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	}

	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	  unsigned timeout(0);

	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " waiting for idle" << std::endl;
	    _goodFsmInterface[i]->print();
	  }
	
	  while(!_goodFsmInterface[i]->isIdle() &&
		timeout<_timeoutLimit) {
	    timeout++;
	    usleep(10);
	  }
	
	  if(timeout>=_timeoutLimit) {
	    std::cerr << "Shm" << i << " timed out waiting for idle" << std::endl;
	  } else {
	    assert(_goodFsmInterface[i]->matchingStates());
	    //_goodFsmInterface[i]->commandPacket().initialize();
	  }
      
	  // MORE HERE
	  if(_printLevel>0) {
	    std::cout << std::endl << "Shm" << i << " back in Static" << std::endl;
	    _goodFsmInterface[i]->commandPacket().resetRecord();
	    _goodFsmInterface[i]->print();
	  }
	}
      }

      /*
	std::cout << std::endl << "Checking transients" << std::endl;
	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) {
	std::cout << "Shm" << i << " sending propose" << std::endl;
	_goodFsmInterface[i]->print();
	}
	std::cout << "Proc" << i << ": ";_goodFsmInterface[i]->print();
	//while(!_goodFsmInterface[i]->matchingStates());
	std::cout << "Proc" << i << ": ";_goodFsmInterface[i]->print();
        }
	}
	std::cout << std::endl << "Going back to static" << std::endl;
	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) {
	std::cout << "Shm" << i << " sending propose" << std::endl;
	_goodFsmInterface[i]->print();
	}
	std::cout << "Proc" << i << ": ";_goodFsmInterface[i]->print();
	assert(_goodFsmInterface[i]->startStatic());
	std::cout << "Proc" << i << ": ";_goodFsmInterface[i]->print();
        }
	}
	std::cout << std::endl << "Waiting for statics" << std::endl;
	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	if(_printLevel>0) {
	std::cout << "Shm" << i << " sending propose" << std::endl;
	_goodFsmInterface[i]->print();
	}
	std::cout << "Proc" << i << ": ";_goodFsmInterface[i]->print();
	while(!_goodFsmInterface[i]->isStaticState());
	std::cout << "Proc" << i << ": ";_goodFsmInterface[i]->print();
        }
	}
	std::cout << std::endl << "Checking statics" << std::endl;
	if(_printLevel>0) {
	std::cout << "Shm" << i << " sending propose" << std::endl;
	_goodFsmInterface[i]->print();
	}
	for(unsigned i(0);i<_goodFsmInterface.size();i++) {
	std::cout << "Proc" << i << ": ";_goodFsmInterface[i]->print();
	while(!_goodFsmInterface[i]->matchingStates());
	std::cout << "Proc" << i << ": ";_goodFsmInterface[i]->print();
        }
	}
      */
      return true;
    }
    /*
      bool reset(unsigned n) {
    
      goodFsmInterface.resize(1);
      std::vector<bool> good(allFsmInterface.size());
    
      for(unsigned i(0);i<allFsmInterface.size();i++) {
      goodFsmInterface.back()=allFsmInterface[i];
      if(command(RunControlFsmEnums::Reset)) {
      }
      }
      }
  
      void ColdStart() {
      _writePtr=0;
      _readPtr=0;
      }

      bool write(uint16_t n, const uint64_t *p) {
      if(_writePtr==_readPtr+BufferDepth) return false;
      //std::memcpy(_buffer[_writePtr%BufferDepth],p,8*n);
      std::memcpy(_buffer[_writePtr&BufferDepthMask],p,8*n);
      _writePtr++;
      return true;
      }

      uint16_t read(uint64_t *p) {
      if(_writePtr==_readPtr) return 0;
      RecordHeader *h((RecordHeader*)_buffer[_readPtr&BufferDepthMask]);
      uint16_t n(h->totalLength());
      std::memcpy(p,h,8*n);
      _readPtr++;
      return n;
      }
  
      void print(std::ostream &o=std::cout) {
      o << "RunControlEngineT<" << PowerOfTwo << "," << Width << ">::print()" << std::endl;
      o << " Write pointer to memory  = " << std::setw(10) << _writePtr << std::endl
      << " Read pointer from memory = " << std::setw(10) << _readPtr << std::endl;    
      uint32_t diff(_writePtr>_readPtr?_writePtr-_readPtr:0);
      o << " Difference               = " << std::setw(10) << diff << std::endl;
      }
    */
    
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
