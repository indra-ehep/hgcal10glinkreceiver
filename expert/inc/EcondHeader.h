#ifndef Hgcal10gLinkReceiver_EcondHeader_h
#define Hgcal10gLinkReceiver_EcondHeader_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

#include "EcondSubHeader.h"

namespace Hgcal10gLinkReceiver {

  class EcondHeader {
  public:
    enum {
      //HeaderPattern=0x1e6
      HeaderPattern=0x154
    };

    enum DataType {
      Normal,
      Truncated
    };
  
    // Also used in idle
    enum ResetRequest {
      None,
      RequestA, //???
      RequestB, //???
      RequestC  //???
    };

    EcondHeader() {
      reset();
    }

    void reset() {
      _wordA=(HeaderPattern<<23);
      _wordB=0;
    }

    /*
    uint32_t word(unsigned i) const {
      return word_[i];
    }
    void setWordTo(unsigned i, uint32_t w) {
      word_[i]=w;
    }
    */
    
    uint16_t headerPattern() const {
      return _wordA>>23;
    }
  
    // Historic: length in 16-bit words
    //uint16_t packetLength() const {}

    // Length in 32-bit words
    uint16_t payloadLength() const {
      return (_wordA>>14)&0x1ff;
    }
    void setPayloadLengthTo(uint16_t l) {
      _wordA=(_wordA&0xff803fff)|(l<<14);
      if(l==0) _wordA|=0x40; // Set truncation bit
    }
    uint16_t packetWords() const {
      return payloadLength();
    }
    uint16_t totalPacketLength() const {
      return payloadLength()+2;
    }
  
    void setPacketWords(uint16_t w) {
      setPayloadLengthTo(w);
    }

    // P,E,H/T,E/B/O,M,S
    uint8_t qualityFlags() const {
      return (_wordA>>7)&0x3f;
    }

    bool qualityFlagE() const {
      return (qualityFlags()&0x20)!=0;
    }

    uint8_t qualityFlagHT() const {
      return (qualityFlags()>>3)&0x03;
    }

    uint8_t qualityFlagEBO() const {
      return (qualityFlags()>>1)&0x03;
    }

    bool qualityFlagM() const {
      return (qualityFlags()&0x01)!=0;
  
    }

    void setQualityFlagsTo(uint8_t f) {
      _wordA=(_wordA&0xffffc07f)|((f&0x7f)<<7);
      _wordB=(_wordB&0xfffffbff)|((f&0x80)<<3);
    }

    bool truncated() const {
      return (_wordA&0x40)!=0;
    }

    bool passthroughMode() const {
      return (_wordA&0x2000)!=0;
    }

    void setPassthroughMode(bool p) {
      if(p) _wordA|=0x00002000;
      else  _wordA&=0xffffdfff;
    }

    bool subpacketError() const {
      return (_wordB&0x400)!=0;
    }

    uint8_t hamming() const {
      return _wordA&0x3f;
    }

    uint16_t bx() const {
      return (_wordB>>20)&0xfff;
    }

    uint8_t event() const {
      return (_wordB>>14)&0x3f;
    }

    uint8_t orbit() const {
      return (_wordB>>11)&0x7;
    }

    void setBx(uint16_t b) {
      _wordB&=0x000fffff;
      _wordB|=uint32_t(b)<<20;
    }

    void setEvent(uint8_t e) {
      _wordB&=0xfff03fff;
      _wordB|=uint32_t(e&0x3f)<<14;
    }

    void setOrbit(uint8_t o) {
      _wordB&=0xffffc7ff;
      _wordB|=uint32_t(o&0x7)<<11;
    }
    
    DataType dataType() const {
      return (DataType)((_wordA>>6)&0x1);
    }

    void setDataTypeTo(DataType t) {
      _wordA=(_wordA&0xffffffbf)|((t&0x1)<<6);
    }

    ResetRequest resetRequest() const {
      return (ResetRequest)((_wordB>>8)&0x3);
    }

    void setResetRequestTo(ResetRequest r) {
      _wordB=(_wordB&0xfffffcff)|((r&0x3)<<8);
    }

    uint8_t crc() const {
      return (_wordB&0xff);
    }

