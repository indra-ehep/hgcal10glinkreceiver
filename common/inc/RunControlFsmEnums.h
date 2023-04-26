#ifndef RunControlFsmEnums_h
#define RunControlFsmEnums_h

#include <string>

namespace Hgcal10gLinkReceiver {

  namespace RunControlFsmEnums {
    
    enum Command {
      Initialize,
      ConfigureA,
      //PreConfigure=ConfigureA, // OLD
      ConfigureB,
      //Configure=ConfigureB, // OLD
      Start,
      Pause,
      Resume,
      Stop,
      HaltB,
      //Unconfigure=HaltB, // OLD
      HaltA,
      //Halt=HaltA, // OLD
      Reset,
      EndOfCommandEnum
    };
    
    enum State {
      Initial,
      Halted,
      ConfiguredA,
      //PreConfigured=ConfiguredA, // OLD
      ConfiguredB,
      //Configured=ConfiguredB, // OLD
      Running,
      Paused,
      EndOfStaticEnum,
      
      Initializing=EndOfStaticEnum,
      ConfiguringA,
      //PreConfiguring=ConfiguringA, // OLD
      ConfiguringB,
      //Configuring=ConfiguringB, // OLD
      Starting,
      Pausing,
      Resuming,
      Stopping,
      HaltingB,
      //Unconfiguring=HaltingB, // OLD
      HaltingA,
      //Halting=HaltingA, // OLD
      Resetting,
      EndOfStateEnum
    };
    
    enum StateError {
      Good,
      Warning,
      Error,
      EndOfStateErrorEnum
    };
    
    const std::string _unknown="Unknown";
    
    const std::string _commandName[EndOfCommandEnum]={
      "Initialize",
      "ConfigureA",
      "ConfigureB",
      "Start     ",
      "Pause     ",
      "Resume    ",
      "Stop      ",
      "HaltB     ",
      "HaltA     ",
      "Reset     "
    };
    
    const std::string _stateName[EndOfStateEnum]={
      // Statics
      "Initial    ",
      "Halted     ",
      "ConfiguredA",
      "ConfiguredB",
      "Running    ",
      "Paused     ",
      
      // Transitionals
      "Initializing",
      "ConfiguringA",
      "ConfiguringB",
      "Starting    ",
      "Pausing     ",
      "Resuming    ",
      "Stopping    ",
      "HaltingB    ",
      "HaltingA    ",
      "Resetting   ",
    };
    
    const std::string _stateErrorName[EndOfStateErrorEnum]={
      "Good   ",
      "Warning",
      "Error  "
    };
    
    // Conversion of enum values to human-readable strings
    const std::string& commandName(Command r) {
      if(r<EndOfCommandEnum) return _commandName[r];
      return _unknown;
    }
    
    const std::string& stateName(RunControlFsmEnums::State s) {
      if(s<EndOfStateEnum) return _stateName[s];
      return _unknown;
    }
    
    const std::string& stateErrorName(RunControlFsmEnums::StateError e) {
      if(e<EndOfStateErrorEnum) return _stateErrorName[e];
      return _unknown;
    }
    
    State staticStateBeforeCommand(Command c) {
      if(c==Initialize) return Initial;
      if(c==ConfigureA) return Halted;
      if(c==ConfigureB) return ConfiguredA;
      if(c==Start     ) return ConfiguredB;
      if(c==Pause     ) return Running;

      if(c==Reset     ) return Halted;
      if(c==HaltA     ) return ConfiguredA;
      if(c==HaltB     ) return ConfiguredB;
      if(c==Stop      ) return Running;
      if(c==Resume    ) return Paused;

      return EndOfStateEnum;
    }
    
    State staticStateAfterCommand(Command c) {
      if(c==Initialize) return Halted;
      if(c==ConfigureA) return ConfiguredA;
      if(c==ConfigureB) return ConfiguredB;
      if(c==Start     ) return Running;
      if(c==Pause     ) return Paused;

      if(c==Reset     ) return Initial;
      if(c==HaltA     ) return Halted;
      if(c==HaltB     ) return ConfiguredA;
      if(c==Stop      ) return ConfiguredB;
      if(c==Resume    ) return Running;

      return EndOfStateEnum;
    }
    
    State transitionStateForCommand(Command c) {
      if(c==Initialize) return Initializing;
      if(c==ConfigureA) return ConfiguringA;
      if(c==ConfigureB) return ConfiguringB;
      if(c==Start     ) return Starting;
      if(c==Pause     ) return Pausing;

      if(c==Reset     ) return Resetting;
      if(c==HaltA     ) return HaltingA;
      if(c==HaltB     ) return HaltingB;
      if(c==Stop      ) return Stopping;
      if(c==Resume    ) return Resuming;

      return EndOfStateEnum;
    }

    bool staticState(State s) {
      return s<RunControlFsmEnums::EndOfStaticEnum;
    }
  }
}

#endif
