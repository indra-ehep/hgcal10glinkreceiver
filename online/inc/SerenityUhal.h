#ifndef SerenityUhal_h
#define SerenityUhal_h

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

#undef SerenityUhalHardware
//#define SerenityUhalHardware

#ifdef SerenityUhalHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityUhal {
    
  public:
  
    static void setUhalLogLevel() {
#ifdef SerenityUhalHardware
      uhal::setLogLevelTo(uhal::Error());  
#endif
    }

#ifdef SerenityUhalHardware
  SerenityUhal() : lConnectionFilePath("etc/connections.xml"),
      lDeviceId("x0"),
      lConnectionMgr("file://" + lConnectionFilePath),
      lHW(lConnectionMgr.getDevice(lDeviceId)) {
    }
#else
    SerenityUhal() {
    }
#endif
    
    virtual ~SerenityUhal() {
    }
    
    bool makeTable() {
#ifdef SerenityUhalHardware
      std::vector<std::string> temp;
      temp=lHW.getNode("payload").getNodes();
      
      if(_printEnable) {
	for(unsigned i(0);i<temp.size();i++) {
	  std::cout << "ALL string " << temp[i] << std::endl;
	}
	std::cout << std::endl;
      }

      _uhalString.resize(0);
      for(unsigned i(0);i<temp.size();i++) {
	if(temp[i].substr(0,8)=="fc_ctrl.") {
	  std::cout << "Substr = " << temp[i].substr(8,17) << std::endl;
	  if(temp[i].substr(8,17)!="tcds2_emu") {
	    std::cout << "UHAL string " << std::setw(3) << " = " 
		      << temp[i] << std::endl;
	    _uhalString.push_back(temp[i]);
	  }
	}
      }
    
      if(_printEnable) {
	for(unsigned i(0);i<_uhalString.size();i++) {
	  std::cout << "UHAL string " << std::setw(3) << " = " 
		    << _uhalString[i] << std::endl;
	
	  const uhal::Node& lNode = lHW.getNode("payload."+_uhalString[i]);
	  uhal::ValWord<uint32_t> lReg = lNode.read();
	  lHW.dispatch();
	
	  std::cout << "UHAL string " << std::setw(3) << " = "
		    << _uhalString[i] << ", initial value = " 
		    << lReg.value() << std::endl;
	}
	std::cout << std::endl;
      }
#else
      _uhalString.push_back("BLAH");
#endif
      return true;
    }
  
    bool setDefaults() {
      
      /*
	
	python3 hgcal_fc.py -c test_stand/connections.xml -d x0 encoder configure -tts on = fc_ctrl.fpga_fc.ctrl.tts
	python3 hgcal_fc.py -c test_stand/connections.xml -d x0 encoder configure --set_prel1a_offset 3 = fc_ctrl.fpga_fc.ctrl.prel1a_offset
	fc_ctrl.fpga_fc.ctrl.user_prel1a_off_en
	
	python3 hgcal_fc.py -c test_stand/connections.xml -d x0 tcds2_emu configure --seq_disable
	
	fc_ctrl.fc_lpgbt_pair.ctrl.issue_linkrst
	fc_ctrl.fc_lpgbt_pair.fc_cmd.linkrst
      
      */
      
      std::cout << "SETTING SOME DEFAULTS" << std::endl;

      // fpga_fc
      
      uhalWrite("fc_ctrl.fpga_fc.ctrl.tts",0);
      uhalWrite("fc_ctrl.fpga_fc.ctrl.prel1a_offset",3);
      uhalWrite("fc_ctrl.fpga_fc.ctrl.user_prel1a_off_en",1);
      
      uint32_t hgcrocLatencyBufferDepth(10); // ????
      uint32_t cpid(8),cped(10); // ???
      uhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_int_del",cpid);
      uhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.calpulse_ext_del",cped);


     uhalWrite("fc_ctrl.fpga_fc.ctrl.l1a_stretch",0);
     uhalWrite("fc_ctrl.fpga_fc.calpulse_ctrl.ocr_n",8);

     // fc_lpgbt_pair

     uhalWrite("fc_ctrl.fc_lpgbt_pair.ctrl.calpulse_type",1);
     uhalWrite("fc_ctrl.fc_lpgbt_pair.ctrl.user_bx",1);
     uhalWrite("fc_ctrl.fc_lpgbt_pair.fc_cmd.user",0x36);

     // tcds2_emu
     
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_physics",0);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_random",0);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_software",1);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.en_l1a_regular",0);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_reg",0);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_rand",0);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.en_nzs_physics",0);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl1.l1a_prbs_threshold",0xf000);

     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.l1a_physics_mask",0xffffffff);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_mask",0xffffffff);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_all",0xffffffff);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_ext",0xffffffff);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_hgcroc",0xffffffff);
     uhalWrite("",);
     uhalWrite("",);
     uhalWrite("",);
     uhalWrite("",);
     uhalWrite("",);
     uhalWrite("",);
     uhalWrite("",);
     uhalWrite("",);



     

XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2, initial value = 0
XHAL string  = 
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_mask, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.l1a_physics_mask
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.l1a_physics_mask, initial value = 0
xhal string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl3
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl3, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl3.rst_counters
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.ctrl3.rst_counters, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat0
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat0, initial value = 3
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_all
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_all, initial value = 1
XHAL string  = 
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_ext, initial value = 1
XHAL string  = 
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_hgcroc, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat1
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat1, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat2
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat2, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat3
XHAL string  = fc_ctrl.tcds2_emu.ctrl_stat.stat3, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.seq_mem
XHAL string  = fc_ctrl.tcds2_emu.seq_mem, initial value = 1
XHAL string  = fc_ctrl.tcds2_emu.seq_mem.pointer
XHAL string  = fc_ctrl.tcds2_emu.seq_mem.pointer, initial value = 1
XHAL string  = fc_ctrl.tcds2_emu.seq_mem.data
XHAL string  = fc_ctrl.tcds2_emu.seq_mem.data, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters
XHAL string  = fc_ctrl.tcds2_emu.counters, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_physics
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_physics, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_random
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_random, initial value = dcf7ad6c
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_software
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_software, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_regular
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_regular, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_subtype0
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_subtype0, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_subtype1
XHAL string  = fc_ctrl.tcds2_emu.counters.l1a_subtype1, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters.tts_all
XHAL string  = fc_ctrl.tcds2_emu.counters.tts_all, initial value = 1
XHAL string  = fc_ctrl.tcds2_emu.counters.tts_soft
XHAL string  = fc_ctrl.tcds2_emu.counters.tts_soft, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters.tts_hgcroc
XHAL string  = fc_ctrl.tcds2_emu.counters.tts_hgcroc, initial value = 0
XHAL string  = fc_ctrl.tcds2_emu.counters.tts_ext


     
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",12+cpid+hgcrocLatencyBufferDepth);
    

      return true;
    }  

#ifdef SerenityUhalHardware
  uint32_t uhalRead(const std::string &s) {
    const uhal::Node& lNode = lHW.getNode(std::string("payload.")+s);
    uhal::ValWord<uint32_t> lReg = lNode.read();
    lHW.dispatch();
    return lReg.value();
  }
  
  bool uhalWrite(const std::string &s, uint32_t v) {
    std::cout << "uhalWrite: setting " << s << " to  0x"
	      << std::hex << std::setfill('0')
	      << std::setw(8) << v
	      << std::dec << std::setfill(' ')
	      << std::endl;
    
    const uhal::Node& lNode = lHW.getNode(std::string("payload.")+s);
    lNode.write(v);
    lHW.dispatch();
    
    return uhalRead(s)==v;
  }

#else

  uint32_t uhalRead(const std::string &s) {
    return 999;
  }

  bool uhalWrite(const std::string &s, uint32_t v) {
    std::cout << "uhalWrite: setting " << s << " to  0x"
	      << std::hex << std::setfill('0')
	      << std::setw(8) << v
	      << std::dec << std::setfill(' ')
	      << std::endl;
    return true;
  }

#endif

  void print(std::ostream &o=std::cout) {
    o << "SerenityUhal::print()" << std::endl;
    o << " Current settings for " << _uhalString.size()
      << " values:" << std::endl;

    o << std::hex << std::setfill('0');

    for(unsigned i(0);i<_uhalString.size();i++) {
      std::cout << "  " << _uhalString[i] << " = 0x"
		<< std::setw(8) << uhalRead(_uhalString[i])
		<< std::endl;
    }
    o << std::dec << std::setfill(' ');
  }

  
 protected:
  std::vector<std::string> _uhalString;

#ifdef SerenityUhalHardware
  const std::string lConnectionFilePath;
  const std::string lDeviceId;
  uhal::ConnectionManager lConnectionMgr;
  uhal::HwInterface lHW;
#endif

};

}

#endif
