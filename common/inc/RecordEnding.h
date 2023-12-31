#ifndef Hgcal10gLinkReceiver_RecordEnding_h
#define Hgcal10gLinkReceiver_RecordEnding_h

#include <iostream>
#include <iomanip>
#include <cassert>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordEnding : public Record {
  
  public:
    RecordEnding() {
    }
    
    bool valid() const {
      return validPattern() && state()==FsmState::Ending;
    }
    
    void setHeader(uint32_t t=time(0)) {
      setState(FsmState::Ending);
      setPayloadLength(0);
      setUtc(t);
    }

    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "RecordEnding::print()" << std::endl;
      RecordHeader::print(o,s+" ");
    }
    
  private:
  };

}

#endif
