#ifndef Hgcal10gLinkReceiver_EventRecord_h
#define Hgcal10gLinkReceiver_EventRecord_h

#include <iostream>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class EventRecord : public RecordHeader {
  
  public:
    EventRecord() {
    }
    
    void setHeader(uint32_t t=time(0)) {
      setIdentifier(Hgcal10gLinkReceiver::RecordHeader::EventData);
      setFsmState(RunControlFsmShm::Running);
      setUtc(t);
    }

    void print(std::ostream &o=std::cout, std::string s="") {
      o << s << "EventRecord::print()" << std::endl;
      RecordHeader::print(o,s+" ");
      
      for(unsigned i(0);i<length();i++) {
	o << s << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(8) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
    }
  private:
    uint64_t _payload[1];
  };

}

#endif
