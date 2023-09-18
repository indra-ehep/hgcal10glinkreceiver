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
      //SerenityUhal::makeTable();
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

      // Cludge these in here for now; move them later
      m["payload_ctrl_stat.ctrl0.fc_dec_force_lock"   ]=uhalRead("payload_ctrl_stat.ctrl0.fc_dec_force_lock",true);
      m["payload_ctrl_stat.ctrl0.fc_dec_prel1a_offset"]=uhalRead("payload_ctrl_stat.ctrl0.fc_dec_prel1a_offset",true);

#ifdef DthHardware
#else
      m["DAQ_SLink_readout0.source_id"]=uhalRead("DAQ_SLink_readout.source_id",true);
      m["DAQ_SLink_readout1.source_id"]=uhalRead("DAQ_SLink_readout1.source_id",true);
#endif
    }
    /*
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
    */
    void status(YAML::Node &m) {
      m=YAML::Node();
      
      m["l1a_counter_l"]=uhalRead("l1a_counter_l");
      m["l1a_counter_h"]=uhalRead("l1a_counter_h");
      m["orb_counter"  ]=uhalRead("orb_counter");
    }

    void sendPulse(const std::string &s, bool inverted=false) {
      if(!inverted) {
	if(uhalRead(s)==1) uhalWrite(s,0);
	uhalWrite(s,1);
	uhalWrite(s,0);

      } else {
	if(uhalRead(s)==0) uhalWrite(s,1);
	uhalWrite(s,0);
	uhalWrite(s,1);
      }
    }

    void resetSlinkFifo() {
      sendPulse("ctrl_3.rst_slink_fifo",true);
      //uhalWrite("ctrl_3.rst_slink_fifo",1);
      //uhalWrite("ctrl_3.rst_slink_fifo",0);
    }  
    /*
    void resetDaqReadout() {
      uhalWrite("reg_320.ctrl0.daq_readout_rst",1,true);
      uhalWrite("reg_320.ctrl0.daq_readout_rst",0,true);
    }  

    void resetTrgReadout() {
      uhalWrite("reg_320.ctrl0.trig_readout_rst",1,true);
      uhalWrite("reg_320.ctrl0.trig_readout_rst",0,true);
    }  
    */
    bool setDefaults() {

      // Zero all fundamental values
      uhalWrite("ctrl",0);
      uhalWrite("calpulse_ctrl",0);
      uhalWrite("ctrl_2",0);
      //uhalWrite("ctrl_3",0);

      // Set specific values
      uhalWrite("ctrl.tts",0);
      uhalWrite("ctrl.prel1a_offset",3);
      uhalWrite("ctrl.user_prel1a_off_en",1);
      uhalWrite("ctrl.l1a_stretch",0);

      uhalWrite("ctrl_3.rst_slink_fifo",1);
      
      uhalWrite("calpulse_ctrl.calpulse_int_del",8);
      uhalWrite("calpulse_ctrl.calpulse_ext_del",10);
      uhalWrite("calpulse_ctrl.ocr_n",8);

      // Cludge these in here for now; move them later
      uhalWrite("payload_ctrl_stat.ctrl0.fc_dec_force_lock",0,true);
      uhalWrite("payload_ctrl_stat.ctrl0.fc_dec_prel1a_offset",2,true);

      uhalWrite("reg_320.ctrl0.daq_readout_rst",0,true);
      uhalWrite("reg_320.ctrl0.trig_readout_rst",0,true);

      //uhalWrite("DAQ_SLink_readout.source_id",0xce000000|daqBoard<<4|1,true);

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
