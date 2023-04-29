#ifndef Hgcal10gLinkReceiver_RecordRunning_h
#define Hgcal10gLinkReceiver_RecordRunning_h

#include <iostream>

#include "RecordHeader.h"
#include "SlinkBoe.h"
#include "SlinkEoe.h"

namespace Hgcal10gLinkReceiver {

  class RecordRunning : public Record {
  
  public:
    RecordRunning() {
    }
    
    void setHeader(uint32_t t=time(0)) {
      setIdentifier(RecordHeader::EventData);
      setState(FsmState::Running);
      setPayloadLength(0);
      setUtc(t);
    }

    const SlinkBoe* slinkBoe() const {
      return (SlinkBoe*)_payload;
    }
    
    const uint64_t* daqPayload() const {
      return (payloadLength()==0?nullptr:_payload+2);
    }
    
    const SlinkEoe* slinkEoe() const {
      return (SlinkEoe*)(_payload+payloadLength()-2);
    }
    
    SlinkBoe* getSlinkBoe() {
      return (SlinkBoe*)_payload;
    }
    
    uint64_t* getDaqPayload() {
      return (payloadLength()==0?nullptr:_payload+2);
    }
    
    SlinkEoe* getSlinkEoe() {
      return (SlinkEoe*)(_payload+payloadLength()-2);
    }
    
    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "RecordRunning::print()" << std::endl;
      RecordHeader::print(o,s+" ");
      
      for(unsigned i(0);i<payloadLength();i++) {
	o << s << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(16) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
      
      slinkBoe()->print(o,s+" ");
      slinkEoe()->print(o,s+" ");
    }
    
  private:
    uint64_t _payload[4];
  };

}

#endif