    uint8_t calculateCrc() const {
      const uint16_t key(0x1a7); // x^8+x^7+x^5+x^2+x^1+x^0 = 110100111

      uint64_t d(_wordA&0xffffffc0);
      d=(d<<32)|(_wordB&0xffffff00);

      uint16_t tmp(d>>55);

      for(unsigned i(9);i<=64;i++) {
	if((tmp&0xff00)!=0) tmp=key^tmp;
	else                tmp=  0^tmp;
	if(i<64) tmp=(tmp<<1)|((d>>(63-i))&0x1);
      }

      return tmp&0xff;
    }

    bool validCrc() const {
      return crc()==calculateCrc();
    }

    void setCrc() {
      _wordB=(_wordB&0xffffff00)|calculateCrc();
      assert(validCrc()); // TEMP
    }

    bool operator==(const EcondHeader &h) const {
      return ((_wordA&0xfff90001)==(h._wordA&0xfff90001))
      && ((_wordB&0xfc01ffff)==(h._wordB&0xfc01ffff));
    }

    bool valid() const {
      return true; // ???
    }

    void print(const std::string &s="") const {
      std::cout << s << "EcondHeader::print()"
		<< "  Words =" << std::hex << std::setfill('0');
      for(unsigned i(0);i<2;i++) {
	//std::cout << " 0x" << std::setw(8) << _word[i];
	if(i==0) std::cout << " 0x" << std::setw(8) << _wordA;
	else     std::cout << " 0x" << std::setw(8) << _wordB;
      }
      std::cout << std::dec << std::setfill(' ') << std::endl;

      std::cout << s << " Header pattern = 0x"
		<< std::hex << std::setfill('0')
		<< std::setw(4) << headerPattern();
      if(headerPattern()!=HeaderPattern) {
	std::cout << " != expected "<< std::setw(4) << HeaderPattern;
      }
      std::cout << std::dec << std::setfill(' ') << std::endl;

      std::cout << s << " Packet words = " << packetWords();
      //	    << ", data type = ";
      //if     (dataType()==   Normal) std::cout << "Normal";
      //else if(dataType()==Truncated) std::cout << "Truncated";
      if(truncated()) std::cout << ", truncated";
      std::cout << std::endl;
  
      if(passthroughMode()) std::cout << s << " Pass through mode" << std::endl;
      else std::cout << s << " Reformatting mode" << std::endl;
  
      std::cout << s << " Quality flags = 0x"
		<< std::hex << std::setfill('0')
		<< std::setw(2) << unsigned(qualityFlags())
		<< std::dec << std::setfill(' ')
		<< ": E = " << (qualityFlagE()?1:0)
		<< ", HT = " << unsigned(qualityFlagHT())
		<< ", EBO = " << unsigned(qualityFlagEBO())
		<< ", M = " << (qualityFlagE()?1:0)
		<< std::endl;


      std::cout << s << " Data type = "
		<< (dataType()==Normal?"not truncated":"truncated") << std::endl;
      std::cout << s << " Hamming = "
		<< unsigned(hamming()) << std::endl;
      //counters().print(s+" ");

      std::cout << s << " BX, event, orbit = " << std::setw(4)
		<< ((_wordB>>20)&0xfff)
		<< ", " << std::setw(2) << ((_wordB>>14)&0x3f) << ", "
		<< ((_wordB>>11)&0x7) << std::endl;
  
      if(subpacketError()) std::cout << s << " Subpacket error" << std::endl;
      else std::cout << s << " No subpacket error" << std::endl;

      std::cout << s << " Reset request = " << resetRequest() << " = ";
      if     (resetRequest()==    None) std::cout << "None";
      else if(resetRequest()==RequestA) std::cout << "RequestA";
      else if(resetRequest()==RequestB) std::cout << "RequestB";
      else if(resetRequest()==RequestC) std::cout << "RequestC";
      else std::cout << "Unknown";
      std::cout << std::endl;

      std::cout << s << " CRC = 0x" << std::hex << std::setfill('0')
		<< std::setw(2) << unsigned(crc())
		<< std::dec << std::setfill(' ') << " = "
		<< (validCrc()?"valid":"invalid") << std::endl;
      if(!validCrc()) {
	std::cout << s << " Calculated CRC = 0x" << std::hex << std::setfill('0')
		  << std::setw(2) << unsigned(calculateCrc())
		  << std::dec << std::setfill(' ') << std::endl;  
      }
    }

  protected:
    //uint32_t _word[2];
    uint32_t _wordB;
    uint32_t _wordA;
    
  };

}

#endif
