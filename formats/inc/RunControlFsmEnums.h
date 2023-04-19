#ifndef RunControlFsmEnums_h
#define RunControlFsmEnums_h

#include <string>

namespace Hgcal10gLinkReceiver {

  namespace RunControlFsmEnums {
    
    enum Command {
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
      EndOfCommandEnum
    };
    
    enum {
      CommandDataBufferSize=16
    };
    
    enum State {
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
    
    const std::string _stateName[EndOfStateEnum]={
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
    
    const std::string _stateErrorName[EndOfStateErrorEnum]={
      "Good   ",
      "Warning",
      "Error  "
    };
    
    // Conversion of enum values to human-readable strings
    static const std::string& commandName(Command r) {
      if(r<EndOfCommandEnum) return _commandName[r];
      return _unknown;
    }
    
    static const std::string& stateName(RunControlFsmEnums::State s) {
      if(s<EndOfStateEnum) return _stateName[s];
      return _unknown;
    }
    
    static const std::string& stateErrorName(RunControlFsmEnums::StateError e) {
      if(e<EndOfStateErrorEnum) return _stateErrorName[e];
      return _unknown;
    }
  }
}

#endif
