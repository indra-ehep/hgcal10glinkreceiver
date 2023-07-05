#ifndef Hgcal10gLinkReceiver_SerenityEncoder_h
#define Hgcal10gLinkReceiver_SerenityEncoder_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

#include "SerenityUhal.h"
#include "I2cInstruction.h"
#include "UhalInstruction.h"

namespace Hgcal10gLinkReceiver {

  class SerenityEncoder : public SerenityUhal {

  public:
  
    SerenityEncoder() { 
      SerenityUhal::makeTable();
    }
    
    virtual ~SerenityEncoder() {
    }
    
    bool makeTable() {
      return SerenityUhal::makeTable("payload.fc_ctrl.fpga_fc");
    }

    void configuration(YAML::Node &m) {
      m=YAML::Node();

      m["ctrl.prel1a_offset"     ]=uhalRead("ctrl.prel1a_offset");
      m["ctrl.user_prel1a_off_en"]=uhalRead("ctrl.user_prel1a_off_en");
      m["ctrl.l1a_stretch"       ]=uhalRead("ctrl.l1a_stretch");

      m["calpulse_ctrl.calpulse_int_del"]=uhalRead("calpulse_ctrl.calpulse_int_del");
      m["calpulse_ctrl.calpulse_ext_del"]=uhalRead("calpulse_ctrl.calpulse_ext_del");
      m["calpulse_ctrl.ocr_n"           ]=uhalRead("calpulse_ctrl.ocr_n");

      m["ctrl_2"]=uhalRead("ctrl_2");
      m["ctrl_3"]=uhalRead("ctrl_3");      
    }

    void configuration(std::unordered_map<std::string,uint32_t> &m) {
      m.clear();
      
      m["ctrl.prel1a_offset"     ]=uhalRead("ctrl.prel1a_offset");
      m["ctrl.user_prel1a_off_en"]=uhalRead("ctrl.user_prel1a_off_en");
      m["ctrl.l1a_stretch"       ]=uhalRead("ctrl.l1a_stretch");

      m["calpulse_ctrl.calpulse_int_del"]=uhalRead("calpulse_ctrl.calpulse_int_del");
      m["calpulse_ctrl.calpulse_ext_del"]=uhalRead("calpulse_ctrl.calpulse_ext_del");
      m["calpulse_ctrl.ocr_n"           ]=uhalRead("calpulse_ctrl.ocr_n");

      m["ctrl_2"]=uhalRead("ctrl_2");
      m["ctrl_3"]=uhalRead("ctrl_3");
    }

    void status(std::unordered_map<std::string,uint32_t> &m) {
      m.clear();
      
      m["l1a_counter_l"]=uhalRead("l1a_counter_l");
      m["l1a_counter_h"]=uhalRead("l1a_counter_h");
      m["orb_counter"  ]=uhalRead("orb_counter");
    }

    void pulse(std::string &s) {
      if(uhalRead(s)==1) uhalWrite(s,0);
      uhalWrite(s,1);
      uhalWrite(s,0);
    }

    bool setDefaults() {

      // Zero all fundamental values
      uhalWrite("ctrl",0);
      uhalWrite("calpulse_ctrl",0);
      uhalWrite("ctrl_2",0);
      uhalWrite("ctrl_3",0);

      // Set specific values
      uhalWrite("ctrl.tts",0);
      uhalWrite("ctrl.prel1a_offset",3);
      uhalWrite("ctrl.user_prel1a_off_en",1);
      uhalWrite("ctrl.l1a_stretch",0);
      
      uhalWrite("calpulse_ctrl.calpulse_int_del",8);
      uhalWrite("calpulse_ctrl.calpulse_ext_del",10);
      uhalWrite("calpulse_ctrl.ocr_n",8);

      return true;
    }  
    
    void print(std::ostream &o=std::cout) {
      o << "SerenityEncoder::print() Top string "
	<< _uhalTopString << std::endl;
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
