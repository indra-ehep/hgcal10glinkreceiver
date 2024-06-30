#ifndef ModuleTcData_h
#define ModuleTcData_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cassert>

class HgcrocTcData {
public:
  HgcrocTcData() {
    setZero();
  }
  
  void setZero() {
    _data=0;
    _cdata=0;
  }
  
  uint16_t getCdata() const {
    return _cdata;
  }
  
  uint32_t getCharge() const {
    return _data;
  }

  void setCdata(uint16_t a) {
    assert(a<0x80); //compressed HGCROC TC data is packed into7bits 
    _cdata=a;
  }

  void setCharge(uint32_t a) {
    assert(a<0x1000000); //assuming 21bit for HD module, then bit shift of 3, then +1
    _data=a;
  }
  
  
private:
  uint32_t _data;  //raw-uncompressed data
  uint16_t _cdata; //compressed 4E+3M ()
};

class ModuleTcData {
public:
  enum {
    MaxNumberOfTCs = 48
  };

  
  ModuleTcData() {
    setZero();
  }
  
  void setZero() {
    std::memset(_data,0,sizeof(HgcrocTcData)*MaxNumberOfTCs);
    NumberOfTCs=0;
  }
  
  const unsigned getNofTCs() {return NumberOfTCs;}
  const HgcrocTcData* getTCs() const {
    return _data;
  }
  const HgcrocTcData& getTC(uint32_t i) const {
    return _data[i];
  }
  
  void setNofTCs(const unsigned nofTCs) {NumberOfTCs = nofTCs;}
  void setTCs(const HgcrocTcData* data) {
    for(unsigned i(0);i<=NumberOfTCs;i++)
      _data[i] = data[i];
  }
  
  void print() const {
    std::cout << "ModuleTriggerCellData(" << this << ")::print()" << std::endl;
    
    for(unsigned i(0);i<=NumberOfTCs;i++) {
      std::cout << " TC " << std::setw(2) << i << " = 0x"
		<< std::hex << ::std::setfill('0')
		<< std::setw(4) << _data[i].getCdata()
		<< std::dec << ::std::setfill(' ')
		<< std::setw(4) << _data[i].getCharge()
		<< std::endl;
    }    
  }
  
private:
  HgcrocTcData _data[48];
  unsigned NumberOfTCs;
};


#endif
