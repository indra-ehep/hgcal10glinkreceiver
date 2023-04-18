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

  
  RunControlFsmShm() {
    _rcHasLock=true;
    _fsmRequest=Initialize;
    _fsmState=Initial;
    _errorState=Good;

    for(unsigned i(0);i<EndOfFsmStatesEnum;i++) {
      std::cout << std::setw(2) << i << " = "
		<< _fsmStateNames[i] << " is "
		<< (i<EndOfStaticEnum?"Static":"Transitional")
		<< std::endl;
    }
  }
  
  FsmRequests fsmRequest() const {
    return _fsmRequest;
  }
  
  FsmStates fsmState() const {
    return _fsmState;
  }
  
  ErrorStates errorState() const {
    return _errorState;
  }
  
  void setRcLock() {
    _rcHasLock=true;
  }
  
  bool setFsmRequest(FsmRequests r) {
    //if(!_rcHasLock) return false;

    // Some connectivity logic here
    _fsmRequest=r;
    _rcHasLock=false;

    return true;
  }
  
  bool setFsmState(FsmStates s, ErrorStates e) {
    //if(_rcHasLock) return false;

    // Some connectivity logic here
    _fsmState=s;
    _errorState=e;
    _rcHasLock=!_fsmState<EndOfStaticEnum;

    return true;
  }
  
  static const std::string& fsmRequestName(RunControlFsmShm::FsmRequests r) {
    if(r<EndOfFsmRequestsEnum) return _fsmRequestNames[r];
    return _unknown;
  }

  const std::string& fsmRequestName() const {
    return fsmRequestName(_fsmRequest);
  }
  
  const std::string& fsmStateName() const {
    if(_fsmState<EndOfFsmStatesEnum) return _fsmStateNames[_fsmState];
    return _unknown;
  }
  
  const std::string& errorStateName() const {
    if(_errorState<EndOfErrorStatesEnum) return _errorStateNames[_errorState];
    return _unknown;
  }
  
  void print(std::ostream &o=std::cout) const {
    o << "RunControlFsmShm::print()" << std::endl;
    o << " RC request "
      << ( _rcHasLock?"has lock  ":"is blocked")
      << ", request = " << fsmRequestName() << std::endl;
    o << " FSM state  "
      << (!_rcHasLock?"has lock  ":"is blocked") 
      << ", state   = " << fsmStateName() << " = "
      << (_fsmState<EndOfStaticEnum?"Static    ":"Transitional") << std::endl;
    o << " Error state = " << errorStateName() << std::endl;
  }
  
 private:
  bool _rcHasLock;
  FsmRequests _fsmRequest;
  FsmStates   _fsmState;
  ErrorStates _errorState;
  
  //static const bool _fsmStaticState[EndOfFsmStatesEnum];

  static const std::string _unknown;
  static const std::string _fsmRequestNames[EndOfFsmRequestsEnum];
  static const std::string _fsmStateNames[EndOfFsmStatesEnum];
  static const std::string _errorStateNames[EndOfErrorStatesEnum];
};

//const bool RunControlFsmShm::_fsmStaticState[EndOfFsmStatesEnum]={
//  true,false, true,false,false, true,false,false,
//  true,false,false, true,false,false, true
//};

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
  "Initial       ",
  "Halted        ",
  "PreConfigured ",
  "Configured    ",
  "Running       ",
  "Paused        ",

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
