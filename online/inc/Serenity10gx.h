#ifndef Hgcal10gLinkReceiver_Serenity10gx_h
#define Hgcal10gLinkReceiver_Serenity10gx_h

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

  class Serenity10gx : public SerenityUhal {
    
  public:
  
    Serenity10gx() {
    }
    
    virtual ~Serenity10gx() {
    }
    
    bool makeTable() {
      SerenityUhal::makeTable("payload");
      return true;
    }
  
    bool setDefaults() {
      /*
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_physics",0);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_random",0);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_software",1);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_regular",0);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_reg",0);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_rand",0);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_physics",0);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.l1a_prbs_threshold",0xf000);

      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",0);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_mask",0x7);
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.l1a_physics_mask",0xff);
      //uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_all",1);
      //uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_ext",1);
      //uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_hgcroc",1);
     
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",30);
    
      uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length",1);
      uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",0);
      uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.data",(1<<16)|0x0040);
      */
      return true;
    }  

    void configuration(std::vector<uint32_t> &v) {
      v.resize(0);
      /*
      // Control words
      v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl"));
      v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl1"));
      v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2"));
      v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl3"));

      // Sequencer 
      uint32_t length(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length"));
      uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",0);
      for(unsigned i(0);i<length;i++) {
        v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.seq_mem.data"));
      }
      */
    }  

    void print(std::ostream &o=std::cout) {
      o << "Serenity10gx::print()" << std::endl;
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
