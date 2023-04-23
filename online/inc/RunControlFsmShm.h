#ifndef RunControlFsmShm_h
#define RunControlFsmShm_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cassert>

#include "RunControlCommand.h"
#include "RunControlResponse.h"

namespace Hgcal10gLinkReceiver {
  // INHERIT???

  
  class RunControlFsmShm {

  public:
    enum HandshakeState {
      Ping,
      Request,
      StaticState,
      Prepare,
      Accepted,
      Rejected,
      Change,
      Repair,
      Changed,
      Repaired,
      StartStatic,
      EndOfHandshakeStateEnum
    };

    enum Identifier {
      Testing,
      Tcds2,
      Frontend,
      FastControl,
      DaqLink0,
      DaqLink1,
      DaqLink2,
      Spare,
      EndOfIdentifierEnum
    };

    RunControlFsmShm() {
      initialize();
    }

    void initialize() {
      _handshakeState=StaticState;
      _command.initialize();
      _response.initialize();
    }

    // Run Controller: normal operation ///////////////

    // Ping to check if alive

    bool ping() {
      if(_handshakeState!=StaticState) return false;
      _handshakeState=Ping;
      return true;
    }

    bool isStaticState() {
      return _handshakeState==StaticState;
    }
    
    // Prepare to start a command sequence

    bool prepare() {
      if(_handshakeState!=StaticState) return false;
      _handshakeState=Prepare;
      return true;
    }

    // Check if ready to do next step

    bool isAccepted() const {
      return _handshakeState==Accepted;
    }

    bool isRejected() const {
      return _handshakeState==Rejected;
    }

    bool isReady() const {
      return isAccepted() || isRejected();
    }
    
    // Change if ready or repair if rejected

    bool change() {
      if(_handshakeState!=Accepted) return false;
      assert(matchingStates());
      _command.changeSystemState();
      _handshakeState=Change;
      return true;
    }

    bool repair() {
      if(_handshakeState!=Rejected) return false;
      assert(matchingStates());
      _handshakeState=Repair;
      return true;
    }

    bool proceed() {
      return change() || repair();
    }
    
    // Check if completed or repaired
    
    bool isChanged() const {
      return _handshakeState==Changed;
    }
    
    bool isRepaired() const {
      return _handshakeState==Repaired;
    }

    bool isCompleted() const {
      return isChanged() || isRepaired();
    }
    
    // Finish up sequence
    
    bool startStatic() {
      if(_handshakeState==Changed) {
	assert(matchingStates());
	_command.changeSystemState();
	//_command.setNextState();
	//_response.setNextState();
	_handshakeState=StartStatic;
	return true;

      } else if(_handshakeState==Repaired) {
	assert(matchingStates());
	_handshakeState=StartStatic;
	return true;
      }
      
      return false;
    }
    
    
    // Processor: normal operation ///////////////

    // Check if action required

    bool isStaticState() const {
      return _handshakeState==StaticState;
    }

    // Respond to ping

    bool pong() {
      if(_handshakeState!=Ping) return false;
      _handshakeState=StaticState;
      return true;
    }

    // Respond to transition attempt
    
    bool accepted() {
      if(_handshakeState!=Prepare) return false;
      _handshakeState=Accepted;
      return true;
    }
    
    bool rejected() {
      if(_handshakeState!=Prepare) return false;
      _handshakeState=Rejected;
      return true;
    }
    
    // Wait to proceed

    bool isChange() const {
      return _handshakeState==Change;
    }

    bool isRepair() const {
      return _handshakeState==Repair;
    }

    bool isProceed() const {
      return isChange() || isRepair();
    }
    
    // Indicate completion

    bool completed() {
      if(_handshakeState==Change) {
	//_response.changeResponseState();
	_handshakeState=Changed;
	return true;

      } else if(_handshakeState==Repair) {
	_handshakeState=Repaired;
	return true;
      }
      
      return false;
    }

    // Wait for change to static state

    bool isStartStatic() const {
      return _handshakeState==StartStatic;
    }

    bool ended() {
      if(_handshakeState!=StartStatic) return false;
      _handshakeState=StaticState;
      return true;
    }

    // Request a change???

    bool request() {
      if(_handshakeState!=StaticState) return false;
      _handshakeState=Request;
      return true;
    }
    
    ////////////////////////////////////////////
    
