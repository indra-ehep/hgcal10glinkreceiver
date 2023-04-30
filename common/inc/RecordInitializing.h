#ifndef Hgcal10gLinkReceiver_RecordInitializing_h
#define Hgcal10gLinkReceiver_RecordInitializing_h

#include <iostream>
#include <iomanip>
#include <cassert>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordInitializing : public Record {
  
  public:
    RecordInitializing() {
      setHeader();
    }
    
    void setHeader(uint32_t t=time(0)) {
      setState(FsmState::Initializing);
      setPayloadLength(0);
      setUtc(t);
    }

    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "RecordInitializing::print()" << std::endl;
      RecordHeader::print(o,s+" ");
    }
    
  private:
  };

}

#endif
