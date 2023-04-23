#ifndef RunControlResponse_h
#define RunControlResponse_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

#include "RunControlFsmEnums.h"

namespace Hgcal10gLinkReceiver {

  class RunControlResponse {

  public:
    RunControlResponse() {
      initialize();
    }

    void initialize() {
      _processState=RunControlFsmEnums::EndOfStateEnum;
      _processError=RunControlFsmEnums::Good;
      _request=RunControlFsmEnums::EndOfCommandEnum;
    }

    // Get information about state and error level
    RunControlFsmEnums::State processState() const {
      return _processState;
    }
  
    RunControlFsmEnums::StateError processError() const {
      return _processError;
    }

    RunControlFsmEnums::Command request() const {
      return _request;
    }

    // Change the state
    void resetResponseCounter() {
      _responseCounter=0;
    }
    
    void setProcessState(RunControlFsmEnums::State s) {
      _processState=s;
      _responseCounter++;
    }
  
    void setProcessError(RunControlFsmEnums::StateError e) {
      _processError=e;
    }
    
    void setProcess(RunControlFsmEnums::State s,
		    RunControlFsmEnums::StateError e) {
      _processState=s;
      _processError=e;
      _responseCounter++;
    }

    void setRequest(RunControlFsmEnums::Command c) {
      _request=c;
    }

    // Conversion of enum values to human-readable strings
    const std::string& processStateName() const {
      return RunControlFsmEnums::stateName(_processState);
    }
  
    const std::string& processErrorName() const {
      return RunControlFsmEnums::stateErrorName(_processError);
    }

    const std::string& requestName() const {
      return RunControlFsmEnums::commandName(_request);
    }

    // Used for displaying results
    void print(std::ostream &o=std::cout) const {
      o << "RunControlResponse::print()" << std::endl;
      o << " Process state = " << processStateName()
	//<< (staticState()?"Static    ":"Transitional")
	<< std::endl;
      o << " Error state = " << processErrorName() << std::endl;
      o << " Request = " << requestName()
	<< ", counter = " << std::setw(10) << _responseCounter << std::endl;
    }
  
  private:

    // State
    RunControlFsmEnums::State   _processState;
    RunControlFsmEnums::StateError _processError;

    // Request
    RunControlFsmEnums::Command _request;

    // Response counter
    uint32_t _responseCounter;
  };

}

#endif
