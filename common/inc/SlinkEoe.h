#ifndef Hgcal10gLinkReceiver_SlinkEoe_h
#define Hgcal10gLinkReceiver_SlinkEoe_h

#include <iostream>
#include <cstdint>

namespace Hgcal10gLinkReceiver {


class SlinkEoe {
 public:
  enum {
    EoePattern=0xaa
  };
  
  SlinkEoe() {
    _word[0]=uint64_t(EoePattern)<<56;
  }

  SlinkEoe(uint32_t l, uint16_t b, uint32_t o, uint16_t c, uint16_t s) {
    _word[0]=uint64_t(EoePattern)<<56;

    setEventLength(l);
    setBxId(b);
    setOrbitId(o);
    setCrc(c);
    setStatus(s);
  }

  void initialise() {
  }

  uint8_t  eoeHeader() const {
    return _word[0]>>56;
  }

  uint16_t daqCrc() const {
    return (_word[0]>>40)&0xffff;
  }

  uint32_t eventLength() const {
    return (_word[0]>>12)&0xfffff;
  }

  uint16_t bxId() const {
    return _word[0]&0xfff;
  }

  uint32_t orbitId() const {
    return _word[1]>>32;
  }

  uint16_t crc() const {
    return (_word[1]>>16)&0xffff;
  }

  uint16_t status() const {
    return _word[1]&0xffff;
  }

  void setDaqCrc(uint16_t c) {  
    _word[0]&=0xff0000ffffffffff;
    _word[0]|=uint64_t(c)<<40;
  }

  void setEventLength(uint32_t l) {
    assert(l<(1U<<20));
    _word[0]&=0xffffffff00000fff;
    _word[0]|=(uint64_t(l)<<12);
  }

  void setBxId(uint16_t b) {
    assert(b>0 && b<=3564);
    _word[0]&=0xfffffffffffff000;
    _word[0]|=b;
  }

  void setOrbitId(uint32_t o) {
    _word[1]&=0x00000000ffffffff;
    _word[1]|=uint64_t(o)<<32;    
  }

  void setCrc(uint16_t c) {  
    _word[1]&=0xffffffff0000ffff;
    _word[1]|=uint64_t(c)<<16;    
  }

  void setStatus(uint16_t s) {  
    _word[1]&=0xffffffffffff0000;
    _word[1]|=s;
  }

  void incrementEventLength() {
    setEventLength(eventLength()+1);
  }
  
  bool valid() const {
    return eoeHeader()==EoePattern;
  }

  void print(std::ostream &o=std::cout, const std::string &s="") const {
    o << s << "SlinkEoe::print()  words = 0x"
      << std::hex << std::setfill('0')
      << std::setw(16) << _word[0] << ", 0x"
      << std::setw(16) << _word[1]
      << std::dec << std::setfill(' ')
      << std::endl;
    o << s << " EOE header = 0x"
      << std::hex << std::setfill('0')
      << std::setw(2) << unsigned(eoeHeader())
      << std::dec << std::setfill(' ')
      << std::endl;
    o << s << " DAQ CRC = 0x"
      << std::hex << std::setfill('0')
      << std::setw(4) << daqCrc()
      << std::dec << std::setfill(' ')
      << std::endl;
    o << s << " Event length = " << std::setw(7) << eventLength()
      << " 128-bit words" << std::endl;
    o << s << " BX id = " << std::setw(4) << bxId() << std::endl;
    o << s << " Orbit id = " << std::setw(10) << orbitId() << std::endl;
    o << s << " CRC = 0x"
      << std::hex << std::setfill('0')
      << std::setw(4) << crc()
      << std::dec << std::setfill(' ')
      << std::endl;
    o << s << " Status = 0x"
      << std::hex << std::setfill('0')
      << std::setw(4) << status()
      << std::dec << std::setfill(' ')
      << std::endl;
  }

 private:
  uint64_t _word[2];
};

}

#endif
