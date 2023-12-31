#ifndef Hgcal10gLinkReceiver_FileContinuationOpenRecord_h
#define Hgcal10gLinkReceiver_FileContinuationOpenRecord_h

#include <iostream>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class FileContinuationOpenRecord : public RecordT<1> {
  
  public:
    FileContinuationOpenRecord() {
    }
    
    void setHeader(uint32_t t=time(0)) {
      setIdentifier(Hgcal10gLinkReceiver::RecordHeader::StateData);
      setState(FsmState::ConfiguringA);
      setPayloadLength(1);
      setUtc(t);
    }

    uint32_t superRunNumber() const {
      return _payload[0]&0xffffffff;
    }

    void setSuperRunNumber(uint32_t t=time(0)) {
      _payload[0]&=0;//0xffffffff00000000; Nothing in top half yet
      _payload[0]|=t;
    }
   
    void print(std::ostream &o=std::cout, std::string s="") {
      o << s << "FileContinuationOpenRecord::print()" << std::endl;
      RecordHeader::print(o,s+" ");
      
      for(unsigned i(0);i<payloadLength();i++) {
	o << s << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(8) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
	if(i==0) {
	  o << s << "    SuperRun number = "
	    << std::setw(10) << superRunNumber() << std::endl;
	}
      }
    }
  private:
  };

}

#endif
