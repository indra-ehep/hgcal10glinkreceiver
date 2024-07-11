#ifndef TPGFEDataformat_h
#define TPGFEDataformat_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cassert>

namespace TPGFEDataformat{

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////// Data Formats by Paul Dauncey /////////////////////////////////
  ///provision for tctp is added by Indra (without altering the existing functionalitites)
  
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

    uint16_t getTcTp() const {
      return (_data>>12)&0x3;
    }

    uint16_t getAdc() const {
      if(isTot()) return 0;
      return _data&0x3ff;
    }

    uint16_t getTot() const {
      if(!isTot()) return 0;
      return _data&0xfff;
    }

    void setAdc(uint16_t a, uint16_t tctp=0x0) {
      assert(a<0x400 and tctp<0x4);
      _data=tctp<<12|a;
    }
    
    void setTot(uint16_t a, uint16_t tctp=0x3) {
      assert(a<0x1000 and tctp<0x4);
      _data=tctp<<12|a|0x8000;
    }

    void print() const {
      std::cout << "HalfHgcrocChannelData(" << this << ")::print()" << std::endl;

      std::cout << std::dec << ::std::setfill(' ') << ", "
		<< "TcTp: " << getTcTp() << ", "
		<< (isTot()?"TOT = ":"ADC = ") << std::setw(4)
		<< (isTot()?getTot():getAdc())
		<< std::endl;

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

      for(unsigned i(0);i<NumberOfChannels;i++) {
	std::cout << " Channel " << std::setw(2) << i << " = 0x"
		  << std::hex << ::std::setfill('0')
		  << std::setw(4) << p[i]
		  << std::dec << ::std::setfill(' ') << ", "
		  << "TcTp: " << _data[i].getTcTp() << ", "
		  << (_data[i].isTot()?"TOT = ":"ADC = ") << std::setw(4)
		  << (_data[i].isTot()?_data[i].getTot():_data[i].getAdc())
		  << std::endl;
      }    
    }

  private:
    HalfHgcrocChannelData _data[36];
  };


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
  
    const std::string& typeName() const {
      return _typeName[_data>>14];
    }

    void setModuleSum(uint8_t e) {
      _data=BestC|e<<6|0x3f;    
    }

    void setTriggerCell(Type t, uint8_t a, uint16_t e) {
      if(t==BestC || t==STC4A) {
	if(t==BestC) assert(a<48);//47-->48
	else assert(a<4);
	assert(e<0x80); //but modsum is 5E+3M 0x80 --> 0x100
	_data=t|e<<6|a;
      
      } else {
	if(t==STC4B) assert(a<4);
	else assert(a<16);
	assert(e<0x200);
	_data=t|e<<4|a;      
      }
    }
  
    void print() const {
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
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  
  class HgcrocTcData {
  public:
    HgcrocTcData() {
      setZero();
    }
  
    void setZero() {
      _data=0;
      _cdata=0;
      _isTot=false;
    }

    bool isTot() const {
      return _isTot;
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
  
    void setTot(bool a) {
      _isTot=a;
    }
  
  private:
    uint32_t _data;  //raw-uncompressed data
    uint16_t _cdata; //compressed 4E+3M ()
    bool _isTot;     //isTOT or ADC
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
  
    const uint32_t getNofTCs() const {return uint32_t(NumberOfTCs);}
    const HgcrocTcData* getTCs() const {
      return _data;
    }
    const HgcrocTcData& getTC(uint32_t i) const {
      return _data[i];
    }
  
    void setNofTCs(const unsigned nofTCs) {NumberOfTCs = nofTCs;}
    void setTCs(const HgcrocTcData* data) {
      for(uint16_t i(0);i<=NumberOfTCs;i++)
	_data[i] = data[i];
    }
  
    void print() const {
      std::cout << "ModuleTriggerCellData(" << this << ")::print()" << std::endl;
    
      for(uint16_t i(0);i<NumberOfTCs;i++) {
	std::cout << " TC " << std::setw(2) << i << ": compressed = "
		  << std::dec << ::std::setfill(' ')
		  << std::setw(10) << _data[i].getCdata()
		  << std::dec << ::std::setfill(' ')
		  << ", raw-uncompressed "
		  << std::setw(10) << _data[i].getCharge()
		  << ", istot : "
		  << std::setw(10) << _data[i].isTot()
		  << std::endl;
      }    
    }
  
  private:
    HgcrocTcData _data[48];
    uint16_t NumberOfTCs;
  };


}

#endif
