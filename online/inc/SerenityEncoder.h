#ifndef SerenityEncoder_h
#define SerenityEncoder_h

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
      SerenityUhal::makeTable("payload.fc_ctrl.fpga_fc");
      return true;
    }
  
    bool setDefaults() {
      uhalWrite("payload.fc_ctrl.fpga_fc.ctrl.tts",1);
      uhalWrite("payload.fc_ctrl.fpga_fc.ctrl.prel1a_offset",3);
      uhalWrite("payload.fc_ctrl.fpga_fc.ctrl.user_prel1a_off_en",1);
      
      uint32_t hgcrocLatencyBufferDepth(10); // ????
      uint32_t cpid(8),cped(10); // ???
      uhalWrite("payload.fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_int_del",cpid);
      uhalWrite("payload.fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_ext_del",cped);


      uhalWrite("payload.fc_ctrl.fpga_fc.ctrl.l1a_stretch",0);
      uhalWrite("payload.fc_ctrl.fpga_fc.calpulse_ctrl.ocr_n",8);

      // fc_lpgbt_pair

      uhalWrite("payload.fc_ctrl.fc_lpgbt_pair.ctrl.calpulse_type",1);
      uhalWrite("payload.fc_ctrl.fc_lpgbt_pair.ctrl.user_bx",1);
      uhalWrite("payload.fc_ctrl.fc_lpgbt_pair.fc_cmd.user",0x36);

      return true;
    }  

    void print(std::ostream &o=std::cout) {
      o << "SerenityEncoder::print()" << std::endl;
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
