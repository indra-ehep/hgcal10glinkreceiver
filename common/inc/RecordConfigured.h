#ifndef Hgcal10gLinkReceiver_RecordConfigured_h
#define Hgcal10gLinkReceiver_RecordConfigured_h

#include <iostream>
#include <iomanip>
#include <cassert>
#include <unordered_map>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordConfigured : public RecordT<1023> {
  
  public:
    enum Type {
      HGCROC,
      ECOND,
      ECONT,
      BE,
      TCDS2,
      EndOfTypeEnum
    };
    
    RecordConfigured() {
    }
    
    void setHeader(uint32_t c) {
      setState(FsmState::Configured);
      setPayloadLength(2);
      setUtc(c);
      setType(EndOfTypeEnum);
      setLocation(0);
    }
  
    bool valid() const {
      return validPattern() && state()==FsmState::Configured;
    }
    
    uint32_t relayNumber() const {
      return _payload[0]&0xffffffff;
    }

    uint32_t configurationNumber() const {
      return _payload[0]>>32;
    }

    Type type() const {
      return (Type)(_payload[1]&0xffffffff);
    }

    uint32_t location() const {
      return _payload[1]>>32;
    }

    void setRelayNumber(uint32_t t=time(0)) {
      _payload[0]&=0xffffffff00000000;
      _payload[0]|=t;
    }
   
    void setConfigurationCounter(uint32_t c) {
      _payload[0]&=0x00000000ffffffff;
      _payload[0]|=uint64_t(c)<<32;
    }
    
    void setType(Type t) {
      _payload[1]=0xdeadcafe00000000;
      _payload[1]|=t;
    }

    void setLocation(uint32_t l) {
      _payload[1]&=0x00000000ffffffff;
      _payload[1]|=uint64_t(l)<<32;
    }

    void addData32(uint32_t d) {
      if((_payload[payloadLength()-1]>>32)==0xdeaddead) {
	_payload[payloadLength()-1]&=0x00000000ffffffff;
	_payload[payloadLength()-1]|=uint64_t(d)<<32;
      } else {
	_payload[payloadLength()]=0xdeaddead00000000|d;
	incrementPayloadLength(1);
      }
    }

    void addData64(uint64_t d) {
      _payload[payloadLength()]=d;
      incrementPayloadLength(1);
    }

    void addString(const std::string &s) {
      unsigned n((s.size()+8)/8);
      _payload[payloadLength()+n]=0xffffffff;
      std::memcpy(_payload+payloadLength(),s.c_str(),s.size()+1);
      incrementPayloadLength(n);
    }

    void configuration(std::unordered_map<std::string,uint32_t> &m) const {
      m.clear();

      const uint32_t *p((uint32_t*)(_payload+2));
      for(unsigned n32(0);n32<2*(payloadLength()-2)-1;) {
	std::string s((const char*)(p+n32));
	n32+=(s.size()+4)/4;
	m[s]=p[n32];
	n32++;
      }
    }
    
    void setConfiguration(const std::unordered_map<std::string,uint32_t> &m) {
      uint32_t *p((uint32_t*)(_payload+2));

      unsigned n32(0);

      for(auto i(m.begin());i!=m.end();i++) {
	const std::string &s(i->first);
	unsigned n((s.size()+4)/4);
	std::memcpy(p+n32,s.c_str(),s.size()+1);
	n32+=n;
	p[n32]=i->second;
	n32++;
      }

      setPayloadLength(2+(n32+1)/2);
    }
    
    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "RecordConfigured::print()" << std::endl;
      RecordHeader::print(o,s+" ");
      
      for(unsigned i(0);i<payloadLength();i++) {
	o << s << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(16) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
      
      o << s << "  Relay number          = "
	<< std::setw(10) << relayNumber() << std::endl;
      o << s << "  Configuration number  = "
        << std::setw(10) << configurationNumber() << std::endl;
      o << s << "  Type                  = "
	<< std::setw(10) << type() << std::endl;
      o << s << "  Location              = "
	<< std::setw(10) << location() << std::endl;
    }

  private:
  };

}

#endif
