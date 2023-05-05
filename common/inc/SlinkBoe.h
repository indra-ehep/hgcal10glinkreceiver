#ifndef Hgcal10gLinkReceiver_SlinkBoe_h
#define Hgcal10gLinkReceiver_SlinkBoe_h

#include <iostream>
#include <cstdint>

namespace Hgcal10gLinkReceiver {

class SlinkBoe {
 public:
  enum {
    BoePattern=0x55
  };
  
  SlinkBoe() {
    reset();
  }

  SlinkBoe(uint64_t e, uint8_t st, uint16_t t, uint32_t s) {
    reset();

    setEventId(e);
    setL1aSubType(st);
    setL1aType(t);
    setSourceId(s);
  }

  void reset() {
    _word[0]=uint64_t(16*BoePattern+3)<<52;
    _word[1]=0;
  }
  
  uint8_t  boeHeader() const {
    return _word[0]>>56;
  }

  bool validPattern() const {
    return boeHeader()==BoePattern;
  }
  
  uint8_t  version() const {
    return (_word[0]>>52)&0x0f;
  }
  
  uint64_t eventId() const {
    return _word[0]&0xfffffffffff;
  }
  
  uint8_t  l1aSubType() const{
    return (_word[1]>>48)&0xff;
  }

  uint16_t l1aType() const{
    return (_word[1]>>32)&0xffff;
  }
  
  uint32_t sourceId() const{
    return _word[1]&0xffffffff;
  }

  void setEventId(uint64_t e) {
    assert(e<(1UL<<44));
    _word[0]&=0xfffff00000000000;
    _word[0]|=e;
  }
  
  void setL1aSubType(uint8_t st) {
    _word[1]&=0x00ff000000000000;
    _word[1]|=uint64_t(st)<<48;
  }
  
  void setL1aType(uint16_t t) {
    _word[1]&=0x0000ffff00000000;
    _word[1]|=uint64_t(t)<<32;
  }
  
  void setSourceId(uint32_t s) {
    _word[1]&=0x00000000ffffffff;
    _word[1]|=s;
  }

  bool valid() const {
    return boeHeader()==BoePattern && version()==0x03;
  }

  void print(std::ostream &o=std::cout, const std::string &s="") const {
    o << s << "SlinkBoe::print()  words = 0x"
      << std::hex << std::setfill('0')
      << std::setw(16) << _word[0] << ", 0x"
      << std::setw(16) << _word[1]
      << std::dec << std::setfill(' ')
      << std::endl;
    o << s << " BOE header = 0x"
      << std::hex << std::setfill('0')
      << std::setw(2) << unsigned(boeHeader())
      << std::dec << std::setfill(' ')
      << std::endl;
    o << s << " Version = " << std::setw(2) << unsigned(version())
      << std::endl;
    o << s << " Event id = " << std::setw(14) << eventId()
      << std::endl;
    o << s << " Content id: L1A type = 0x"
      << std::hex << std::setfill('0')
      << std::setw(4) << l1aType() << ", sub-type = 0x"
      << std::setw(2) << unsigned(l1aSubType())
      << std::dec << std::setfill(' ')
      << std::endl;
    o << s << " Source id = 0x"
      << std::hex << std::setfill('0')
      << std::setw(8) << sourceId()
      << std::dec << std::setfill(' ')
      << std::endl;
  }

 private:
  uint64_t _word[2];
};

}

#endif
