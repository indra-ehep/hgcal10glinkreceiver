#ifndef FsmInterface_h
#define FsmInterface_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cassert>

#include "FsmCommandPacket.h"

namespace Hgcal10gLinkReceiver {
  // INHERIT???

  
  class FsmInterface {

  public:
    enum HandshakeState {
      Ping,
      Idle,
      Propose,
      Accepted,
      Rejected,
      Change,
      Repair,
      Changed,
      Repaired,
      StartStatic,
      Request,
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

    FsmInterface() {
      initialize();
    }

    void initialize() {
      _systemState=FsmState::Initial;
      _commandPacket.initialize();
      _commandHandshake=Idle;

      _processState=FsmState::Initial;
      _requestPacket.initialize();
      _requestHandshake=Idle;
    }

    // Run Controller: normal operation ///////////////

    // Ping to check if alive

    bool ping() {
      if(_commandHandshake!=Idle) return false;
      _commandHandshake=Ping;
      return true;
    }

    bool isIdle() {
      return _commandHandshake==Idle;
    }
    
    // Prepare to start a command sequence

    bool propose() {
      if(_commandHandshake!=Idle) return false;
      _commandHandshake=Propose;
      return true;
    }

    // Check if ready to do next step

    bool isAccepted() const {
      return _commandHandshake==Accepted;
    }

    bool isRejected() const {
      return _commandHandshake==Rejected;
    }

    // Change if ready or repair if rejected

    bool change() {
      if(_commandHandshake!=Accepted) return false;
      assert(matchingStates());
      changeSystemState();
      _commandHandshake=Change;
      return true;
    }

    bool repair() {
      if(_commandHandshake!=Rejected) return false;
      assert(matchingStates());
      _commandHandshake=Repair;
      return true;
    }
    
    // Check if completed or repaired
    
    bool isChanged() const {
      return _commandHandshake==Changed;
    }
    
    bool isRepaired() const {
      return _commandHandshake==Repaired;
    }

    // Finish up sequence
    
    bool startStatic() {
      if(_commandHandshake==Changed) {
	assert(matchingStates());
	changeSystemState();
	//_commandPacket.setNextState();
	_commandHandshake=StartStatic;
	return true;

      } else if(_commandHandshake==Repaired) {
	assert(matchingStates());
	_commandHandshake=StartStatic;
	return true;
      }
      
      return false;
    }

    // Request start
    bool isPropose() const {
      return _requestHandshake==Propose;
    }    

    // Request complete
    bool idle() {
      return _requestHandshake==Idle;
    }
    
    // Processor: normal operation ///////////////

    // Check if action required

    // Respond to ping

    bool pong() {
      if(_commandHandshake!=Ping) return false;
      _commandHandshake=Idle;
      return true;
    }

    // Respond to transition attempt
    
    bool accepted() {
      if(_commandHandshake!=Propose) return false;
      _commandHandshake=Accepted;
      return true;
    }
    
    bool rejected() {
      if(_commandHandshake!=Propose) return false;
      _commandHandshake=Rejected;
      return true;
    }
    
    // Wait to proceed

    bool isChange() const {
      return _commandHandshake==Change;
    }

    bool isRepair() const {
      return _commandHandshake==Repair;
    }

    bool isProceed() const {
      return isChange() || isRepair();
    }
    
    // Indicate completion

    bool completed() {
      if(_commandHandshake==Change) {
	//changeRequestState();
	_commandHandshake=Changed;
	return true;

      } else if(_commandHandshake==Repair) {
	_commandHandshake=Repaired;
	return true;
      }
      
      return false;
    }

    // Wait for change to static state

    bool isStartStatic() const {
      return _commandHandshake==StartStatic;
    }

    bool ended() {
      if(_commandHandshake!=StartStatic) return false;
      _commandHandshake=Idle;
      return true;
    }

    // Request a change???

    bool request() {
      if(_commandHandshake!=Idle) return false;
      _commandHandshake=Request;
      return true;
    }
    
    bool matchingStates() const {
      return _processState==_systemState;
    }
    

    FsmState::State systemState() const {
      return _systemState;
    }
    
    FsmCommandPacket& commandPacket() {
      return _commandPacket;
    }

    HandshakeState commandHandshake() const {
      return _commandHandshake;
    }


    void setCommandPacket(const FsmCommandPacket &p) {
      _commandPacket=p;
    }

    void setCommandHandshake(HandshakeState h) {
      _commandHandshake=h;
    }

    void setRequestPacket(const FsmCommandPacket &p) {
      _requestPacket=p;
    }

    void setRequestHandshake(HandshakeState h) {
      _requestHandshake=h;
    }

    void changeSystemState() {
      if(FsmState::staticState(_systemState)) {
        _systemState=FsmCommand::transitionStateForCommand(_commandPacket.command());
      } else {
        _systemState=FsmCommand::staticStateAfterCommand(_commandPacket.command());
      }
    }

