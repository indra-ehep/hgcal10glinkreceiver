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
      SerenityUhal::makeTable("payload.eth");
      return true;
    }
  
    bool setDefaults() {
      uhalWrite("ctrl.reg.en",0);
      uhalWrite("ctrl.reg.duty_cycle",3);
      uhalWrite("ctrl.pkt_len",0x20);
      uhalWrite("ctrl.reg.decrement_len",0);
      uhalWrite("ctrl.pause_interval",0);

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
