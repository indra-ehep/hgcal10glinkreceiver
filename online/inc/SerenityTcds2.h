#ifndef SerenityTcds2_h
#define SerenityTcds2_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "I2cInstruction.h"
#include "UhalInstruction.h"

#ifdef SerenityTcds2Hardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityTcds2 : public SerenityUhal {
    
  public:
  
    SerenityTcds2() {
    }
    
    virtual ~SerenityTcds2() {
    }
    
    bool makeTable() {
      SerenityUhal::makeTable("payload.fc_ctrl.tcds2_emu");
      return true;
    }
  
    bool setDefaults() {
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_physics",0);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_random",0);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_software",1);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_regular",0);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_reg",0);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_rand",0);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_physics",0);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.l1a_prbs_threshold",0xf000);

      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",0);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_mask",0x7);
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.l1a_physics_mask",0xff);
      //uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_all",1);
      //uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_ext",1);
      //uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_hgcroc",1);
     
      uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",30);
    
      return true;
    }  

    void print(std::ostream &o=std::cout) {
      o << "SerenityTcds2::print()" << std::endl;
      o << " Current settings for " << _uhalString.size()
	<< " values:" << std::endl;

      o << std::hex << std::setfill('0');

      for(unsigned i(0);i<_uhalString.size();i++) {
	std::cout << "  " << _uhalString[i] << " = 0x"
		  << std::setw(8) << uhalRead(_uhalString[i])
		  << std::endl;
      }
      o << std::dec << std::setfill(' ');

      uhalWrite("fc_ctrl.tcds2_emu.seq_mem.pointer",0);
      uint32_t seqLength=uhalRead("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length");
      for(unsigned i(0);i<seqLength;i++) {
	uint32_t seqMemPointer=uhalRead("fc_ctrl.tcds2_emu.seq_mem.pointer");
	uint32_t seqMemData=uhalRead("fc_ctrl.tcds2_emu.seq_mem.data");
	std::cout << "Seq l, mp, md = " << seqLength << ", " << seqMemPointer
		  << ", 0x" << std::hex << std::setfill('0') 
		  << std::setw(8) << seqMemData 
		  << std::dec << std::setfill(' ')
		  << " meaning BX = " << (seqMemData>>16)
		  << std::endl;
      }
    }

  
  protected:

  };

}

#endif
