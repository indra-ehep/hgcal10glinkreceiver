#ifndef Hgcal10gLinkReceiver_RecordResetting_h
#define Hgcal10gLinkReceiver_RecordResetting_h

#include <iostream>
#include <iomanip>
#include <cassert>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordResetting : public Record {
  
  public:
    RecordResetting() {
    }
    
    void setHeader(uint32_t t=time(0)) {
      setState(FsmState::Resetting);
      setPayloadLength(0);
      setUtc(t);
    }

    void print(std::ostream &o=std::cout, std::string s="") {
      o << s << "RecordResetting::print()" << std::endl;
      RecordHeader::print(o,s+" ");
    }
    
  private:
  };

}

#endif
