#ifndef Hgcal10gLinkReceiver_RecordHaltingA_h
#define Hgcal10gLinkReceiver_RecordHaltingA_h

#include <iostream>
#include <iomanip>
#include <cassert>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordHaltingA : public RecordT<1> {
  
  public:
    RecordHaltingA() {
    }
    
    void setHeader(uint32_t t=time(0)) {
      setState(FsmState::HaltingA);
      setPayloadLength(1);
      setUtc(t);
    }

    uint32_t superRunNumber() const {
      return _payload[0]&0xffffffff;
    }

    uint32_t numberOfConfigurations() const {
      return _payload[0]>>32;
    }
    /*
    uint32_t numberOfRuns() const {
      return _payload[1]&0xffffffff;
    }
    */
    void setSuperRunNumber(uint32_t t=time(0)) {
      _payload[0]&=0xffffffff00000000;
      _payload[0]|=t;
    }
   
    void setNumberOfConfigurations(uint32_t c) {
      _payload[0]&=0x00000000ffffffff;
      _payload[0]|=uint64_t(c)<<32;
    }
    /* 
    void setNumberOfRuns(uint32_t n) {
      _payload[1]&=0xffffffff00000000;
      _payload[1]|=n;
    }
    */
    void print(std::ostream &o=std::cout, std::string s="") {
      o << s << "RecordHaltingA::print()" << std::endl;
      RecordHeader::print(o,s+" ");
      
      for(unsigned i(0);i<payloadLength();i++) {
	o << s << "  Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(16) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
      o << s << " SuperRun number          = "
	<< std::setw(10) << superRunNumber() << std::endl;
      o << s << " Number of configurations = "
	<< std::setw(10) << numberOfConfigurations() << std::endl;
      //o << s << " Number of runs           = "
      //<< std::setw(10) << numberOfRuns() << std::endl;
    }
    
  private:
  };

}

#endif