    // Get information about lock
    bool rcLock() {
      return _handshakeState==StaticState;
    }
    bool commandEnabled() {
      return _handshakeState==StaticState;
    }

    // Get information about command
    RunControlFsmEnums::Command command() const {
      return _command.command();
    }
  
    RunControlFsmEnums::State systemState() const {
      return _command.systemState();
    }
    
    uint32_t commandDataSize() const {
      return _command.commandDataSize();
    }
  
    const uint64_t* commandDataBuffer() const {
      return _command.commandDataBuffer();
    }

    // Get information about state and error level

    RunControlFsmEnums::State processState() const {
      return _response.processState();
    }
  
    RunControlFsmEnums::StateError processError() const {
      return _response.processError();
    }

    RunControlFsmEnums::Command request() const {
      return _response.request();
    }

    bool matchingStates() const {
      return _response.processState()==_command.systemState();
    }
    
    // Override control for who has access
    void setRcLock(bool l=true) {
      assert(false);
      _handshakeState=StaticState;
    }

    // Send or force a request
    void forceCommand(RunControlFsmEnums::Command r) {
      _command.setCommand(r);
      //assert(false);
      //_handshakeState=StaticState;
    }
  
    bool setCommand(RunControlFsmEnums::Command r) {
      if(_handshakeState!=StaticState) return false;
      forceCommand(r);
      return true;
    }
  
    bool forceSystemState(RunControlFsmEnums::State s) {
      _command.setSystemState(s);
      return true;
    }
  
    void setSystemState() {
      _command.setSystemState();
    }

    bool setSystemState(RunControlFsmEnums::State s) {
      if(_handshakeState==StaticState) return false;
      forceSystemState(s);
      return true;
    }

    void setRequest(RunControlFsmEnums::Command r) {
      _response.setRequest(r);
    }

    void resetCommandData() {
      _command.resetCommandData();
    }

    bool setCommandData(uint16_t s, uint64_t *d) {
      _command.setCommandData(s,d);
      return true;
    }

    // Change the process state
    void forceProcessState(RunControlFsmEnums::State s) {
      _response.setProcessState(s);
      //_handshakeState==StaticState=RunControlFsmEnums::staticState(s);
    }
  
    bool setProcessState(RunControlFsmEnums::State s) {
      if(_handshakeState==StaticState) return false;
      forceProcessState(s);
      return true;
    }
  
    void forceProcessError(RunControlFsmEnums::StateError e) {
      _response.setProcessError(e);
    }
  
    bool setProcessError(RunControlFsmEnums::StateError e) {
      if(_response.processError()>e) return false;
      forceProcessError(e);
      return true;
    }
  
    bool setProcess(RunControlFsmEnums::State s, RunControlFsmEnums::StateError e) {
      if(!setProcessState(s)) return false;
      _response.setProcessError(e);
      return true;
    }
    
    const std::string& commandName() const {
      return RunControlFsmEnums::commandName(_command.command());
    }
  
    const std::string& systemStateName() const {
      return RunControlFsmEnums::stateName(_command.systemState());
    }
  
    const std::string& processStateName() const {
      return RunControlFsmEnums::stateName(_response.processState());
    }
  
    const std::string& processErrorName() const {
      return RunControlFsmEnums::stateErrorName(_response.processError());
    }

    const std::string& requestName() const {
      return RunControlFsmEnums::commandName(_response.request());
    }

    // Used for displaying results
    void print(std::ostream &o=std::cout) const {
      o << "RunControlFsmShm::print()" << std::endl;
      _command.print(o);
      _response.print(o);
      o << " Handshake state = " << _handshakeState
	<< std::endl;
    }
  
    static const uint32_t identifier[EndOfIdentifierEnum];

    // Handshake
    HandshakeState _handshakeState;

  private:

    RunControlCommand _command;
    RunControlResponse _response;
  };

  const uint32_t RunControlFsmShm::identifier[EndOfIdentifierEnum]={
    0xce0000, // Testing
    0xce1cd5, // TCDS2
    0xcebefc, // Frontend (via BE)
    0xcebefe, // BE Fast control
    0xceda00, // DAQ link0 receiver
    0xceda01, // DAQ link1 receiver
    0xceda02, // DAQ link2 receiver
    0xcedead  // Spare (movable stage?)
  };
}

#endif
