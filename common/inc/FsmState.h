#ifndef FsmState_h
#define FsmState_h

#include <string>

namespace Hgcal10gLinkReceiver {

  class FsmState {

  public:
    enum State {
      // Statics
      Initial,
      Halted,
      ConfiguredA,
      ConfiguredB,
      Running,
      Paused,
      EndOfStaticEnum,
      
      // Transients
      Initializing=EndOfStaticEnum,
      ConfiguringA,
      ConfiguringB,
      Starting,
      Pausing,
      Resuming,
      Stopping,
      HaltingB,
      HaltingA,
      Resetting,
      EndOfStateEnum
    };

    FsmState() {
    }
    
    FsmState(State s) {
      _state=s;
    }
    
    State state() const {
      return _state;
    }

    void setState(State s) {
      _state=s;
    }

    bool staticState() const {
      return _state<EndOfStaticEnum;
    }
    
    const std::string& stateName() const {
      return stateName(_state);
    }

    bool allowedChange(State s) {
      return allowedChange(_state,s);
    }
    
    // Utilities
    static bool staticState(State s) {
      return s<EndOfStaticEnum;
    }

    static bool allowedChange(State s1, State s2) {

      // Resetting is an exception
      if(staticState(s1) && s2==Resetting) return true;
	
      return	      
	// Static to Transient
	(s1==Initial     && s2==Initializing) ||
	(s1==Halted      && s2==ConfiguringA) ||
	(s1==Halted      && s2==Resetting   ) ||
	(s1==ConfiguredA && s2==ConfiguringB) ||
	(s1==ConfiguredA && s2==HaltingA    ) ||
	(s1==ConfiguredB && s2==Starting    ) ||
	(s1==ConfiguredB && s2==HaltingB    ) ||
	(s1==Running     && s2==Pausing     ) ||	      
	(s1==Running     && s2==Stopping    ) ||
	(s1==Paused      && s2==Resuming    ) ||
      
	// Transient to Static
	(s1==Initializing && s2==Halted     ) ||
	(s1==ConfiguringA && s2==ConfiguredA) ||
	(s1==ConfiguringB && s2==ConfiguredB) ||
	(s1==Starting     && s2==Running    ) ||
	(s1==Pausing      && s2==Paused     ) ||
	(s1==Resuming     && s2==Running    ) ||
	(s1==Stopping     && s2==ConfiguredB) ||
	(s1==HaltingB     && s2==ConfiguredA) ||
	(s1==HaltingA     && s2==Halted     ) ||
	(s1==Resetting    && s2==Initial    );      
    }

    // Conversion of enum values to human-readable strings    
    static const std::string& stateName(State s) {
      if(s<EndOfStateEnum) return _stateName[s];
      return _unknown;
    }
      
  private:
    State _state;
  
    static const std::string _unknown;
    static const std::string _stateName[EndOfStateEnum];
  };
 
  const std::string FsmState::_unknown="Unknown";
    
  const std::string FsmState::_stateName[EndOfStateEnum]={
    // Statics
    "Initial    ",
    "Halted     ",
    "ConfiguredA",
    "ConfiguredB",
    "Running    ",
    "Paused     ",
      
    // Transients
    "Initializing",
    "ConfiguringA",
    "ConfiguringB",
    "Starting    ",
    "Pausing     ",
    "Resuming    ",
    "Stopping    ",
    "HaltingB    ",
    "HaltingA    ",
    "Resetting   "
  };

}
#endif
