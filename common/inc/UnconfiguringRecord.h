#ifndef Hgcal10gLinkReceiver_UnconfiguringRecord_h
#define Hgcal10gLinkReceiver_UnconfiguringRecord_h

#include <iostream>
#include <iomanip>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class UnconfiguringRecord : public Record<1> {
  
  public:
    UnconfiguringRecord() {
    }
    
    void setHeader(uint32_t t=time(0)) {
      setIdentifier(Hgcal10gLinkReceiver::RecordHeader::StateData);
      setState(RunControlFsmEnums::Unconfiguring);
      setLength();
      setUtc(t);
    }
    /*
    uint32_t superRunNumber() const {
      return _payload[0]&0xffffffff;
    }

    void setSuperRunNumber(uint32_t t=time(0)) {
      _payload[0]&=0;//0xffffffff00000000; Nothing in top half yet
      _payload[0]|=t;
    }
    */
    void print(std::ostream &o=std::cout, std::string s="") {
      o << s << "UnconfiguringRecord::print()" << std::endl;
      RecordHeader::print(o,s+" ");
      
      for(unsigned i(0);i<length();i++) {
	o << s << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(16) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
    }
  private:
  };

}

#endif
