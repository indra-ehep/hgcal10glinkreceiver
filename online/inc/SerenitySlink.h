#ifndef Hgcal10gLinkReceiver_SerenitySlink_h
#define Hgcal10gLinkReceiver_SerenitySlink_h

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

  class SerenitySlink : public SerenityUhal {
    
  public:
  
    SerenitySlink() {
      //uhalWrite("csr.ctrl.global_quad_reset",1);
      //uhalWrite("
    }
    
    virtual ~SerenitySlink() {
    }
    
    bool makeTable() {
      return SerenityUhal::makeTable("slink");
    }

    void configuration(YAML::Node &m) {
      m=YAML::Node();

      for(unsigned q(0);q<1;q++) {
	YAML::Node mq;
	uhalWrite("csr.ctrl.quad_select",q);

	mq["quad.csr.ctrl.bypass_ram_nentries"]=uhalRead("quad.csr.ctrl.bypass_ram_nentries");
	mq["quad.csr.ctrl.use_bypass_ram"]=uhalRead("quad.csr.ctrl.use_bypass_ram");
	mq["quad.csr.ctrl.sw_bypass_loaded"]=uhalRead("quad.csr.ctrl.sw_bypass_loaded");
	mq["quad.csr.ctrl.capture_channel"]=uhalRead("quad.csr.ctrl.capture_channel");
	mq["quad.csr.ctrl.ram_capture"]=uhalRead("quad.csr.ctrl.ram_capture");
	mq["quad.csr.ctrl.use_generator"]=uhalRead("quad.csr.ctrl.use_generator");

	//mq["quad.csr.ctrl.global_core_reset"]=uhalRead("quad.csr.ctrl.global_core_reset");
	//mq["quad.csr.ctrl.generator_reset"]=uhalRead("quad.csr.ctrl.generator_reset");
	//mq["quad.csr.ctrl.bypass_reset"]=uhalRead("quad.csr.ctrl.bypass_reset");
	//mq["quad.csr.ctrl.global_channel_reset"]=uhalRead("quad.csr.ctrl.global_channel_reset");
	
	for(unsigned c(0);c<2;c++) {
	  YAML::Node mc;
	  uhalWrite("quad.csr.ctrl.channel_select",c);
	  
	  mc["quad.channel.csr_channel.ctrl.source_id"]=uhalRead("quad.channel.csr_channel.ctrl.source_id");

	  mc["quad.channel.csr_core.ctrl.core_status_addr"]=uhalRead("quad.channel.csr_core.ctrl.core_status_addr");

	  mc["quad.generator.csr.ctrl.reset"             ]=uhalRead("quad.generator.csr.ctrl.reset");
	  mc["quad.generator.csr.ctrl.enable"            ]=uhalRead("quad.generator.csr.ctrl.enable");
	  mc["quad.generator.csr.ctrl.decrement"         ]=uhalRead("quad.generator.csr.ctrl.decrement");
	  mc["quad.generator.csr.ctrl.length"            ]=uhalRead("quad.generator.csr.ctrl.length");
	  mc["quad.generator.csr.ctrl.pause"             ]=uhalRead("quad.generator.csr.ctrl.pause");
	  mc["quad.generator.csr.ctrl.physics_type"      ]=uhalRead("quad.generator.csr.ctrl.physics_type");
	  mc["quad.generator.csr.ctrl.l1a_type"          ]=uhalRead("quad.generator.csr.ctrl.l1a_type");
	  mc["quad.generator.csr.ctrl.ignore_lff"        ]=uhalRead("quad.generator.csr.ctrl.ignore_lff");
	  mc["quad.generator.csr.ctrl.inject_err_trailer"]=uhalRead("quad.generator.csr.ctrl.inject_err_trailer");
	  mc["quad.generator.csr.ctrl.inject_err_header" ]=uhalRead("quad.generator.csr.ctrl.inject_err_header");

	  // NOT CLEAR IF ONE RAM PER CHANNEL OR TWO PER QUAD; THIS ASSUMES ONE PER CHANNEL
	  unsigned n(uhalRead("quad.csr.ctrl.bypass_ram_nentries"));
	  std::ostringstream sbp;
	  sbp << "quad.bypass_ram_" << c;

	  for(unsigned a(0);a<n;a++) {
	    std::ostringstream sbpd;
	    //sbpd << sbp.str() << ".data_" << std::setfill('0') << std::setw(4) << a;
	    sbpd << "quad.bypass_ram.data_" << std::setfill('0') << std::setw(4) << a;

	    uhalWrite(sbp.str()+".addr",a);
	    mc[sbpd.str()]=uhalRead(sbp.str()+".data");
	  }

	  std::ostringstream sout;
	  sout << "Channel_" << c;
	  mq[sout.str()]=mc;
	}

	std::ostringstream sout;
	sout << "Quad_" << std::setfill('0') << std::setw(2) << q;
	m[sout.str()]=mq;
      }

      // UNCLEAR WHAT CHAN_SEL RANGE IS HERE
      /*
	ALL string freq 0x1001000
	ALL string freq.ctrl 0x1001000
	ALL string freq.ctrl.chan_sel 0x1001000
	ALL string freq.ctrl.en_crap_mode 0x1001000
	ALL string freq.freq 0x1001001
	ALL string freq.freq.count 0x1001001
	ALL string freq.freq.valid 0x1001001
      */
    }
    
    bool setDefaults() {

      return true;
    }  

    void status(YAML::Node &m) {
      m=YAML::Node();

      m["csr.stat.enabled_quads"  ]=uhalRead("csr.stat.enabled_quads");
      m["csr.stat.qpll_lock"      ]=uhalRead("csr.stat.qpll_lock");
      m["csr.stat.mmcm_lock"      ]=uhalRead("csr.stat.mmcm_lock");
      m["csr.stat.refclk_index"   ]=uhalRead("csr.stat.refclk_index");
      m["csr.stat.refclk_is_async"]=uhalRead("csr.stat.refclk_is_async");

      for(unsigned q(0);q<1;q++) {
	YAML::Node mq;
	uhalWrite("csr.ctrl.quad_select",q);

	mq["quad.csr.stat.link_down_any"     ]=uhalRead("quad.csr.stat.link_down_any");
	mq["quad.csr.stat.capture_done"      ]=uhalRead("quad.csr.stat.capture_done");
	mq["quad.csr.stat.bypass_loaded"     ]=uhalRead("quad.csr.stat.bypass_loaded");
	mq["quad.csr.stat.refclk_freq"       ]=uhalRead("quad.csr.stat.refclk_freq");
	mq["quad.csr.stat.xcvr_type"         ]=uhalRead("quad.csr.stat.xcvr_type");
	mq["quad.csr.stat.throughput"        ]=uhalRead("quad.csr.stat.throughput");
	mq["quad.csr.stat.region_equivalent" ]=uhalRead("quad.csr.stat.region_equivalent");
	mq["quad.csr.stat.channel_mask"      ]=uhalRead("quad.csr.stat.channel_mask");
	mq["quad.csr.stat.core0_word_per_sec"]=uhalRead("quad.csr.stat.core0_word_per_sec");
	mq["quad.csr.stat.core1_word_per_sec"]=uhalRead("quad.csr.stat.core1_word_per_sec");
	mq["quad.csr.stat.core2_word_per_sec"]=uhalRead("quad.csr.stat.core2_word_per_sec");
	mq["quad.csr.stat.core3_word_per_sec"]=uhalRead("quad.csr.stat.core3_word_per_sec");
	mq["quad.csr.stat.bypass_address"    ]=uhalRead("quad.csr.stat.bypass_address");

	mq["quad.generator.csr.stat.is_reset"        ]=uhalRead("quad.generator.csr.stat.is_reset");
	mq["quad.generator.csr.stat.is_backpressured"]=uhalRead("quad.generator.csr.stat.is_backpressured");
	mq["quad.generator.csr.stat.is_paused"       ]=uhalRead("quad.generator.csr.stat.is_paused");
	mq["quad.generator.csr.stat.bx_id"           ]=uhalRead("quad.generator.csr.stat.bx_id");
	mq["quad.generator.csr.stat.orbit_id"        ]=uhalRead("quad.generator.csr.stat.orbit_id");

	for(unsigned c(0);c<2;c++) {
	  YAML::Node mc;
	  uhalWrite("quad.csr.ctrl.channel_select",c);
	  
	  mc["quad.channel.csr_channel.stat.error_excessive_packet"  ]=uhalRead("quad.channel.csr_channel.stat.error_excessive_packet");
	  mc["quad.channel.csr_channel.stat.packet_fifo_full"        ]=uhalRead("quad.channel.csr_channel.stat.packet_fifo_full");
	  mc["quad.channel.csr_channel.stat.error_repeat_header"     ]=uhalRead("quad.channel.csr_channel.stat.error_repeat_header");
	  mc["quad.channel.csr_channel.stat.error_repeat_trailer"    ]=uhalRead("quad.channel.csr_channel.stat.error_repeat_trailer");
	  mc["quad.channel.csr_channel.stat.trigger_fifo_overflow"   ]=uhalRead("quad.channel.csr_channel.stat.trigger_fifo_overflow");
	  mc["quad.channel.csr_channel.stat.data_fifo_overflow"      ]=uhalRead("quad.channel.csr_channel.stat.data_fifo_overflow");
	  mc["quad.channel.csr_channel.stat.packet_fifo_overflow"    ]=uhalRead("quad.channel.csr_channel.stat.packet_fifo_overflow");
	  mc["quad.channel.csr_channel.stat.trigger_fifo_really_full"]=uhalRead("quad.channel.csr_channel.stat.trigger_fifo_really_full");
	  mc["quad.channel.csr_channel.stat.data_fifo_really_full"   ]=uhalRead("quad.channel.csr_channel.stat.data_fifo_really_full");
	  mc["quad.channel.csr_channel.stat.packet_fifo_really_full" ]=uhalRead("quad.channel.csr_channel.stat.packet_fifo_really_full");
	  mc["quad.channel.csr_channel.stat.trigger_fifo_empty"      ]=uhalRead("quad.channel.csr_channel.stat.trigger_fifo_empty");
	  mc["quad.channel.csr_channel.stat.data_fifo_empty"         ]=uhalRead("quad.channel.csr_channel.stat.data_fifo_empty");
	  mc["quad.channel.csr_channel.stat.packet_fifo_empty"       ]=uhalRead("quad.channel.csr_channel.stat.packet_fifo_empty");
	  mc["quad.channel.csr_channel.stat.trigger_fifo_full"       ]=uhalRead("quad.channel.csr_channel.stat.trigger_fifo_full");
	  mc["quad.channel.csr_channel.stat.data_fifo_full"          ]=uhalRead("quad.channel.csr_channel.stat.data_fifo_full");
	  mc["quad.channel.csr_channel.stat.packet_fifo_occupancy"   ]=uhalRead("quad.channel.csr_channel.stat.packet_fifo_occupancy");
	  mc["quad.channel.csr_channel.stat.backpressure_per_sec"    ]=uhalRead("quad.channel.csr_channel.stat.backpressure_per_sec");

	  mc["quad.channel.csr_core.stat.lff_per_sec"   ]=uhalRead("quad.channel.csr_core.stat.lff_per_sec");
	  mc["quad.channel.csr_core.stat.lff"           ]=uhalRead("quad.channel.csr_core.stat.lff");
	  mc["quad.channel.csr_core.stat.link_down"     ]=uhalRead("quad.channel.csr_core.stat.link_down");
	  mc["quad.channel.csr_core.stat.word_per_sec"  ]=uhalRead("quad.channel.csr_core.stat.word_per_sec");
	  mc["quad.channel.csr_core.stat.core_status_l" ]=uhalRead("quad.channel.csr_core.stat.core_status_l");
	  mc["quad.channel.csr_core.stat.core_status_h" ]=uhalRead("quad.channel.csr_core.stat.core_status_h");
	  mc["quad.channel.csr_core.stat.packet_count_l"]=uhalRead("quad.channel.csr_core.stat.packet_count_l");
	  mc["quad.channel.csr_core.stat.packet_count_h"]=uhalRead("quad.channel.csr_core.stat.packet_count_h");

	  std::ostringstream sout;
	  sout << "Channel_" << c;
	  mq[sout.str()]=mc;
	}

	std::ostringstream sout;
	sout << "Quad_" << std::setfill('0') << std::setw(2) << q;
	m[sout.str()]=mq;
      }

    }

    void sendPulse(const std::string &s) {
      if(uhalRead(s)!=0) uhalWrite(s,0);
      uhalWrite(s,1);
      uhalWrite(s,0);
    }  

    void globalQuadReset() {
      sendPulse("csr.ctrl.global_quad_reset");
    }  

    void channelReset() {
      uhalWrite("csr.ctrl.quad_select",0);
      //sendPulse("quad.csr.ctrl.global_core_reset");
      //sendPulse("quad.csr.ctrl.global_channel_reset");

      for(unsigned i(0);i<2;i++) {
	uhalWrite("quad.csr.ctrl.channel_select",i);
	sendPulse("quad.channel.csr_channel.ctrl.reset");
      }
    }  

    void setSourceId(unsigned c, uint32_t s) {
      if(c>=2) return;
      uhalWrite("csr.ctrl.quad_select",0);
      uhalWrite("quad.csr.ctrl.channel_select",c);
      uhalWrite("quad.channel.csr_channel.ctrl.source_id",s);
    }

    void print(std::ostream &o=std::cout) {
      o << "SerenitySlink::print()" << std::endl;
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
