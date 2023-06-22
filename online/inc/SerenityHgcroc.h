#ifndef Hgcal10gLinkReceiver_SerenityHgcroc_h
#define Hgcal10gLinkReceiver_SerenityHgcroc_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <unordered_map>

#include "SerenityUhal.h"
#include "I2cInstruction.h"
#include "UhalInstruction.h"

#include <yaml-cpp/yaml.h>

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityHgcroc : public SerenityUhal {
    
  public:
  
    SerenityHgcroc() {
    }
    
    virtual ~SerenityHgcroc() {
    }
    
    bool makeTable() {
      //return SerenityUhal::makeTable("payload.fc_ctrl.fc_lpgbt_pair");
      return true;
    }

    void configuration(YAML::Node &m) {
    }

    void configuration(std::unordered_map<std::string,uint32_t> &m) {
    }
    
    bool setDefaults() {
      return true;
    }  

    std::vector<unsigned int> i2cRead(unsigned char link, int lpgbt, unsigned hgcroc, unsigned addr, unsigned int len=1) {
      unsigned gAdd(0x70);
      if(lpgbt>0) gAdd=0x71;
      if(lpgbt<0) gAdd=0x72;

      unsigned hAdd(0x08+0x10*hgcroc);

      std::vector<unsigned int> dAdd(1);

      dAdd[0]=addr&0xff;
      std::cout << "Data address R0 =  " << std::hex << dAdd[0] << std::dec << std::endl;
      i2c_write(link,gAdd,hAdd  ,dAdd);

      dAdd[0]=(addr>>8)&0x0f;
      std::cout << "Data address R1 =  " << std::hex << dAdd[0] << std::dec << std::endl;
      i2c_write(link,gAdd,hAdd+1,dAdd);

      return i2c_read(link,gAdd,hAdd+2,len);
    }  

    void i2cReadAll(unsigned char link, int lpgbt, unsigned hgcroc) {
      unsigned sbNumber[96];

      for(unsigned i(0);i<2;i++) {
	for(unsigned j(0);j<39;j++) sbNumber[46*i+j]=14;
	sbNumber[46*i+39]=0;
	sbNumber[46*i+40]=9;
	sbNumber[46*i+41]=15;
	sbNumber[46*i+42]=16;
	sbNumber[46*i+43]=27;
	sbNumber[46*i+44]=0;//14; Halfwise
      }
      sbNumber[45]=21;

      sbNumber[91]=0;
      sbNumber[92]=0;
      sbNumber[93]=0;
      sbNumber[94]=0;
      sbNumber[95]=0;


      unsigned gAdd(0x70);
      if(lpgbt>0) gAdd=0x71;
      if(lpgbt<0) gAdd=0x72;

      unsigned hAdd(0x08+0x10*hgcroc);

      std::vector<unsigned int> dAdd(1);

      unsigned nTot(0);

      for(uint16_t r1(0);r1<12;r1++) {
	//std::cout << "New R1 = " << std::dec << r1 << std::endl;

	dAdd[0]=r1;
	i2c_write(link,gAdd,hAdd+1,dAdd);

	for(uint16_t r0(0);r0<256;r0++) {
	  dAdd[0]=r0;

	  uint16_t add(256*r1+r0);
	  uint16_t sb(add>>5);
	  uint16_t rg(add&0x1f);
	  /*
	  std::cout << "New R0 = " << std::dec << r0 
		    << ", sb, rg = " << sb << ", " << rg 
		    << "/" << sbNumber[sb]
		    << std::endl;
	  */
	  assert(sb<96);

	  if(rg<sbNumber[sb]) {
	    nTot++;
	    i2c_write(link,gAdd,hAdd,dAdd);
	    i2c_read(link,gAdd,hAdd+2);
	  }
	}
      }

      std::cout << "Total number of reads = " << std::dec << nTot << std::endl;
    }

    /*
    std::vector<unsigned int> data;
    data.push_back(addr&0xFF);
    std::cout << "writing to R0: " << std::hex << (addr&0x0f) << std::endl;
    int s = i2c_write(hi, ch, gbtx_addr, 0x18, data);
    data[0]=(addr>>8)&0xFF;
    //data.push_back();
    std::cout << "writing to R1: " << std::hex << data[0] << std::endl;
    s = i2c_write(hi, ch, gbtx_addr, 0x19, data);

    auto read_back =  i2c_read(hi, ch, gbtx_addr, 0x1A, 1);
    std::cout << " Read I2C register : " << "addr: 0x" << addr << "  value: 0x" << std::hex << read_back[0] << std::endl;
    */


    void print(std::ostream &o=std::cout) {
      o << "SerenityHgcroc::print()" << std::endl;
    }
  
  protected:

  };

}

#endif
