#ifndef Hgcal10gLinkReceiver_RunControlCommand_h
#define Hgcal10gLinkReceiver_RunControlCommand_h

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <cstring>

#include "RunControlFsmEnums.h"

namespace Hgcal10gLinkReceiver {

  class RunControlCommand {

  public:
    enum {
      CommandDataBufferSize=1024
    };

    RunControlCommand() {
    }
    
    void initialize() {
      _systemState=RunControlFsmEnums::EndOfStateEnum;
      _command=RunControlFsmEnums::EndOfCommandEnum;
      _commandDataSize=0;
    }

    // Get information about command and its data
    RunControlFsmEnums::Command command() const {
      return _command;
    }
  
    uint32_t commandDataSize() const {
      return _commandDataSize;
    }
  
    const uint64_t* commandDataBuffer() const {
      return _commandDataBuffer;
    }

    uint16_t commandDataBuffer(uint64_t *d) const {
      std::memcpy(d,_commandDataBuffer,sizeof(uint64_t)*_commandDataSize);
      return _commandDataSize;
    }
    
    // Get information about state and error level
    RunControlFsmEnums::State systemState() const {
      return _systemState;
    }

    // Set command and its data
    void resetCommandCounter() {
      _commandCounter=0;
    }

    void setCommand(RunControlFsmEnums::Command r) {
      _command=r;
      //_systemState=transitionStateForCommand(r);
      _commandCounter++;
    }

    void changeSystemState() {
      if(RunControlFsmEnums::staticState(_systemState)) {
	_systemState=transitionStateForCommand(_command);
      } else {
	_systemState=staticStateAfterCommand(_command);
      }
    }
    
    void resetCommandData() {
      _commandDataSize=0;
    }
  
    bool setCommandData(uint16_t s, uint64_t *d) {
      if(s>CommandDataBufferSize) return false;
      _commandDataSize=s;
      std::memcpy(_commandDataBuffer,d,sizeof(uint64_t)*s);
      return true;
    }
  
    // Set the system state
    void setSystemState() {
      _systemState=staticStateAfterCommand(_command);
    }
    
    void setSystemState(RunControlFsmEnums::State s) {
      _systemState=s;
    }

    // Conversion of enum values to human-readable strings
    const std::string& commandName() const {
      return RunControlFsmEnums::commandName(_command);
    }
  
    const std::string& systemStateName() const {
      return RunControlFsmEnums::stateName(_systemState);
    }

    // Used for displaying results
    void print(std::ostream &o=std::cout) const {
      o << "RunControlCommand::print()" << std::endl;
      o << " System state = " << systemStateName()
	<< ", Command = " << commandName()
	<< ", counter = " << std::setw(10) << _commandCounter
	<< ", data size = " << _commandDataSize  << std::endl;
      /*
      for(unsigned i(0);i<std::min(_commandDataSize,uint32_t(CommandDataBufferSize));i++) {
	o << "  Buffer word " << std::setw(2) << i << " = 0x"
	  << std::hex << std::setfill('0')
	  << _commandDataBuffer[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
      o << std::endl;
      */
    }
  
  private:

    // System state
    RunControlFsmEnums::State _systemState;

    // Command
    RunControlFsmEnums::Command _command;

    // Command counter
    uint32_t _commandCounter;
    
    // Command data
    uint32_t _commandDataSize;
    uint64_t _commandDataBuffer[CommandDataBufferSize];
  };

}

#endif
