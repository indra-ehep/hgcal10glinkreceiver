#ifndef Hgcal10gLinkReceiver_SerenityTcds2_h
#define Hgcal10gLinkReceiver_SerenityTcds2_h

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

#include "SerenityUhal.h"
#include "I2cInstruction.h"
#include "UhalInstruction.h"

#ifdef ProcessorHardware
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
      return SerenityUhal::makeTable("payload.fc_ctrl.tcds2_emu");
    }

    void configuration(YAML::Node &m) {
      m=YAML::Node();

      m["ctrl_stat.ctrl.en_l1a_physics"]=uhalRead("ctrl_stat.ctrl.en_l1a_physics");
      m["ctrl_stat.ctrl.en_l1a_random"]=uhalRead("ctrl_stat.ctrl.en_l1a_random");
      m["ctrl_stat.ctrl.en_l1a_software"]=uhalRead("ctrl_stat.ctrl.en_l1a_software");
      m["ctrl_stat.ctrl.en_l1a_regular"]=uhalRead("ctrl_stat.ctrl.en_l1a_regular");
      m["ctrl_stat.ctrl.calpulse_delay"]=uhalRead("ctrl_stat.ctrl.calpulse_delay");

      m["ctrl_stat.ctrl1.en_nzs_reg"]=uhalRead("ctrl_stat.ctrl1.en_nzs_reg");
      m["ctrl_stat.ctrl1.en_nzs_rand"]=uhalRead("ctrl_stat.ctrl1.en_nzs_rand");
      m["ctrl_stat.ctrl1.en_nzs_physics"]=uhalRead("ctrl_stat.ctrl1.en_nzs_physics");
      m["ctrl_stat.ctrl1.l1a_prbs_threshold"]=uhalRead("ctrl_stat.ctrl1.l1a_prbs_threshold");

      m["ctrl_stat.ctrl2.tts_tcds2"]=uhalRead("ctrl_stat.ctrl2.tts_tcds2");
      m["ctrl_stat.ctrl2.tts_mask"]=uhalRead("ctrl_stat.ctrl2.tts_mask");
      m["ctrl_stat.ctrl2.l1a_physics_mask"]=uhalRead("ctrl_stat.ctrl2.l1a_physics_mask");

      uint32_t length(uhalRead("ctrl_stat.ctrl.seq_length"));
      m["ctrl_stat.ctrl.seq_length"]=length;
      //m["seq_mem.pointer"]=uhalRead("seq_mem.pointer");

      for(unsigned i(0);i<length && i<1024;i++) {
	std::ostringstream sout;
        sout << "seq_mem.data_" << std::setfill('0') << std::setw(4) << i;
        m[sout.str()]=uhalRead("seq_mem.data");
      }
    }

    void configuration(std::unordered_map<std::string,uint32_t> &m) {
      m.clear();

      m["ctrl_stat.ctrl.en_l1a_physics"]=uhalRead("ctrl_stat.ctrl.en_l1a_physics");
      m["ctrl_stat.ctrl.en_l1a_random"]=uhalRead("ctrl_stat.ctrl.en_l1a_random");
      m["ctrl_stat.ctrl.en_l1a_software"]=uhalRead("ctrl_stat.ctrl.en_l1a_software");
      m["ctrl_stat.ctrl.en_l1a_regular"]=uhalRead("ctrl_stat.ctrl.en_l1a_regular");
      m["ctrl_stat.ctrl.calpulse_delay"]=uhalRead("ctrl_stat.ctrl.calpulse_delay");

      m["ctrl_stat.ctrl1.en_nzs_reg"]=uhalRead("ctrl_stat.ctrl1.en_nzs_reg");
      m["ctrl_stat.ctrl1.en_nzs_rand"]=uhalRead("ctrl_stat.ctrl1.en_nzs_rand");
      m["ctrl_stat.ctrl1.en_nzs_physics"]=uhalRead("ctrl_stat.ctrl1.en_nzs_physics");
      m["ctrl_stat.ctrl1.l1a_prbs_threshold"]=uhalRead("ctrl_stat.ctrl1.l1a_prbs_threshold");

      m["ctrl_stat.ctrl2.tts_tcds2"]=uhalRead("ctrl_stat.ctrl2.tts_tcds2");
      m["ctrl_stat.ctrl2.tts_mask"]=uhalRead("ctrl_stat.ctrl2.tts_mask");
      m["ctrl_stat.ctrl2.l1a_physics_mask"]=uhalRead("ctrl_stat.ctrl2.l1a_physics_mask");

      m["ctrl_stat.ctrl.seq_length"]=uhalRead("ctrl_stat.ctrl.seq_length");
      //m["seq_mem.pointer"]=uhalRead("seq_mem.pointer");
      
      for(unsigned i(0);i<m["ctrl_stat.ctrl.seq_length"] && i<1;i++) {
	std::ostringstream sout;
	sout << "seq_mem.data_" << std::setfill('0') << std::setw(4) << i;
	m[sout.str()]=uhalRead("seq_mem.data");
      }
    }

    void status(std::unordered_map<std::string,uint32_t> &m) {
      m.clear();

      m["ctrl_stat.stat0.tts_all"]=uhalRead("ctrl_stat.stat0.tts_all");
      m["ctrl_stat.stat0.tts_ext"]=uhalRead("ctrl_stat.stat0.tts_ext");
      m["ctrl_stat.stat0.tts_hgcroc"]=uhalRead("ctrl_stat.stat0.tts_hgcroc");
    }
  
    bool setDefaults() {
      uhalWrite("ctrl_stat.ctrl.en_l1a_physics",0);
      uhalWrite("ctrl_stat.ctrl.en_l1a_random",0);
      uhalWrite("ctrl_stat.ctrl.en_l1a_software",1);
      uhalWrite("ctrl_stat.ctrl.en_l1a_regular",0);
      uhalWrite("ctrl_stat.ctrl.calpulse_delay",30);

      uhalWrite("ctrl_stat.ctrl1.en_nzs_reg",0);
      uhalWrite("ctrl_stat.ctrl1.en_nzs_rand",0);
      uhalWrite("ctrl_stat.ctrl1.en_nzs_physics",0);
      uhalWrite("ctrl_stat.ctrl1.l1a_prbs_threshold",0xf000);

      uhalWrite("ctrl_stat.ctrl2.tts_tcds2",0);
      uhalWrite("ctrl_stat.ctrl2.tts_mask",0x7);
      uhalWrite("ctrl_stat.ctrl2.l1a_physics_mask",0xff);

      //uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_all",1);
      //uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_ext",1);
      //uhalWrite("payload.fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_hgcroc",1);
     
      uhalWrite("ctrl_stat.ctrl.seq_length",1);
      uhalWrite("seq_mem.pointer",0);
      uhalWrite("seq_mem.data",(1<<16)|0x0040);

      return true;
    }  

    void sendPulse(const std::string &s) {
      if(uhalRead(s)==1) uhalWrite(s,0);
      uhalWrite(s,1);
      uhalWrite(s,0);
    }  

    void sendOcr() {
      sendPulse("ctrl_stat.ctrl1.arm_ocr");
    }  

    void sendEbr() {
      sendPulse("ctrl_stat.ctrl1.arm_ebr");
    }  

    void sendEcr() {
      sendPulse("ctrl_stat.ctrl1.arm_ecr");
    }  

    void configuration(std::vector<uint32_t> &v) {
      v.resize(0);

      // Control words
      v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl"));
      v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl1"));
      v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl2"));
      v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl3"));

      // Sequencer 
      //uint32_t length(uhalRead("payload.fc_ctrl.tcds2_emu.ctrl_stat.ctrl.seq_length"));
      uint32_t length(uhalRead("ctrl_stat.ctrl.seq_length"));
      //uhalWrite("payload.fc_ctrl.tcds2_emu.seq_mem.pointer",0);
      uhalWrite("seq_mem.pointer",0);
      for(unsigned i(0);i<length;i++) {
        //v.push_back(uhalRead("payload.fc_ctrl.tcds2_emu.seq_mem.data"));
        v.push_back(uhalRead("seq_mem.data"));
      }
    }  

    void print(std::ostream &o=std::cout) {
      o << "SerenityTcds2::print()" << std::endl;
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

      uhalWrite("seq_mem.pointer",0);
      uint32_t seqLength(uhalRead("ctrl_stat.ctrl.seq_length"));
      if(seqLength==999) seqLength=0; // CLUDGE

      for(unsigned i(0);i<seqLength;i++) {
	uint32_t seqMemPointer=uhalRead("seq_mem.pointer");
	uint32_t seqMemData=uhalRead("seq_mem.data");
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
