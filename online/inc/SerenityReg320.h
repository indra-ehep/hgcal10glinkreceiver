#ifndef Hgcal10gLinkReceiver_SerenityReg320_h
#define Hgcal10gLinkReceiver_SerenityReg320_h

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

namespace Hgcal10gLinkReceiver {

  class SerenityReg320 : public SerenityUhal {
    
  public:
  
    SerenityReg320() {
    }
    
    virtual ~SerenityReg320() {
    }
    
    bool makeTable() {
      SerenityUhal::makeTable("payload.reg_320");
      return true;
    }
  
    bool setDefaults() {
      /*
      uhalWrite("ctrl_stat.ctrl0.nelinks_loc",4);
      uhalWrite("ctrl_stat.ctrl0.trig_threshold",127);
      uhalWrite("ctrl_stat.ctrl0.hist_sel",histSel_);
      uhalWrite("ctrl_stat.ctrl1.bx_latency",0);
      uhalWrite("ctrl_stat.ctrl1.l1a_delay",0);
      */
      return true;
    }  

    /*

// Not to be touched
|    |    |    ├──payload.reg_320.ctrl0.eb_rd_ack
|    |    |    ├──payload.reg_320.ctrl2.buffer_rst
|    |    |    ├──payload.reg_320.ctrl2.buffer_en

// Encoder -> MiniDaq?
|    |    |    ├──payload.reg_320.ctrl0.daq_readout_rst
|    |    |    ├──payload.reg_320.ctrl0.trig_readout_rst

// Fake spill -> TCDS2
|    |    |    ├──payload.reg_320.ctrl1.l1a_throttle_user

// 10G (!!!)
// Reset for counters
|    |    |    ├──payload.reg_320.ctrl3.pkt_counters_rst

|    |    |    ├──payload.reg_320.ctrl1.slink_daq_readout_bp_en
|    |    |    ├──payload.reg_320.ctrl1.slink_trig_readout_bp_en
|    |    |    ├──payload.reg_320.ctrl1.rate_throttle_daq_en
|    |    |    ├──payload.reg_320.ctrl1.rate_throttle_trig_en

// TCDS2
|    |    |    ├──payload.reg_320.ctrl1.ext_trigger_delay
|    |    |    ├──payload.reg_320.ctrl1.ext_trigger_window


payload_ctrl_stat.ctrl0.throttle_en_miniDAQ
    */

    void configuration(YAML::Node &m) {
      m=YAML::Node();
      
      //m["ctrl_stat.ctrl0.nelinks_loc"   ]=uhalRead("ctrl_stat.ctrl0.nelinks_loc");

    }

    void status(YAML::Node &m) {
      m=YAML::Node();
      
      //m["ctrl_stat.stat.lock"       ]=uhalRead("ctrl_stat.stat.lock");

    }

    void monitoring(YAML::Node &m) {
      m=YAML::Node();

      m["stat0"                        ]=uhalRead("stat0");
      m["stat1"                        ]=uhalRead("stat1");
      m["stat2.buffer_full"            ]=uhalRead("stat2.buffer_full");

      m["stat3.ch0_trg_backpressure"   ]=uhalRead("stat3.ch0_trg_backpressure");
      m["stat3.ch1_daq_backpressure"   ]=uhalRead("stat3.ch1_daq_backpressure");
      m["stat3.ch2_daq1_backpressure"  ]=uhalRead("stat3.ch2_daq1_backpressure");
      m["stat3.mDAQ_evbuf_empty"       ]=uhalRead("stat3.mDAQ_evbuf_empty");
      m["stat3.mDAQ1_evbuf_empty"      ]=uhalRead("stat3.mDAQ1_evbuf_empty");
      m["stat3.mDAQ_packet_in_buffer"  ]=uhalRead("stat3.mDAQ_packet_in_buffer");
      m["stat3.mDAQ1_packet_in_buffer" ]=uhalRead("stat3.mDAQ1_packet_in_buffer");
      m["stat3.trg_eth_pktsize_error"  ]=uhalRead("stat3.trg_eth_pktsize_error");
      m["stat3.mDAQ_eth_pktsize_error" ]=uhalRead("stat3.mDAQ_eth_pktsize_error");
      m["stat3.mDAQ1_eth_pktsize_error"]=uhalRead("stat3.mDAQ1_eth_pktsize_error");
      m["stat3.mDAQ_pktsize_error"     ]=uhalRead("stat3.mDAQ_pktsize_error");
      m["stat3.mDAQ_big_pkt_size"      ]=uhalRead("stat3.mDAQ_big_pkt_size");
    }

    uint32_t uhalReg040Read(const std::string &s) {
      return uhalTopRead("payload.payload_ctrl_stat."+s);
    }

    bool uhalReg040Write(const std::string &s, uint32_t v) {
      return uhalTopRead("payload.payload_ctrl_stat."+s,v);
    }

    void sendReg040Pulse(const std::string &s) {
      sendPulse(std::string("payload_ctrl_stat.")+s);
    }  

    uint32_t uhalReg320Read(const std::string &s) {
      return uhalTopRead("payload.reg320."+s);
    }

    bool uhalReg320Write(const std::string &s, uint32_t v) {
      return uhalTopRead("payload.reg320."+s,v);
    }

    void sendReg320Pulse(const std::string &s) {
      sendPulse(std::string("reg_320.")+s);
    }  

    void print(std::ostream &o=std::cout) {
      o << "SerenityReg320::print()" << std::endl;
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
    unsigned histSel_;
  };

}

#endif
