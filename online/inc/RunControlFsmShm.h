#ifndef RunControlFsmShm_h
#define RunControlFsmShm_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>

#include "RunControlFsmEnums.h"

namespace Hgcal10gLinkReceiver {

  class RunControlFsmShm {

  public:
    enum {
      CommandDataBufferSize=1024
    };

    RunControlFsmShm() {
      _rcHasLock=true;
      _command=RunControlFsmEnums::EndOfCommandEnum;
      _commandDataSize=0;
      _state=RunControlFsmEnums::EndOfStateEnum;
      _stateError=RunControlFsmEnums::Good;
      _request=RunControlFsmEnums::EndOfCommandEnum;
    }

    // Get information about lock
    bool rcLock() const {
      return _rcHasLock;
    }

    // Get information about command and its data
    RunControlFsmEnums::Command command() const {
      return _command;
    }
  
    uint32_t commandDataSize() const {
      return _commandDataSize;
    }
  
    uint64_t* commandDataBuffer() {
      return _commandDataBuffer;
    }

    // Get information about state and error level
    RunControlFsmEnums::State state() const {
      return _state;
    }
  
    bool staticState() const {
      return _state<RunControlFsmEnums::EndOfStaticEnum;
    }
  
    RunControlFsmEnums::StateError stateError() const {
      return _stateError;
    }

    // Override control for who has access
    void setRcLock(bool l=true) {
      _rcHasLock=l;
    }

    // Send or force a request
    void forceCommand(RunControlFsmEnums::Command r) {
      _command=r;

      //_header.setIdentifier(RecordHeader::StateData);
      //_header.setTransition(RunControlFsmEnums::Initializing);
      //_header.setUtc();

      _rcHasLock=false;
    }
  
    bool setCommand(RunControlFsmEnums::Command r) {
      if(!_rcHasLock) return false;
      forceCommand(r);
      return true;
    }
  
    bool setCommandDataSize(uint16_t s) {
      if(s>CommandDataBufferSize) return false;
      _commandDataSize=s;
      //_header.setLength(s);
      return true;
    }
  
    // Change the state
    void forceState(RunControlFsmEnums::State s) {
      _state=s;
      _rcHasLock=staticState();
    }
  
    bool setState(RunControlFsmEnums::State s) {
      if(_rcHasLock) return false;
      forceState(s);
      return true;
    }
  
    void forceErrorState(RunControlFsmEnums::StateError e) {
      _stateError=e;
    }
  
    bool setErrorState(RunControlFsmEnums::StateError e) {
      if(_stateError<e) {
	_stateError=e;
	return true;
      }
      return false;
    }
  
    bool setState(RunControlFsmEnums::State s, RunControlFsmEnums::StateError e) {
      if(!setState(s)) return false;
      _stateError=e;
      return true;
    }

    // Conversion of enum values to human-readable strings
    /*
      static const std::string& commandName(RunControlShm::Command r) {
      if(r<EndOfCommandEnum) return _commandNames[r];
      return _unknown;
      }

      static const std::string& stateName(RunControlShm::State s) {
      if(s<EndOfStateEnum) return _stateNames[s];
      return _unknown;
      }

      static const std::string& stateErrorName(RunControlShm::ErrorStates e) {
      if(e<EndOfErrorStatesEnum) return _stateErrorNames[e];
      return _unknown;
      }
    */
    const std::string& commandName() const {
      return RunControlFsmEnums::commandName(_command);
    }
  
    const std::string& stateName() const {
      return RunControlFsmEnums::stateName(_state);
    }
  
    const std::string& stateErrorName() const {
      return RunControlFsmEnums::stateErrorName(_stateError);
    }

    const std::string& requestName() const {
      return RunControlFsmEnums::commandName(_request);
    }

    // Setting requests with data
    /*
      bool setPreConfigure(PreConfigureCommandData &d) {
      _commandDataSize=(sizeof(PreConfigureCommandData)+7)/8;
      //_commandDataBuffer;
      return setCommand(PreConfigure);
      }
    */
    // Used for displaying results
    void print(std::ostream &o=std::cout) const {
      o << "RunControlFsmShm::print()" << std::endl;
      o << " RC request "
	<< ( _rcHasLock?"has lock  ":"is blocked")
	<< ", request = " << commandName()
	<< ", data size = " << _commandDataSize  << std::endl;

      for(unsigned i(0);i<std::min(_commandDataSize,uint32_t(CommandDataBufferSize));i++) {
	o << "  Buffer word " << std::setw(2) << i << " = 0x"
	  << std::hex << std::setfill('0')
	  << _commandDataBuffer[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
      o << std::endl;
    
      o << " FSM state  "
	<< (!_rcHasLock?"has lock  ":"is blocked") 
	<< ", state   = " << stateName() << " = "
	<< (staticState()?"Static    ":"Transitional") << std::endl;
      o << " Error state = " << stateErrorName() << std::endl;
      o << " Request = " << requestName() << std::endl;
    }
  
  private:

    // Handshake
    bool _rcHasLock;

    // Command
    RunControlFsmEnums::Command _command;

    // State
    RunControlFsmEnums::State   _state;
    RunControlFsmEnums::StateError _stateError;
    RunControlFsmEnums::Command _request;

    // Command data
    uint32_t _commandDataSize;
    //RecordHeader _header;
    uint64_t _commandDataBuffer[CommandDataBufferSize];
  };

}

#endif
