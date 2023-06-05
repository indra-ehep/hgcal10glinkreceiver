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

#include "SerenityUhal.h"
#include "I2cInstruction.h"
#include "UhalInstruction.h"

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityEncoder : public SerenityUhal {
    
  public:
  
    SerenityEncoder() {
    }
    
    virtual ~SerenityEncoder() {
    }
    
    bool makeTable() {
      return SerenityUhal::makeTable("payload.fc_ctrl.fpga_fc");
      //return true;
    }
  
    bool setDefaults() {
      _uhalData.resize(7);
      
      uhalWrite("ctrl",0);
      uhalWrite("ctrl.tts",1);
      uhalWrite("ctrl.prel1a_offset",3);
      uhalWrite("ctrl.user_prel1a_off_en",1);
      uhalWrite("ctrl.l1a_stretch",0);
      
      uhalWrite("calpulse_ctrl",0);
      uhalWrite("calpulse_ctrl.calpulse_int_del",8);
      uhalWrite("calpulse_ctrl.calpulse_ext_del",10);
      uhalWrite("calpulse_ctrl.ocr_n",8);

      uhalWrite("ctrl_2",0);
      uhalWrite("ctrl_3",0);

      uhalWrite("l1a_counter_l",0);
      uhalWrite("l1a_counter_h",0);
      uhalWrite("orb_counter",0);
      
      // fc_lpgbt_pair

      uhalWrite("payload.fc_ctrl.fc_lpgbt_pair.ctrl.calpulse_type",1);
      uhalWrite("payload.fc_ctrl.fc_lpgbt_pair.ctrl.user_bx",1);
      uhalWrite("payload.fc_ctrl.fc_lpgbt_pair.fc_cmd.user",0x36);

      return true;
    }  

#ifndef ProcessorHardware
    
    bool uhalWrite(const std::string &s, uint32_t v) {
      if(s=="ctrl")          {_uhalData[0]=v;return true;}
      if(s=="calpulse_ctrl") {_uhalData[1]=v;return true;}
      if(s=="ctrl_2")        {_uhalData[2]=v;return true;}
      if(s=="ctrl_3")        {_uhalData[3]=v;return true;}
      if(s=="l1a_counter_l") {_uhalData[4]=v;return true;}
      if(s=="l1a_counter_h") {_uhalData[5]=v;return true;}
      if(s=="orb_counter")   {_uhalData[6]=v;return true;}

      return false;
    }
    
    uint32_t uhalRead(const std::string &s) {
      if(s=="ctrl") return _uhalData[0];
      /*
	<node id="tts" mask="0x00000001"/>
	<node id="rst_l1a_counter" mask="0x00000010"/>
	<node id="en_l1a_ecr" mask="0x00000100"/>
	<node id="en_orb_sync" mask="0x00000200"/>
	<node id="prel1a_offset" mask="0x00038000"/>
	<node id="user_prel1a_off_en" mask="0x00040000"/>
	<node id="l1a_stretch" mask="0x0F000000"/>
	<node id="event_counter_rst" mask="0x80000000"/>
      */
      if(s=="ctrl.tts")                return (_uhalData[0]    )&0x01;
      if(s=="ctrl.rst_l1a_counter")    return (_uhalData[0]>> 4)&0x01;
      if(s=="ctrl.en_l1a_ecr")         return (_uhalData[0]>> 8)&0x01;
      if(s=="ctrl.en_orb_sync")        return (_uhalData[0]>> 9)&0x01;
      if(s=="ctrl.prel1a_offset")      return (_uhalData[0]>>15)&0x07;
      if(s=="ctrl.user_prel1a_off_en") return (_uhalData[0]>>18)&0x01;
      if(s=="ctrl.l1a_stretch")        return (_uhalData[0]>>24)&0x0f;
      if(s=="ctrl.event_counter_rst")  return (_uhalData[0]>>31)&0x01;

      if(s=="calpulse_ctrl") return _uhalData[1];
      /*
	<node id="calpulse_int_del" mask="0x000000FF"/>
	<node id="calpulse_ext_del" mask="0x0000FF00"/>
	<node id="ocr_n" mask="0x00FF0000"/>
      */
      if(s=="calpulse_ctrl.calpulse_int_del") return (_uhalData[1]    )&0xff;
      if(s=="calpulse_ctrl.calpulse_ext_del") return (_uhalData[1]>> 8)&0xff;
      if(s=="calpulse_ctrl.ocr_n")            return (_uhalData[1]>>16)&0xff;

      if(s=="ctrl_2") return _uhalData[2];
      if(s=="ctrl_3") return _uhalData[3];

      if(s=="l1a_counter_l") return _uhalData[4];
      if(s=="l1a_counter_h") return _uhalData[5];
      if(s=="orb_counter")   return _uhalData[6];

      return 0xffffffff;
    }
    
#endif

    
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
