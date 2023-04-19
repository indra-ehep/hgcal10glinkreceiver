#ifndef RunControlFsmShm_h
#define RunControlFsmShm_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cassert>
#include <vector>

class RunControlFsmShm {

 public:
  enum FsmRequests {
    Initialize,
    ColdReset,
    Reset,
    Shutdown,
    PreConfigure,
    Halt,
    Configure,
    Unconfigure,
    Start,
    Stop,
    Pause,
    Resume,
    EndOfFsmRequestsEnum
  };

  enum {
    FsmRequestDataBufferSize=16
  };
  
  enum FsmStates {
    Initial,
    Halted,
    PreConfigured,
    Configured,
    Running,
    Paused,
    EndOfStaticEnum,
    
    Initializing=EndOfStaticEnum,
    PreConfiguring,
    Configuring,
    Starting,
    Pausing,
    Resuming,
    Stopping,
    Unconfiguring,
    Halting,
    EndOfFsmStatesEnum
  };

  enum ErrorStates {
    Good,
    Warning,
    Error,
    EndOfErrorStatesEnum
  };

  // Set defaults in contructor  
  RunControlFsmShm() {
    _rcHasLock=true;
    //_fsmRequest=Initialize;
    _fsmRequest=EndOfFsmRequestsEnum;
    _fsmRequestDataSize=0;
    _fsmState=Initial;
    _errorState=Good;
  }

  // Get information about request and its data
  FsmRequests fsmRequest() const {
    return _fsmRequest;
  }
  
  uint32_t fsmRequestDataSize() const {
    return _fsmRequestDataSize;
  }
  
  uint64_t* fsmRequestDataBuffer() {
    return _fsmRequestDataBuffer;
  }
  
  // Get information about state and error level
  FsmStates fsmState() const {
    return _fsmState;
  }
  
  bool fsmStaticState() const {
    return _fsmState<EndOfStaticEnum;
  }
  
  ErrorStates errorState() const {
    return _errorState;
  }

  // Control who has access
  void setRcLock() {
    _rcHasLock=true;
  }

  // Send or force a request
  void forceFsmRequest(FsmRequests r) {
    _fsmRequest=r;
    _rcHasLock=false;
  }
  
  bool setFsmRequest(FsmRequests r) {
    if(!_rcHasLock) return false;
    forceFsmReset(r);
    return true;
  }
  
  // Change the state
  void forceFsmState(FsmStates s) {
    _fsmState=s;
    _rcHasLock=fsmStaticState();
  }
  
  bool setFsmState(FsmStates s) {
    if(_rcHasLock) return false;
    forceFsmState(s);
    return true;
  }
  
  bool setFsmState(FsmStates s, ErrorStates e) {
    if(!setFsmState(s)) return false;
    _errorState=e;
    return true;
  }

  // Conversion of enum values to human-readable strings
  static const std::string& fsmRequestName(RunControlFsmShm::FsmRequests r) {
    if(r<EndOfFsmRequestsEnum) return _fsmRequestNames[r];
    return _unknown;
  }

  static const std::string& fsmStateName(RunControlFsmShm::FsmStates s) {
    if(r<EndOfFsmStatesEnum) return _fsmStateNames[s];
    return _unknown;
  }

  static const std::string& errorStateName(RunControlFsmShm::ErrorStates e) {
    if(r<EndOfErrorStatesEnum) return _fsmErrorStateNames[e];
    return _unknown;
  }

  const std::string& fsmRequestName() const {
    return fsmRequestName(_fsmRequest);
  }
  
  const std::string& fsmStateName() const {
    return fstStateName(_fsmState);
  }
  
  const std::string& errorStateName() const {
    return errorStateName(_errorState);
  }

  // Used for displaying results
  void print(std::ostream &o=std::cout) const {
    o << "RunControlFsmShm::print()" << std::endl;
    o << " RC request "
      << ( _rcHasLock?"has lock  ":"is blocked")
      << ", request = " << fsmRequestName()
      << ", data size = " << _fsmRequestDataSize  << std::endl;

    for(unsigned i(0);i<std::min(_fsmRequestDataSize,uint32_t(FsmRequestBufferSize));i++) {
      o << "  Buffer word " << std::setw(2) << i << " = 0x"
	<< std::hex << std::setfill('0')
	<< _fsmRequestDataBuffer[i]
	<< std::dec << std::setfill(' ') << std::endl;
    }
    o << std::endl;
    
    o << " FSM state  "
      << (!_rcHasLock?"has lock  ":"is blocked") 
      << ", state   = " << fsmStateName() << " = "
      << (_fsmState<EndOfStaticEnum?"Static    ":"Transitional") << std::endl;
    o << " Error state = " << errorStateName() << std::endl;
  }
  
 private:

  // Handshake
  bool _rcHasLock;

  // Request
  FsmRequests _fsmRequest;
  uint32_t _fsmRequestDataSize;
  uint64_t _fsmRequestDataBuffer[FsmRequestDataBufferSize];

  // State
  FsmStates   _fsmState;
  ErrorStates _errorState;

  // Human-readable names
  static const std::string _unknown;
  static const std::string _fsmRequestNames[EndOfFsmRequestsEnum];
  static const std::string _fsmStateNames[EndOfFsmStatesEnum];
  static const std::string _errorStateNames[EndOfErrorStatesEnum];
};

const std::string RunControlFsmShm::_unknown="Unknown";

const std::string RunControlFsmShm::_fsmRequestNames[EndOfFsmRequestsEnum]={
    "Initialize  ",
    "ColdReset   ",
    "Reset       ",
    "Shutdown    ",
    "PreConfigure",
    "Halt        ",
    "Configure   ",
    "Unconfigure ",
    "Start       ",
    "Stop        ",
    "Pause       ",
    "Resume      "
};

const std::string RunControlFsmShm::_fsmStateNames[EndOfFsmStatesEnum]={
  // Statics
  "Initial       ",
  "Halted        ",
  "PreConfigured ",
  "Configured    ",
  "Running       ",
  "Paused        ",

  // Transitionals
  "Initializing  ",
  "PreConfiguring",
  "Configuring   ",
  "Starting      ",
  "Pausing       ",
  "Resuming      ",
  "Stopping      ",
  "Unconfiguring ",
  "Halting       ",
};

const std::string RunControlFsmShm::_errorStateNames[EndOfErrorStatesEnum]={
  "Good   ",
  "Warning",
  "Error  "
};

#endif
