#ifndef Hgcal10gLinkReceiver_Serenity10g_h
#define Hgcal10gLinkReceiver_Serenity10g_h

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

  class Serenity10g : public SerenityUhal {
    
  public:
  
    Serenity10g() {
    }
    
    virtual ~Serenity10g() {
    }
    
    bool makeTable() {
      SerenityUhal::makeTable("eth10g");
      return true;
    }
  
    void reset() {

      for(unsigned j(0);j<2;j++) {
	uhalWrite("quad_ctrl.select",0x0);
	uhalWrite("quad.channel_ctrl.select",j);
	uhalWrite("quad.channel.ctrl.reg.rst",1);
	//uhalWrite("quad.channel.stat.lff_watchdog",0);
      }

      usleep(1000);

      for(unsigned j(0);j<2;j++) {	
	uhalWrite("quad_ctrl.select",0x0);
	uhalWrite("quad.channel_ctrl.select",j);
	uhalWrite("quad.channel.ctrl.reg.rst",0);
	//uhalWrite("quad.channel.stat.lff_watchdog",0);
      }
    }
  
    bool setDefaults() {

      if(true) {

      //channel 0
      uint32_t local_ip0(0xC0A80352);
      uint32_t remote_ip0(0xC0A80302);
      uint32_t local_port0(0x04D2);
      uint32_t remote_port0(0x04D2);
      uint32_t run_id0(0xABD);
      //uint32_t aggregate_en0(0x1);
      uint32_t aggregate_en0(0x0); // No aggregation with ECON-T data (v2xx onwards)
      uint32_t aggregate_limit0(0xFF);
      uint32_t watchdog_threshold0(0x60);
      //uint32_t watchdog_threshold0(0xffffffff);

      //channel 1
      uint32_t local_ip1(0xC0A80452);
      uint32_t remote_ip1(0xC0A80402);
      uint32_t local_port1(0x04D3);
      uint32_t remote_port1(0x04D3);
      uint32_t run_id1(0xFEC);
      uint32_t aggregate_en1(0x0);
      uint32_t aggregate_limit1(0xFF);
      uint32_t watchdog_threshold1(0x60);
      //uint32_t watchdog_threshold1(0xffffffff);


      // disable the eth link
      uhalWrite("quad_ctrl.select",0x0);

      uhalWrite("quad.channel_ctrl.select",0x0);
      uhalWrite("quad.channel.ctrl.reg.rst",1);

      uhalWrite("quad.channel_ctrl.select",0x1);
      uhalWrite("quad.channel.ctrl.reg.rst",1);
      
      // disable the echo server
      uhalWrite("quad_ctrl.select",0x0);

      uhalWrite("quad.channel_ctrl.select",0x0);
      uhalWrite("quad.channel.ctrl.reg.echo",0);

      uhalWrite("quad.channel_ctrl.select",0x1);
      uhalWrite("quad.channel.ctrl.reg.echo",0);

      // configure the eth interface
      uhalWrite("quad_ctrl.select",0x0);

      uhalWrite("quad.channel_ctrl.select",0x0);
      uhalWrite("quad.channel.ctrl.local_ip",local_ip0);
      uhalWrite("quad.channel.ctrl.remote_ip",remote_ip0);
      uhalWrite("quad.channel.ctrl.ports.local",local_port0);
      uhalWrite("quad.channel.ctrl.ports.remote",remote_port0);
      uhalWrite("quad.channel.ctrl.reg.run",run_id0);

      //uhalWrite("quad_ctrl.select",0x0);

      uhalWrite("quad.channel_ctrl.select",0x1);
      uhalWrite("quad.channel.ctrl.local_ip",local_ip1);
      uhalWrite("quad.channel.ctrl.remote_ip",remote_ip1);
      uhalWrite("quad.channel.ctrl.ports.local",local_port1);
      uhalWrite("quad.channel.ctrl.ports.remote",remote_port1);
      uhalWrite("quad.channel.ctrl.reg.run",run_id1);
      
      // small pkt aggregation
      uhalWrite("quad_ctrl.select",0x0);

      uhalWrite("quad.channel_ctrl.select",0x0);
      uhalWrite("quad.channel.ctrl.reg.aggregate",aggregate_en0);
      uhalWrite("quad.channel.ctrl.reg.aggregate_limit",aggregate_limit0);

      uhalWrite("quad.channel_ctrl.select",0x1);
      uhalWrite("quad.channel.ctrl.reg.aggregate",aggregate_en1);
      uhalWrite("quad.channel.ctrl.reg.aggregate_limit",aggregate_limit1);
      
      // set the lff WD threshold
      uhalWrite("quad_ctrl.select",0x0);

      uhalWrite("quad.channel_ctrl.select",0x0);
      uhalWrite("quad.channel.ctrl.watchdog_threshold",watchdog_threshold0);
      uhalWrite("quad.channel.ctrl.reg.heartbeat",1);
      uhalWrite("quad.channel.ctrl.heartbeat_threshold",0x13000000);

      uhalWrite("quad.channel_ctrl.select",0x1);
      uhalWrite("quad.channel.ctrl.watchdog_threshold",watchdog_threshold1);
      uhalWrite("quad.channel.ctrl.reg.heartbeat",1);
      uhalWrite("quad.channel.ctrl.heartbeat_threshold",0x13000000);
      
      // enable the eth link
      uhalWrite("quad_ctrl.select",0x0);

      uhalWrite("quad.channel_ctrl.select",0x0);
      uhalWrite("quad.channel.ctrl.reg.rst",0);

      uhalWrite("quad.channel_ctrl.select",0x1);
      uhalWrite("quad.channel.ctrl.reg.rst",0);
      }

      // Cludge for now; move later
      uhalWrite("reg_320.ctrl1.slink_daq_readout_bp_en",0,true);
      uhalWrite("reg_320.ctrl1.slink_trig_readout_bp_en",0,true);

      // SHOULD BE ONE FOR NOW!!!!
      uhalWrite("reg_320.ctrl1.rate_throttle_daq_en",1,true);
      uhalWrite("reg_320.ctrl1.rate_throttle_trig_en",1,true);
      //uhalWrite("reg_320.ctrl1.rate_throttle_daq_en",0,true);
      //uhalWrite("reg_320.ctrl1.rate_throttle_trig_en",0,true);

      return true;
    }  

    void configuration(YAML::Node &m) {
      m=YAML::Node();

      for(unsigned j(0);j<2;j++) {
	std::ostringstream sout;
	sout << j;

	uhalWrite("quad_ctrl.select",0x0);
	uhalWrite("quad.channel_ctrl.select",j);

	m[std::string("quad.channel.ctrl.reg.echo_"     )+sout.str()]=uhalRead("quad.channel.ctrl.reg.echo");

	m[std::string("quad.channel.ctrl.local_ip_"    )+sout.str()]=uhalRead("quad.channel.ctrl.local_ip");
	m[std::string("quad.channel.ctrl.remote_ip_"   )+sout.str()]=uhalRead("quad.channel.ctrl.remote_ip");
	m[std::string("quad.channel.ctrl.ports.local_" )+sout.str()]=uhalRead("quad.channel.ctrl.ports.local");
	m[std::string("quad.channel.ctrl.ports.remote_")+sout.str()]=uhalRead("quad.channel.ctrl.ports.remote");
	m[std::string("quad.channel.ctrl.reg.run_"     )+sout.str()]=uhalRead("quad.channel.ctrl.reg.run");

	m[std::string("quad.channel.ctrl.reg.aggregate_"      )+sout.str()]=uhalRead("quad.channel.ctrl.reg.aggregate");
	m[std::string("quad.channel.ctrl.reg.aggregate_limit_")+sout.str()]=uhalRead("quad.channel.ctrl.reg.aggregate_limit");
      }

      m["reg_320.ctrl1.slink_daq_readout_bp_en"]=uhalRead("reg_320.ctrl1.slink_daq_readout_bp_en",true);
      m["reg_320.ctrl1.slink_trig_readout_bp_en"]=uhalRead("reg_320.ctrl1.slink_trig_readout_bp_en",true);
      m["reg_320.ctrl1.rate_throttle_daq_en"]=uhalRead("reg_320.ctrl1.rate_throttle_daq_en",true);
      m["reg_320.ctrl1.rate_throttle_trig_en"]=uhalRead("reg_320.ctrl1.rate_throttle_trig_en",true);
    }

    void status(YAML::Node &m) {
      m=YAML::Node();

      uhalWrite("quad_ctrl.select",0x0);

      for(unsigned j(0);j<2;j++) {
	uhalWrite("quad.channel_ctrl.select",j);	
	{
	std::ostringstream sout;
        sout << "quad.channel.stat.pktcnt_l_" << std::setfill('0') << std::setw(2) << j;
	m[sout.str()]=uhalRead("quad.channel.stat.pktcnt_l");
	}

	{
	std::ostringstream sout;
        sout << "quad.channel.stat.pktcnt_h_" << std::setfill('0') << std::setw(2) << j;
	m[sout.str()]=uhalRead("quad.channel.stat.pktcnt_h");
	}

	{
	std::ostringstream sout;
        sout << "quad.channel.stat.lff_l_" << std::setfill('0') << std::setw(2) << j;
	m[sout.str()]=uhalRead("quad.channel.stat.lff_l");
	}

	{
	std::ostringstream sout;
        sout << "quad.channel.stat.lff_h_" << std::setfill('0') << std::setw(2) << j;
	m[sout.str()]=uhalRead("quad.channel.stat.lff_h");
	}

	{
	std::ostringstream sout;
        sout << "quad.channel.stat.lff_watchdog_" << std::setfill('0') << std::setw(2) << j;
	m[sout.str()]=uhalRead("quad.channel.stat.lff_watchdog");
	}
	/*
	m["quad.channel.stat.pktcnt_h"    ][j]=uhalRead("quad.channel.stat.pktcnt_h");
	m["quad.channel.stat.lff_l"       ][j]=uhalRead("quad.channel.stat.lff_l");
	m["quad.channel.stat.lff_h"       ][j]=uhalRead("quad.channel.stat.lff_h");
	m["quad.channel.stat.lff_watchdog"][j]=uhalRead("quad.channel.stat.lff_watchdog");
	*/
      }
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
      o << "Serenity10g::print()" << std::endl;
      o << " Current settings for " << _uhalString.size()
	<< " values:" << std::endl;

      o << std::hex << std::setfill('0');

      uhalWrite("quad_ctrl.select",0x0);
      for(unsigned j(0);j<2;j++) {
	uhalWrite("quad.channel_ctrl.select",j);
	std::cout << " Quad channel " << j << std::endl;

	for(unsigned i(0);i<_uhalString.size();i++) {
	  uint32_t v(uhalRead(_uhalString[i]));
	  std::cout << "   " << _uhalString[i] << " = 0x"
		    << std::hex << std::setfill('0')
		    << std::setw(8) << v
		    << std::dec << std::setfill(' ')
		    << " = " << v
		    << std::endl;
	}
	o << std::dec << std::setfill(' ');
      }
    }

  
  protected:

  };

}

#endif
