#ifndef Hgcal10gLinkReceiver_I2cInstruction_h
#define Hgcal10gLinkReceiver_I2cInstruction_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

namespace Hgcal10gLinkReceiver {

  class I2cInstruction {
  
  public:
    I2cInstruction() {
    }
    
    uint16_t address() const {
      return _data>>16;
    }
    
    uint8_t mask() const {
      return (_data>>8)&0xff;
    }
    
    uint8_t value() const {
      return _data&0xff;
    }
    
    uint32_t data() const {
      return _data;
    }
    
    void setAddress(uint16_t a) {
      _data&=0x0000ffff;
      _data|=uint32_t(a&0xfff)<<16; // LIMIT TO 12 BITS!
    }

    void setMask(uint8_t m) {
      _data&=0xffff00ff;
      _data|=uint16_t(m)<<8;
    }

    void setValue(uint8_t v) {
      _data&=0xffffff00;
      _data|=v;
    }

    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "I2cInstruction::print() Address 0x"
	<< std::hex << std::setfill('0')
	<< std::setw(4) << address()
	<< ", mask 0x" << std::setw(2) << unsigned(mask())
	<< ", value 0x" << std::setw(2) << unsigned(value())
	<< std::dec << std::setfill(' ')
	<< std::endl;
    }
    
  private:
    uint32_t _data;
  };

}

#endif
