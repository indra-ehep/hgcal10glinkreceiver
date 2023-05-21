#ifndef Hgcal10gLinkReceiver_FsmInterface_h
#define Hgcal10gLinkReceiver_FsmInterface_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cassert>

#include "FsmCommandPacket.h"

namespace Hgcal10gLinkReceiver {
  
  class FsmInterface {

  public:
    enum Handshake {
      Idle,
      Prepare,
      Ready,
      GoToTransient,
      Completed,
      GoToStatic,
      Ping,
      EndOfHandshakeEnum
    };

    FsmInterface() {
      coldStart();
    }

    void coldStart() {
      //_record.reset(FsmState::Initial);
      _record.reset(FsmState::Resetting);
      _handshake=Idle;
      //_processState=FsmState::Initial;
      _processState=FsmState::Resetting;
    }

    // Run Controller: normal operation ///////////////

    // Check if alive

    bool ping() {
      if(_handshake!=Idle) return false;
      _handshake=Ping;
      return true;
    }
    
    // Prepare for a transition sequence

    bool prepare() {
      if(_handshake!=Idle) return false;
      _handshake=Prepare;
      return true;
    }

    // Check if ready to do next step

    bool isReady() const {
      return _handshake==Ready;
    }

    // Change to transient state

    bool goToTransient() {
      if(_handshake!=Ready) return false;
      _handshake=GoToTransient;
      return true;
    }

    // Check if completed
    
    bool isCompleted() const {
      return _handshake==Completed;
    }
    
    // Change to static state
    
    bool goToStatic() {
      if(_handshake!=Completed) return false;
      _handshake=GoToStatic;
      return true;
    }

    // Check if in indefinite static state

    bool isIdle() {
      return _handshake==Idle;
    }
    
    // Processor: normal operation ///////////////

    // Indicate alive

    bool pong() {
      if(_handshake!=Ping) return false;
      _handshake=Idle;
      return true;
    }
    
    // Check if starting new transition

    bool isPrepare() const {
      return _handshake==Prepare;
    }

    // Respond to transition preparation
    
    bool ready() {
      if(_handshake!=Prepare) return false;
      _handshake=Ready;
      return true;
    }
    
    // Wait to change to transient state

    bool isGoToTransient() const {
      return _handshake==GoToTransient;
    }
    
    // Indicate completion

    bool completed() {
      if(_handshake!=GoToTransient) return false;
      _handshake=Completed;
      return true;
    }

    // Wait to change to static state

    bool isGoToStatic() const {
      return _handshake==GoToStatic;
    }

    // Indicate in indefinite static state
    
    bool idle() {
      if(_handshake!=GoToStatic) return false;
      _handshake=Idle;
      return true;
    }

    // Get values
    
    FsmState::State systemState() const {
      return _record.state();
    }
    
    FsmState::State processState() const {
      return _processState;
    }
    
    bool matchingStates() const {
      return _processState==systemState();
    }
    
    const RecordT<15>& record() const {
      return _record;
    }

    Handshake handshake() const {
      return _handshake;
    }

    // Set values
    
    void setPrepareRecord(FsmState::State s) {
      _record.reset(s);
    }

    void setRecord(const Record &r) {
      _record.deepCopy(r);
    }

    RecordT<15>& getRecord() {
      return _record;
    }

    void setGoToRecord(const Record *r) {
      /*
      _record.reset(s);
      _record.setUtc();
      _record.setPayloadLength(1);

      uint32_t *p((uint32_t*)(_record.getPayload()));
      p[0]=key;
      p[1]=number;
      */
      _record.deepCopy(r);
    }

    void changeProcessState() {
      _processState=_record.state();
    }

    void setProcessState(FsmState::State s) {
      _processState=s;
    }

    void forceProcessState(FsmState::State s) {
      _processState=s;
    }
  
    FsmState::State* getProcessState() {
      return &_processState;
    }
  
    const std::string& systemStateName() const {
      return FsmState::stateName(systemState());
    }
  
    const std::string& processStateName() const {
      return FsmState::stateName(_processState);
    }

    const std::string& handshakeName() const {
      return handshakeName(_handshake);
    }

    static const std::string& handshakeName(Handshake h) {
      if(h<EndOfHandshakeEnum) return _handshakeName[h];
      return _unknown;
    }

    // Used for displaying results
    void print(std::ostream &o=std::cout) const {
      o << "FsmInterface::print()" << std::endl;
      o << " System state    = " <<  systemStateName() << std::endl;
      o << " Processor state = " << processStateName() << std::endl;
      o << " Handshake = " << handshakeName() << std::endl;
      _record.print(o);
    }
  
  private:
    RecordT<15> _record;
    FsmState::State _processState;
    Handshake _handshake;

    static const std::string _unknown;
    static const std::string _handshakeName[EndOfHandshakeEnum];
  };

  const std::string FsmInterface::_unknown="Unknown";

  const std::string FsmInterface::_handshakeName[EndOfHandshakeEnum]={
    "Idle",
    "Prepare",
    "Ready",
    "GoToTransient",
    "Completed",
    "GoToStatic",
    "Ping"
  };
}

#endif
