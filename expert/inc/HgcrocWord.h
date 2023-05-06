#ifndef Hgcal10gLinkReceiver_HgcrocWord_h
#define Hgcal10gLinkReceiver_HgcrocWord_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

namespace Hgcal10gLinkReceiver {

  class HgcrocWord {
  
  public:
    HgcrocWord() {
      _data=0;
    }
    
    bool tp() const {
      return (_data&0x80000000)!=0;
    }
    
    bool tc() const {
      return (_data&0x40000000)!=0;
    }
    
    uint16_t adcM() const {
      return (_data>>20)&0x3ff;
    }
    
    uint16_t adc() const {
      return (_data>>10)&0x3ff;
    }

    uint16_t toa() const {
      return _data&0x3ff;
    }
    
    void setAdcM(uint16_t a) {
      _data&=0xc00fffff;
      _data|=uint32_t(a&0x3ff)<<20;
    }

    void setAdc(uint16_t a) {
      _data&=0xcff003ff;
      _data|=uint32_t(a&0x3ff)<<10;
    }

    void setToa(uint16_t a) {
      _data&=0xcffffc00;
      _data|=uint32_t(a&0x3ff);
    }

    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "HgcrocWord::print() TP = " << tp() << ", TC = " << tc()
	<< ", ADC-1 = " << std::setw(4) << adcM()
	<< ", ADC = " << std::setw(10) << adc() 
	<< ", TOA = "<< std::setw(10) << toa() << std::endl;
    }
      
  private:
    uint32_t _data;
  };

}

#endif
