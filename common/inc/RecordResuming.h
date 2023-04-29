#ifndef Hgcal10gLinkReceiver_RecordResuming_h
#define Hgcal10gLinkReceiver_RecordResuming_h

#include <iostream>
#include <iomanip>
#include <cassert>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordResuming : public Record {
  
  public:
    RecordResuming() {
    }
    
    void setHeader(uint32_t t=time(0)) {
      setState(FsmState::Resuming);
      setPayloadLength(0);
      setUtc(t);
    }

    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "RecordResuming::print()" << std::endl;
      RecordHeader::print(o,s+" ");
    }
    
  private:
  };

}

#endif