    void changeProcessState() {
      if(FsmState::staticState(_processState)) {
        _processState=FsmCommand::transitionStateForCommand(_commandPacket.command());
      } else {
        _processState=FsmCommand::staticStateAfterCommand(_commandPacket.command());
      }
    }

    
    ////////////////////////////////////////////
    /*    
    // Get information about lock
    bool rcLock() {
      return _commandHandshake==Idle;
    }
    bool commandEnabled() {
      return _commandHandshake==Idle;
    }

    // Get information about command
    FsmCommand::Command command() const {
      return _commandPacket.command();
    }
  
    FsmState:State systemState() const {
      return _systemState;
    }
    
    uint32_t commandDataSize() const {
      return _commandPacket.commandDataSize();
    }
  
    const uint64_t* commandDataBuffer() const {
      return _commandPacket.commandDataBuffer();
    }

    // Get information about state and error level
    */
    FsmState::State processState() const {
      return _processState;
    }
    /*
    FsmState:StateError processError() const {
      return _requestPacket.processError();
    }

    FsmCommand::Command request() const {
      return _requestPacket.request();
    }

    // Override control for who has access
    void setRcLock(bool l=true) {
      assert(false);
      _commandHandshake=Idle;
    }

    // Send or force a request
    void forceCommand(FsmCommand::Command r) {
      _commandPacket.setCommand(r);
      //assert(false);
      //_commandHandshake=Idle;
    }
  
    bool setCommand(FsmCommand::Command r) {
      if(_commandHandshake!=Idle) return false;
      forceCommand(r);
      return true;
    }
  
    bool forceSystemState(FsmState:State s) {
      _commandPacket.setSystemState(s);
      return true;
    }
  
    void setSystemState() {
      _commandPacket.setSystemState();
    }

    bool setSystemState(FsmState:State s) {
      if(_commandHandshake==Idle) return false;
      forceSystemState(s);
      return true;
    }

    void setRequest(FsmCommand::Command r) {
      _requestPacket.setRequest(r);
    }

    void resetCommandData() {
      _commandPacket.resetCommandData();
    }

    bool setCommandData(uint16_t s, uint64_t *d) {
      _commandPacket.setCommandData(s,d);
      return true;
    }
    */
    // Change the process state
    void forceProcessState(FsmState::State s) {
      _processState=s;
      //_commandHandshake==Idle=FsmCommand::staticState(s);
    }
    /*
    bool setProcessState(FsmState:State s) {
      if(_commandHandshake==Idle) return false;
      forceProcessState(s);
      return true;
    }
  
    void forceProcessError(FsmState:StateError e) {
      _requestPacket.setProcessError(e);
    }
  
    bool setProcessError(FsmState:StateError e) {
      if(_requestPacket.processError()>e) return false;
      forceProcessError(e);
      return true;
    }

    bool setProcess(FsmState:State s, FsmState:StateError e) {
      if(!setProcessState(s)) return false;
      _requestPacket.setProcessError(e);
      return true;
    }
    */      
    const std::string& commandName() const {
      return FsmCommand::commandName(_commandPacket.command());
    }
  
    const std::string& systemStateName() const {
      return FsmState::stateName(_systemState);
    }
  
    const std::string& processStateName() const {
      return FsmState::stateName(_processState);
    }
    /*  
    const std::string& processErrorName() const {
      return FsmCommand::stateErrorName(_requestPacket.processError());
    }
    */
    const std::string& requestName() const {
      return FsmCommand::commandName(_requestPacket.command());
    }

    const std::string& commandHandshakeName() const {
      return handshakeName(_commandHandshake);
    }

    const std::string& requestHandshakeName() const {
      return handshakeName(_requestHandshake);
    }
    
    // Used for displaying results
    void print(std::ostream &o=std::cout) const {
      o << "FsmInterface::print()" << std::endl;
      o << " RunControl:" << std::endl;
      o << "  System state = " << systemStateName() << std::endl << " ";
      _commandPacket.print(o);
      o << "  Command handshake = " << commandHandshakeName()
	<< std::endl;
      o << " Processor:" << std::endl;
      o << "  Processor state = " << processStateName() << std::endl << " "
      //_requestPacket.print(o);
      //o << "  Request handshake = " << requestHandshakeName()
	<< std::endl;
    }
  
  private:
    FsmState::State _systemState;
    FsmCommandPacket _commandPacket;
    HandshakeState _commandHandshake;

    FsmState::State _processState;
    FsmCommandPacket _requestPacket;
    HandshakeState _requestHandshake;

    static const uint32_t identifier[EndOfIdentifierEnum];

    static const std::string _unknown;

    static const std::string _handshakeName[EndOfHandshakeStateEnum];

    static const std::string& handshakeName(HandshakeState h) {
      if(h<EndOfHandshakeStateEnum) return _handshakeName[h];
      return _unknown;
    }
  };

  const std::string FsmInterface::_unknown="Unknown";

  const std::string FsmInterface::_handshakeName[EndOfHandshakeStateEnum]={
    "Ping",
    "Idle",
    "Propose",
    "Accepted",
    "Rejected",
    "Change",
    "Repair",
    "Changed",
    "Repaired",
    "Request",
    "StartStatic"
  };

  const uint32_t FsmInterface::identifier[EndOfIdentifierEnum]={
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
