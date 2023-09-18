#ifndef Hgcal10gLinkReceiver_SerenityRegs_h
#define Hgcal10gLinkReceiver_SerenityRegs_h

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

namespace Hgcal10gLinkReceiver {

  class SerenityRegs : public SerenityUhal {
    
  public:
  
    SerenityRegs() {
    }
    
    virtual ~SerenityRegs() {
    }

    /*

// Not to be touched
|    |    |    ├──payload.reg_320.ctrl0.eb_rd_ack
|    |    |    ├──payload.reg_320.ctrl2.buffer_rst
|    |    |    ├──payload.reg_320.ctrl2.buffer_en

// MiniDaq
|    |    |    ├──payload.reg_320.ctrl0.daq_readout_rst
|    |    |    ├──payload.reg_320.ctrl0.trig_readout_rst
|    |    |    ├──payload_ctrl_stat.ctrl0.throttle_en_miniDAQ

// 10G (only)
|    |    |    ├──payload.reg_320.ctrl3.pkt_counters_rst
|    |    |    ├──payload.reg_320.ctrl1.slink_daq_readout_bp_en
|    |    |    ├──payload.reg_320.ctrl1.slink_trig_readout_bp_en
|    |    |    ├──payload.reg_320.ctrl1.rate_throttle_daq_en
|    |    |    ├──payload.reg_320.ctrl1.rate_throttle_trig_en

// TCDS2
|    |    |    ├──payload.reg_320.ctrl1.ext_trigger_delay
|    |    |    ├──payload.reg_320.ctrl1.ext_trigger_window
|    |    |    ├──payload.reg_320.ctrl1.l1a_throttle_user

    */

    uint32_t uhalReg040Read(const std::string &s) {
      return uhalTopRead("payload.payload_ctrl_stat."+s);
    }

    bool uhalReg040Write(const std::string &s, uint32_t v) {
      return uhalTopWrite("payload.payload_ctrl_stat."+s,v);
    }

    void sendReg040Pulse(const std::string &s) {
      sendPulse(std::string("payload_ctrl_stat.")+s);
    }  

    uint32_t uhalReg320Read(const std::string &s) {
      return uhalTopRead("payload.reg_320."+s);
    }

    bool uhalReg320Write(const std::string &s, uint32_t v) {
      return uhalTopWrite("payload.reg_320."+s,v);
    }

    void sendReg320Pulse(const std::string &s) {
      sendPulse(std::string("reg_320.")+s);
    }  

    uint32_t uhalCounterRead(const std::string &s) {
      return uhalTopRead("payload.counters."+s);
    }
  };

}

#endif
