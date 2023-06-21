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
      std::cout << "Data address R0 =  " << std::hex << dAdd[0] << std::endl;
      i2c_write(link,gAdd,hAdd  ,dAdd);

      dAdd[0]=(addr>>8)&0x0f;
      std::cout << "Data address R1 =  " << std::hex << dAdd[0] << std::endl;
      i2c_write(link,gAdd,hAdd+1,dAdd);

      return i2c_read(link,gAdd,hAdd+2,len);
    }    

    void print(std::ostream &o=std::cout) {
      o << "SerenityHgcroc::print()" << std::endl;
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


  
  protected:

  };

}

#endif
