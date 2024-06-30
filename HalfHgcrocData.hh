#ifndef HalfHgcrocData_h
#define HalfHgcrocData_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cassert>


class HalfHgcrocChannelData {
  public:
    HalfHgcrocChannelData() {
      setZero();
    }

    void setZero() {
      _data=0;
    }
    
    bool isTot() const {
      return _data>=0x8000;
    }

    uint16_t getAdc() const {
      if(isTot()) return 0;
      return _data&0x3ff;
    }
    
    uint16_t getTot() const {
      if(!isTot()) return 0;
      return _data&0xfff;
    }

    void setAdc(uint16_t a) {
      assert(a<0x400);
      _data=a;
    }

    void setTot(uint16_t a) {
      assert(a<0x1000);
      _data=a|0x8000;
    }
    
  private:
    uint16_t _data;
  };
  
class HalfHgcrocData {
public:
  enum {
    NumberOfChannels=36
  };
  
  HalfHgcrocData() {
    setZero();
  }

  void setZero() {
    std::memset(_data,0,sizeof(HalfHgcrocChannelData)*NumberOfChannels);
  }
  
  const HalfHgcrocChannelData* getChannels() const {
    return _data;
  }

  /////////////// Following are the modification/addition for emulation ///////////////////
  // HalfHgcrocChannelData* setChannels() {
  //   return _data;
  // }
  void setChannels(const HalfHgcrocChannelData* data) {
    for(unsigned i(0);i<=NumberOfChannels;i++)
      _data[i] = data[i];
  }
  const HalfHgcrocChannelData& getChannelData(uint32_t i) const {
    return _data[i];
  }
  ///////////////////////////////////////////////////////////////////////////////////////
  
  void print() const {
    std::cout << "HalfHgcrocData(" << this << ")::print()" << std::endl;

    const uint16_t *p((const uint16_t*)_data);

    for(unsigned i(0);i<=NumberOfChannels;i++) {
      std::cout << " Channel " << std::setw(2) << i << " = 0x"
		<< std::hex << ::std::setfill('0')
		<< std::setw(4) << p[i]
		<< std::dec << ::std::setfill(' ') << ", "
		<< (_data[i].isTot()?"TOT = ":"ADC = ") << std::setw(4)
		<< (_data[i].isTot()?_data[i].getTot():_data[i].getAdc())
		<< std::endl;
    }    
  }

private:
  HalfHgcrocChannelData _data[36];
};

#endif
