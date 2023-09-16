#ifndef Hgcal10gLinkReceiver_SerenityUnpacker_h
#define Hgcal10gLinkReceiver_SerenityUnpacker_h

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

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityUnpacker : public SerenityUhal {
    
  public:
  
    SerenityUnpacker() {
      histSel_=0;
    }
    
    virtual ~SerenityUnpacker() {
    }
    
    bool makeTable(const std::string &s="") {
      SerenityUhal::makeTable(std::string("payload.unpacker")+s);
      return true;
    }
  
    bool setDefaults() {
      uhalWrite("ctrl_stat.ctrl0.nelinks_loc",4);
      uhalWrite("ctrl_stat.ctrl0.trig_threshold",127);
      uhalWrite("ctrl_stat.ctrl0.hist_sel",histSel_);
      uhalWrite("ctrl_stat.ctrl1.bx_latency",0);
      uhalWrite("ctrl_stat.ctrl1.l1a_delay",0);
      
      histSel_=(histSel_+1)%4;

      return true;
    }  

    void configuration(YAML::Node &m) {
      m=YAML::Node();
      
      m["ctrl_stat.ctrl0.nelinks_loc"   ]=uhalRead("ctrl_stat.ctrl0.nelinks_loc");
      m["ctrl_stat.ctrl0.trig_threshold"]=uhalRead("ctrl_stat.ctrl0.trig_threshold");
      m["ctrl_stat.ctrl1.bx_latency"    ]=uhalRead("ctrl_stat.ctrl1.bx_latency");
      m["ctrl_stat.ctrl1.l1a_delay"     ]=uhalRead("ctrl_stat.ctrl1.l1a_delay");
      m["ctrl_stat.ctrl0.hist_sel"      ]=uhalRead("ctrl_stat.ctrl0.hist_sel");
    }

    void status(YAML::Node &m) {
      m=YAML::Node();
      
      m["ctrl_stat.stat.lock"       ]=uhalRead("ctrl_stat.stat.lock");
      m["ctrl_stat.stat.lock_type"  ]=uhalRead("ctrl_stat.stat.lock_type");
      m["ctrl_stat.stat.bc0_lock_bx"]=uhalRead("ctrl_stat.stat.bc0_lock_bx");

      m["ctrl_stat.lock_unlock_event.lock_bx"  ]=uhalRead("ctrl_stat.lock_unlock_event.lock_bx");
      m["ctrl_stat.lock_unlock_event.unlock_bx"]=uhalRead("ctrl_stat.lock_unlock_event.unlock_bx");

      m["counters.lock_l"           ]=uhalRead("counters.lock_l");
      m["counters.lock_h"           ]=uhalRead("counters.lock_h");
      m["counters.ms_invalid_l"     ]=uhalRead("counters.ms_invalid_l");
      m["counters.ms_invalid_h"     ]=uhalRead("counters.ms_invalid_h");
      m["counters.bc0_latency_err_l"]=uhalRead("counters.bc0_latency_err_l");
      m["counters.bc0_latency_err_h"]=uhalRead("counters.bc0_latency_err_h");
      m["counters.bc0_hdr_l"        ]=uhalRead("counters.bc0_hdr_l");
      m["counters.bc0_hdr_h"        ]=uhalRead("counters.bc0_hdr_h");

      uhalWrite("hist_hoc.ptr",0);

      unsigned ab(uhalRead("ctrl_stat.ctrl0.hist_sel"));
      unsigned ea(ab/2);
      ab=ab%2;

      //uint64_t tempTotal(0);
      //uint64_t tempStc;

      if(ea==0) {
	for(unsigned i(0);i<=24;i++) {
	  std::ostringstream sout;
	  sout << "hist_hoc_stc" << std::setfill('0') << std::setw(2) << (i+24*ab)/4;
	  if(i<24) sout << "_Address" << std::setw(1) << i%4;
	  else sout << "_total";

	  m[sout.str()+"_l"]=uhalRead("hist_hoc.d");
	  m[sout.str()+"_h"]=uhalRead("hist_hoc.d");
	}

      } else {
	for(unsigned i(0);i<=128;i++) {
	  std::ostringstream sout;
	  sout << "hist_hoc_stc" << std::setfill('0') << (ab==0?"A":"B");
	  if(i<128) sout << "_Energy" << std::setw(3) << i;
	  else sout << "_total";

	  m[sout.str()+"_l"]=uhalRead("hist_hoc.d");
	  m[sout.str()+"_h"]=uhalRead("hist_hoc.d");
	}
      }
    }  

    void sendPulse(const std::string &s) {
      if(uhalRead(s)!=0) uhalWrite(s,0);
      uhalWrite(s,1);
      uhalWrite(s,0);
    }  

    void reset() {
      sendPulse("ctrl_stat.ctrl0.rst");
      sendPulse("ctrl_stat.ctrl0.rst_counters");
    }  

    void print(std::ostream &o=std::cout) {
      o << "SerenityUnpacker::print()" << std::endl;
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
