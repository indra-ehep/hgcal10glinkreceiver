#ifndef Hgcal10gLinkReceiver_DthHeader_h
#define Hgcal10gLinkReceiver_DthHeader_h

#include <iostream>
#include <iomanip>
#include <cstdint>

namespace Hgcal10gLinkReceiver {

class DthHeader {
 public:
  DthHeader() {
    reset();
  }

  void reset() {
    _word[1]=0x000004ce0002efc8;
    _word[0]=0x0004c00000005a47;
  }
  
  uint16_t  length() const {
    return (_word[0]>>48);
  }
  
  uint16_t  bxNumber() const {
    return (_word[0]>>32)&0xffff;
  }
    
  uint16_t  blockNumber() const{
    return (_word[0]>>16)&0xffff;
  }

  uint16_t id() const{
    return (_word[0]    )&0xffff;
  }
  
  uint32_t eventNumber() const {
    return _word[1]&0xffffffff;
  }

  uint32_t sourceId() const{
    return _word[1]>>32;
  }

  void setEventId(uint64_t e) {
    assert(e<(1UL<<44));
    _word[1]&=0xfffff00000000000;
    _word[1]|=e;
  }
  
  void setL1aSubType(uint8_t st) {
    _word[0]&=0x00ff000000000000;
    _word[0]|=uint64_t(st)<<48;
  }
  
  void setL1aType(uint16_t t) {
    _word[0]&=0x0000ffff00000000;
    _word[0]|=uint64_t(t)<<32;
  }
  
  void setSourceId(uint32_t s) {
    _word[0]&=0x00000000ffffffff;
    _word[0]|=s;
  }

  bool valid() const {
    return boeHeader()==BoePattern && version()==0x03;
  }

  void print(std::ostream &o=std::cout, const std::string &s="") const {
    o << s << "DthHeader::print()  words = 0x"
      << std::hex << std::setfill('0')
      << std::setw(16) << _word[1] << ", 0x"
      << std::setw(16) << _word[0]
      << std::dec << std::setfill(' ')
      << std::endl;
    o << s << " ID = " << std::setw(5) << id()
      << std::endl;
    o << s << " Block number = " << std::setw(5) << blockNumber()
      << std::endl;
    o << s << " BX number = " << std::setw(5) << bxNumber()
      << std::endl;
    o << s << " Length = " << std::setw(5) << length()
      << std::endl;
    o << s << " Event number = " << std::setw(10) << eventNumber()
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
