#ifndef Hgcal10gLinkReceiver_EcondSubHeader_h
#define Hgcal10gLinkReceiver_EcondSubHeader_h

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cassert>

namespace Hgcal10gLinkReceiver {

  class EcondSubHeader {

 public:
    EcondSubHeader() {
      _wordA=0;
      _wordB=0;
    }
    /*
      uint32_t word(unsigned i) const {
      assert(i<2);
      return _word[i];
      }
      void setWordTo(unsigned i, uint32_t w) {
      assert(i<2);
      _word[i]=w;
      }
    */
    uint8_t status() const {
      return (_wordA>>29)&0x7;
    }

    uint8_t hamming() const {
      return (_wordA>>26)&0x7;
    }

    bool format() const {
      return ((_wordA>>25)&0x1)!=0;
    }
  
    uint16_t commonMode(unsigned c) const {
      assert(c<2);
      return (_wordA>>(15-10*c))&0x3ff;
    }

    uint64_t channelMap() const {
      uint64_t m(_wordA&0x1f);
      m=(m<<32)|_wordB;
      return m;
    }
  
    std::vector<unsigned> channelList() const {
      uint64_t m(channelMap());
      std::vector<unsigned> l;
      for(unsigned i(0);i<37;i++) {
	if(((m>>i)&0x1)!=0) l.push_back(i);
      }
      return l;
    }
  
    unsigned numberOfChannels() const {
      uint64_t m(channelMap());
      unsigned n(0);
      for(unsigned i(0);i<37;i++) {
	if(((m>>i)&0x1)!=0) n++;
      }
      return n;
    }
  
    void print(const std::string &s="") const {
      std::cout << s << "EcondSubHeader::print()"
		<< "  Words =" << std::hex << std::setfill('0');
      for(unsigned i(0);i<2;i++) {
	//std::cout << " 0x" << std::setw(8) << _word[i];
	if(i==0) std::cout << " 0x" << std::setw(8) << _wordA;
	else     std::cout << " 0x" << std::setw(8) << _wordB;
      }
      std::cout << std::dec << std::setfill(' ') << std::endl;
    
      std::cout << s << " Status = 0x" << std::hex << std::setfill('0');
      std::cout << std::setw(1) << unsigned(status());
      std::cout << std::dec << std::setfill(' ') << std::endl;
    
      std::cout << s << " Hamming = 0x" << std::hex << std::setfill('0');
      std::cout << std::setw(1) << unsigned(hamming());
      std::cout << std::dec << std::setfill(' ') << std::endl;
    
      if(format()) std::cout << s << " Empty single-word sub-header" << std::endl;
      else std::cout << s << " Regular two-word sub-header" << std::endl;

      std::cout << s << " Common modes = "
		<< std::setw(4) << commonMode(0) << ", "
		<< std::setw(4) << commonMode(1)  << std::endl;
    
      if(format()) {
	if((_wordA&0x10)!=0) {
	  std::cout << s << " Empty because of unmasked Stat bit errors"
		    << std::endl;
	} else {
	  std::cout << s << " Empty because no channels were above threshold"
		    << std::endl;
	}
	std::cout << s << " Unused = 0x" << std::hex << std::setfill('0');
	std::cout << std::setw(2) << (_wordA&0x0f);
	std::cout << std::dec << std::setfill(' ') << std::endl;
      
      } else {
	uint64_t chMap(channelMap());
	std::cout << s << " Channel map = 0x" << std::hex << std::setfill('0');
	std::cout << std::setw(10) << chMap;
	std::cout << std::dec << std::setfill(' ') << " = 0b";
	std::vector<unsigned> ch;
	for(unsigned i(0);i<37;i++) {
	  if(((chMap>>(36-i))&0x1)!=0) {
	    std::cout << "1";
	    ch.push_back(i);
	  } else {
	    std::cout << "0";
	  }
	}
	std::cout << std::endl;
	if(ch.size()>0) {
	  std::cout << "  Number = " << std::setw(2) << ch.size()
		    << ", channels:";
	  for(unsigned i(0);i<ch.size();i++) {
	    std::cout << std::setw(3) << ch[i];
	  }	
	  std::cout << std::endl;
	}
      }
    }
  
  protected:
    //uint32_t _word[2];
    uint32_t _wordA;
    uint32_t _wordB;
    
  };

}

#endif
