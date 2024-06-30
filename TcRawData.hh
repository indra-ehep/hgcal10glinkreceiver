#ifndef TcRawData_h
#define TcRawData_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cassert>


class TcRawData {
public:
  enum Type {
    BestC=0x0000, // 4E3M
    STC4A=0x4000, // 4E3M
    STC4B=0x8000, // 5E4M
    STC16=0xc000  // 5E4M
  };
  
  TcRawData() : _data(0) {
  }

  TcRawData(uint8_t e) {
    setModuleSum(e);
  }

  TcRawData(Type t, uint8_t a, uint16_t e) {
    setTriggerCell(t,a,e);
  }

  uint8_t address() const {
    return (_data&0x8000)==0?_data&0x3f:_data&0x0f;
  }

  uint16_t energy() const {
    return (_data&0x8000)==0?(_data>>6)&0x007f:(_data>>4)&0x01ff;
  }

  uint8_t moduleSum() const {
    if(!isModuleSum()) return 0;
    return (_data>>6)&0x00ff;
  }

  bool isModuleSum() const {
    return (_data&0xc03f)==0x003f;
  }

  bool is4E3M() const {
    return (_data&0x8000)==0;
  }
  
  bool is5E4M() const {
    return (_data&0x8000)!=0;
  }
  
  Type type() const {
    return Type(_data&0xc000);
  }
  
  const std::string& typeName() {
    return _typeName[_data>>14];
  }

  void setModuleSum(uint8_t e) {
    _data=BestC|e<<6|0x3f;    
  }

  void setTriggerCell(Type t, uint8_t a, uint16_t e) {
    if(t==BestC || t==STC4A) {
      if(t==BestC) assert(a<47);
      else assert(a<4);
      assert(e<0x80);
      _data=t|e<<6|a;
      
    } else {
      if(t==STC4B) assert(a<4);
      else assert(a<16);
      assert(e<0x200);
      _data=t|e<<4|a;      
    }
  }
  
  void print() {
    std::cout << "TcRawData(" << this << ")::print(): Data = 0x"
	      << std::hex << ::std::setfill('0')
	      << std::setw(4) << _data
	      << std::dec << ::std::setfill(' ')
	      << ", type = " << typeName();
    if(isModuleSum()) std::cout << ",   module sum" << ", energy = "
				<< std::setw(3) << unsigned(moduleSum())
				<< std::endl;
    else std::cout << ", address = " << std::setw(2) << unsigned(address())
		   << ", energy = " << std::setw(3) << energy() << std::endl;
  }    

private:
  static std::string _typeName[4];
  uint16_t _data;
};

std::string TcRawData::_typeName[4]={"BestC","STC4A","STC4B","STC16"};

#endif
