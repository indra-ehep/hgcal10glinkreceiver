#ifndef FsmCommand_h
#define FsmCommand_h

#include <string>

#include "FsmState.h"

namespace Hgcal10gLinkReceiver {

  class FsmCommand {
  public:
    enum Command {
      Initialize,
      ConfigureA,
      ConfigureB,
      Start,
      Pause,
      Resume,
      Stop,
      HaltB,
      HaltA,
      Reset,
      End,
      EndOfCommandEnum
    };

    FsmCommand() {
    }
    
    FsmCommand(Command c) {
      _command=c;
    }
    
    Command command() const {
      return _command;
    }

    void setCommand(Command c) {
      _command=c;
    }
  
    const std::string& commandName() const {
      return commandName(_command);
    }

    bool allowedCommand(FsmState::State s) {
      return allowedCommand(s,_command);
    }

    FsmState::State staticStateBeforeCommand() {
      return staticStateBeforeCommand(_command);
    }
    
    FsmState::State transitionStateForCommand() {
      return transitionStateForCommand(_command);
    }

    FsmState::State staticStateAfterCommand() {
      return staticStateAfterCommand(_command);
    }
      
    // Utilities
    
    // Conversion of enum values to human-readable strings
    static const std::string& commandName(Command r) {
      if(r<EndOfCommandEnum) return _commandName[r];
      return _unknown;
    }

    static bool allowedCommand(FsmState::State s, Command c) {
      if(!FsmState::staticState(s)) return false;

      // Reset is an exception
      if(c==Reset) return true;

      return
	(s==FsmState::Initial     && c==End       ) ||
	(s==FsmState::Initial     && c==Initialize) ||
	(s==FsmState::Halted      && c==ConfigureA) ||
	(s==FsmState::Halted      && c==Reset     ) ||
	(s==FsmState::ConfiguredA && c==ConfigureB) ||
	(s==FsmState::ConfiguredA && c==HaltA     ) ||
	(s==FsmState::ConfiguredB && c==Start     ) ||
	(s==FsmState::ConfiguredB && c==HaltB     ) ||
	(s==FsmState::Running     && c==Pause     ) ||
	(s==FsmState::Running     && c==Stop      ) ||
	(s==FsmState::Paused      && c==Resume    );
    }

    static FsmState::State staticStateBeforeCommand(Command c) {
      if(c==Initialize) return FsmState::Initial;
      if(c==ConfigureA) return FsmState::Halted;
      if(c==ConfigureB) return FsmState::ConfiguredA;
      if(c==Start     ) return FsmState::ConfiguredB;
      if(c==Pause     ) return FsmState::Running;

      if(c==End       ) return FsmState::Initial;
      if(c==Reset     ) return FsmState::Halted;
      if(c==HaltA     ) return FsmState::ConfiguredA;
      if(c==HaltB     ) return FsmState::ConfiguredB;
      if(c==Stop      ) return FsmState::Running;
      if(c==Resume    ) return FsmState::Paused;

      return FsmState::EndOfStateEnum;
    }
    
    static FsmState::State staticStateAfterCommand(Command c) {
      if(c==Initialize) return FsmState::Halted;
      if(c==ConfigureA) return FsmState::ConfiguredA;
      if(c==ConfigureB) return FsmState::ConfiguredB;
      if(c==Start     ) return FsmState::Running;
      if(c==Pause     ) return FsmState::Paused;

      if(c==End       ) return FsmState::Shutdown;
      if(c==Reset     ) return FsmState::Initial;
      if(c==HaltA     ) return FsmState::Halted;
      if(c==HaltB     ) return FsmState::ConfiguredA;
      if(c==Stop      ) return FsmState::ConfiguredB;
      if(c==Resume    ) return FsmState::Running;

      return FsmState::EndOfStateEnum;
    }
    
    static FsmState::State transitionStateForCommand(Command c) {
      if(c==Initialize) return FsmState::Initializing;
      if(c==ConfigureA) return FsmState::ConfiguringA;
      if(c==ConfigureB) return FsmState::ConfiguringB;
      if(c==Start     ) return FsmState::Starting;
      if(c==Pause     ) return FsmState::Pausing;

      if(c==End       ) return FsmState::Ending;
      if(c==Reset     ) return FsmState::Resetting;
      if(c==HaltA     ) return FsmState::HaltingA;
      if(c==HaltB     ) return FsmState::HaltingB;
      if(c==Stop      ) return FsmState::Stopping;
      if(c==Resume    ) return FsmState::Resuming;

      return FsmState::EndOfStateEnum;
    }
    
  private:
    Command _command;
    
    static const std::string _unknown;
    static const std::string _commandName[EndOfCommandEnum];
  };

  
  const std::string FsmCommand::_unknown="Unknown";
    
  const std::string FsmCommand::_commandName[EndOfCommandEnum]={
    "Initialize",
    "ConfigureA",
    "ConfigureB",
    "Start     ",
    "Pause     ",
    "Resume    ",
    "Stop      ",
    "HaltB     ",
    "HaltA     ",
    "Reset     ",
    "End       "
  };

}

#endif
