#ifndef Hgcal10gLinkReceiver_RecordPausing_h
#define Hgcal10gLinkReceiver_RecordPausing_h

#include <iostream>
#include <iomanip>
#include <cassert>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordPausing : public Record {
  
  public:
    RecordPausing() {
    }
    
    void setHeader(uint32_t t=time(0)) {
      setState(FsmState::Pausing);
      setPayloadLength(0);
      setUtc(t);
    }

    void print(std::ostream &o=std::cout, std::string s="") {
      o << s << "RecordPausing::print()" << std::endl;
      RecordHeader::print(o,s+" ");
    }
    
  private:
  };

}

#endif
