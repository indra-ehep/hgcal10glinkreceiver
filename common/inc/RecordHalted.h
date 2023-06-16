#ifndef Hgcal10gLinkReceiver_RecordHalted_h
#define Hgcal10gLinkReceiver_RecordHalted_h

#include <iostream>
#include <iomanip>
#include <cassert>

#include "RecordYaml.h"

namespace Hgcal10gLinkReceiver {

  class RecordHalted : public RecordYaml {
  
  public:
    RecordHalted() {
    }
    
    void setHeader(uint32_t c) {
      setState(FsmState::Halted);
      setUtc(c);
      setString("Null");
    }

    bool valid() const {
      return validPattern() && state()==FsmState::Halted;
    }
    
    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "RecordHalted::print()" << std::endl;
      RecordYaml::print(o,s+" ");
    }
    
  private:
  };

}

#endif
