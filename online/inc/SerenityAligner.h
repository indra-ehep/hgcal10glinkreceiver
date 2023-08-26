#ifndef Hgcal10gLinkReceiver_SerenityAligner_h
#define Hgcal10gLinkReceiver_SerenityAligner_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include <yaml-cpp/yaml.h>

#include "SerenityUhal.h"
#include "I2cInstruction.h"
#include "UhalInstruction.h"

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityAligner : public SerenityUhal {
    
  public:
  
    SerenityAligner() {
    }
    
    virtual ~SerenityAligner() {
    }
    
    bool makeTable() {
      _uhalTopString="payload";
      //SerenityUhal::makeTable("payload");
      return true;
    }
  
    bool setDefaults() {
      // DONE ELSEWHERE!

      /*
      uhalWrite("ctrl.reg.en",0);
      uhalWrite("ctrl.reg.duty_cycle",3);
      uhalWrite("ctrl.pkt_len",0x20);
      uhalWrite("ctrl.reg.decrement_len",0);
      uhalWrite("ctrl.pause_interval",0);
      */
      return true;
    }  

    void configuration(YAML::Node &m) {
      m=YAML::Node();

      for(unsigned i(0);i<4;i++) {
	YAML::Node ml;
	
	std::ostringstream sout;
	sout << "lpgbt" << i;
	std::string base(sout.str());
	sout << ".lpgbt_frame.";

	ml["lpgbt_frame.ctrl"]=uhalRead(sout.str()+"ctrl");
	ml["lpgbt_frame.shift_elink0"]=uhalRead(sout.str()+"shift_elink0");
	ml["lpgbt_frame.shift_elink1"]=uhalRead(sout.str()+"shift_elink1");
	ml["lpgbt_frame.shift_elink2"]=uhalRead(sout.str()+"shift_elink2");
	ml["lpgbt_frame.shift_elink3"]=uhalRead(sout.str()+"shift_elink3");
	ml["lpgbt_frame.shift_elink4"]=uhalRead(sout.str()+"shift_elink4");
	ml["lpgbt_frame.shift_elink5"]=uhalRead(sout.str()+"shift_elink5");
	ml["lpgbt_frame.shift_elink6"]=uhalRead(sout.str()+"shift_elink6");

	m[base]=ml;
      }
    }

    void print(std::ostream &o=std::cout) {
      o << "SerenityAligner::print()" << std::endl;
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
