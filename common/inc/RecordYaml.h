#ifndef Hgcal10gLinkReceiver_RecordYaml_h
#define Hgcal10gLinkReceiver_RecordYaml_h

#include <iostream>
#include <iomanip>
#include <cassert>
#include <unordered_map>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordYaml : public RecordT<4095> {
  
  public:
    RecordYaml() {
    }
    
    void setHeader(uint32_t c) {
      setState(FsmState::Configured);
      setUtc(c);
      setString("Null");
    }
    /*  
    bool valid() const {
      return validPattern() && state()==FsmState::Configured;
    }
    */

    void setString(const std::string &s) {
      unsigned n((s.size()+8)/8);
      _payload[n-1]=0;
      std::memcpy(_payload,s.c_str(),s.size()+1);
      setPayloadLength(n);
    }

    const char* string() const {
      return (const char*)_payload;
    }

    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "RecordYaml::print()" << std::endl;
      RecordHeader::print(o,s+" ");
      /*      
      for(unsigned i(0);i<payloadLength();i++) {
	o << s << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(16) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
      */
      if(payloadLength()>0) {
	std::string st(string());
	o << s << " String contents" << std::endl;
	o << st << std::endl;
      }
    }

  private:
  };

}

#endif
