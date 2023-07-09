#ifndef Hgcal10gLinkReceiver_SerenityLpgbt_h
#define Hgcal10gLinkReceiver_SerenityLpgbt_h

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

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityLpgbt : public SerenityUhal {
    
  public:
  
    SerenityLpgbt() {
    }
    
    virtual ~SerenityLpgbt() {
    }
    
    bool makeTable() {
      return SerenityUhal::makeTable("payload.fc_ctrl.fc_lpgbt_pair");
    }

    void configuration(YAML::Node &m) {
      m=YAML::Node();

      for(unsigned i(0);i<8;i++) {
        std::ostringstream sout;
        sout << "_" << i;

	m[std::string("ctrl.calpulse_type")+sout.str()]=uhalRead("ctrl.calpulse_type");
	m[std::string("ctrl.user_bx"      )+sout.str()]=uhalRead("ctrl.user_bx");
	m[std::string("ctrl.loop_user_cmd")+sout.str()]=uhalRead("ctrl.loop_user_cmd");
	m[std::string("fc_cmd.user"       )+sout.str()]=uhalRead("fc_cmd.user");
      }
    }

    void configuration(std::unordered_map<std::string,uint32_t> &m) {
      m.clear();

      m["ctrl.calpulse_type"]=uhalRead("ctrl.calpulse_type");
      m["ctrl.user_bx"]=uhalRead("ctrl.user_bx");
      m["ctrl.loop_user_cmd"]=uhalRead("ctrl.loop_user_cmd");
      m["fc_cmd.user"]=uhalRead("fc_cmd.user");
    }
    
    bool setDefaults() {
      uhalWrite("ctrl.calpulse_type",1);
      uhalWrite("ctrl.user_bx",1);
      uhalWrite("ctrl.loop_user_cmd",0);
      uhalWrite("fc_cmd.user",0x36);
      return true;
    }  

    void print(std::ostream &o=std::cout) {
      o << "SerenityLpgbt::print()" << std::endl;
      o << " Current settings for " << _uhalString.size()
	<< " values:" << std::endl;

      o << std::hex << std::setfill('0');

      for(unsigned i(0);i<_uhalString.size();i++) {
	uint32_t v(uhalRead(_uhalString[i]));
	std::cout << "  " << _uhalString[i] << " = 0x"
		  << std::hex << std::setfill('0')
		  << std::setw(8) << v
		  << std::dec << std::setfill(' ')
		  << " = " << v
		  << std::endl;
      }
      o << std::dec << std::setfill(' ');
    }

  
  protected:

  };

}

#endif
