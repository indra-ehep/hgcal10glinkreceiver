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

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityUhal {
    
  public:
  
    static void setUhalLogLevel() {
#ifdef ProcessorHardware
      uhal::setLogLevelTo(uhal::Error());  
#endif
    }

#ifdef ProcessorHardware
  SerenityUhal() : lConnectionFilePath("etc/connections.xml"),
      lDeviceId("x0"),
      lConnectionMgr("file://" + lConnectionFilePath),
      lHW(lConnectionMgr.getDevice(lDeviceId)) {
      _printEnable=true;
    }
#else
    SerenityUhal() {
    }
#endif
    
    virtual ~SerenityUhal() {
    }
    
    bool makeTable(const std::string &s="payload") {
#ifdef ProcessorHardware
      std::vector<std::string> temp;
      temp=lHW.getNode(s).getNodes();
      
      if(_printEnable) {
	for(unsigned i(0);i<temp.size();i++) {
	  std::cout << "ALL string " << temp[i] << std::endl;
	}
	std::cout << std::endl;
      }

      _uhalString.resize(0);
      for(unsigned i(0);i<temp.size();i++) {
	if(s=="payload") {
	  if(temp[i].substr(0,8)=="fc_ctrl.") {
	    std::cout << "Substr = " << temp[i].substr(8,17) << std::endl;
	    //if(temp[i].substr(8,17)!="tcds2_emu") {
	    std::cout << "UHAL string " << std::setw(3) << " = " 
		      << temp[i] << std::endl;
	    _uhalString.push_back(temp[i]);
	    //}
	  }
	} else {
	  _uhalString.push_back(s+"."+temp[i]);
	}
      }
    
      if(_printEnable) {
	for(unsigned i(0);i<_uhalString.size();i++) {
	  std::cout << "UHAL string " << std::setw(3) << " = " 
		    << _uhalString[i] << std::endl;
	
	  const uhal::Node& lNode = lHW.getNode(_uhalString[i]);
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
      
      uhalWrite("fc_ctrl.fpga_fc.ctrl.tts",1);
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

     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_tcds2",0x1);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.tts_mask",0x7);
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl2.l1a_physics_mask",0xff);
     //uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_all",1);
     //uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_ext",1);
     //uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.stat0.tts_hgcroc",1);
     
     uhalWrite("fc_ctrl.tcds2_emu.ctrl_stat.ctrl.calpulse_delay",12+cpid+hgcrocLatencyBufferDepth);
    

      return true;
    }  

#ifdef ProcessorHardware
  uint32_t uhalRead(const std::string &s) {
    //const uhal::Node& lNode = lHW.getNode(std::string("payload.")+s);
    const uhal::Node& lNode = lHW.getNode(s);
    uhal::ValWord<uint32_t> lReg = lNode.read();
    lHW.dispatch();

    std::cout << "uhalRead: reading " << s << " as  0x"
	      << std::hex << std::setfill('0')
	      << std::setw(8) << lReg.value()
	      << std::dec << std::setfill(' ')
	      << std::endl;

    return lReg.value();
  }
  
  bool uhalWrite(const std::string &s, uint32_t v) {
    std::cout << "uhalWrite: setting " << s << " to  0x"
	      << std::hex << std::setfill('0')
	      << std::setw(8) << v
	      << std::dec << std::setfill(' ')
	      << std::endl;
    
    //const uhal::Node& lNode = lHW.getNode(std::string("payload.")+s);
    const uhal::Node& lNode = lHW.getNode(s);
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

  void csv(std::ostream &o=std::cout) {
    for(unsigned i(0);i<_uhalString.size();i++) {
      uint32_t v(uhalRead(_uhalString[i]));
      unsigned nComma(0);
      for(unsigned j(0);j<_uhalString[i].size();j++) {
	if(_uhalString[i][j]=='.') {
	  _uhalString[i][j]=',';
	  nComma++;
	}
      }
      o << _uhalString[i];
      for(unsigned j(nComma);j<5;j++) {
	o << ',';
      }
      o << v << std::endl;
    }
  }

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
  bool _printEnable;
  std::vector<std::string> _uhalString;

#ifdef ProcessorHardware
  const std::string lConnectionFilePath;
  const std::string lDeviceId;
  uhal::ConnectionManager lConnectionMgr;
  uhal::HwInterface lHW;
#endif

};

}

#endif
