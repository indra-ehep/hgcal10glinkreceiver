#ifndef Hgcal10gLinkReceiver_BeTriggerData_h
#define Hgcal10gLinkReceiver_BeTriggerData_h

#include <iostream>
#include <iomanip>
#include <cstdint>


namespace Hgcal10gLinkReceiver {

  class BeTriggerData {
  
  public:
    enum Type {
      Scintillator,
      Econt,
      Unpacker,
      Reserved,
      EndOfTypeEnum
    };

    
    BeTriggerData() {
      reset();
    }
    
    void reset() {
      _header=0;
    }

    uint8_t numberLtL1a(Type t) {
      if(t>=EndOfTypeEnum) return 0;
      return (_header>>(16*t))&0xff;
    }
    
    uint8_t numberGeL1a(Type t) {
      if(t>=EndOfTypeEnum) return 0;
      return (_header>>(16*t+8))&0xff;
    }
    
    uint16_t number(Type t) {
      return uint16_t(numberLtL1a(t))+uint16_t(numberGeL1a(t));
    }
    
    const uint32_t* typePayload(Type t) {
      uint32_t *p(this+1);
      for(unsigned i(0);i<t;i++) {
	if(i==0) p+= 6*number(Type(i)); // 4 x scint + spill + padding
	if(i==1) p+=10*number(Type(i)); // 5 x elinks, 2 x ECON-Ts
	if(i==2) p+=12*number(Type(i)); // 6 x words, 2 x ECON-Ts
      }
      return p;
    }
    
    void print(std::ostream &o=std::cout, std::string s="") const {
      o << s << "BeTriggerData::print()" << std::endl;

      o << s << " Header = 0x"
        << std::hex << std::setfill('0')
        << std::setw(16) << _header
        << std::dec << std::setfill(' ')
	<< std::endl;
      for(unsigned i(0);i<EndOfTypeEnum;i++) {
	Type t(i);
	o << s << "  Type " << t << ": BX less than L1A = "
	  << unsigned(numberLtL1a(t))
	  << ", greater than or equal to L1A = "
	  << unsigned(numberGeL1a(t))
	  << std::endl;
      }
    }
    
  private:
    uint64_t _header;
  };

}

#endif
