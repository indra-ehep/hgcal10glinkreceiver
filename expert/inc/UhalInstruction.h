#ifndef Hgcal10gLinkReceiver_UhalInstruction_h
#define Hgcal10gLinkReceiver_UhalInstruction_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

namespace Hgcal10gLinkReceiver {

  class UhalInstruction {
  
  public:
    UhalInstruction() {
      _data=0;
    }
    
    uint32_t address() const {
      return _data>>32;
    }
    
    uint32_t value() const {
      return _data&0xffffffff;
    }
    
    uint64_t data() const {
      return _data;
    }
    
    void setAddress(uint32_t a) {
      _data&=0x00000000ffffffff;
      _data|=uint64_t(a)<<32;
    }

    void setValue(uint8_t v) {
      _data&=0xffffffff00000000;
      _data|=v;
    }

    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "UhalInstruction::print() Address 0x"
	<< std::hex << std::setfill('0')
	<< std::setw(8) << address()
	<< ", value 0x" << std::setw(8) << value()
	<< std::dec << std::setfill(' ')
	<< std::endl;
    }
      
  private:
    uint64_t _data;
  };

}

#endif
