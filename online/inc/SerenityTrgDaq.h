#ifndef Hgcal10gLinkReceiver_SerenityTrgDaq_h
#define Hgcal10gLinkReceiver_SerenityTrgDaq_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "SerenityRegs.h"
#include "I2cInstruction.h"
#include "UhalInstruction.h"

//#ifdef ProcessorHardware
//#include "uhal/uhal.hpp"
//#include "uhal/ValMem.hpp"
//#endif

namespace Hgcal10gLinkReceiver {

  class SerenityTrgDaq : public SerenityRegs {
    
  public:
  
    SerenityTrgDaq() {
    }
    
    virtual ~SerenityTrgDaq() {
    }
    
    bool makeTable() {
      //SerenityUhal::makeTable("payload.trigger_ro");
      _uhalTopString="payload.trigger_ro";
      return true;
    }

    void configuration(YAML::Node &m) {
      m=YAML::Node();

      //m["SLink.source_id"]=uhalRead("SLink.source_id");

      for(unsigned i(0);i<8;i++) {
	std::ostringstream sout;
	sout << "daq_ro.DAQro" << i << ".";

	m[sout.str()+"latency"   ]=uhalRead(sout.str()+"latency");
	m[sout.str()+"event_size"]=uhalRead(sout.str()+"event_size");
	m[sout.str()+"ext_ro"    ]=uhalRead(sout.str()+"ext_ro");
      }
    }
  
    bool setDefaults() {
      unsigned flag(2);

      if(flag==0) {
	unsigned bxLatency(0);

	// ECON-T output
	uhalWrite("daq_ro.DAQro0.latency",1+4*bxLatency);
	uhalWrite("daq_ro.DAQro0.event_size",4);
	uhalWrite("daq_ro.DAQro0.ext_ro",2);
	
	// Unpacker outputs
	uhalWrite("daq_ro.DAQro1.latency",0+6*bxLatency);
	uhalWrite("daq_ro.DAQro1.event_size",6);
	uhalWrite("daq_ro.DAQro1.ext_ro",2);
	
	// Scintillators
	uhalWrite("daq_ro.DAQro2.latency",3+5*bxLatency);
	uhalWrite("daq_ro.DAQro2.event_size",5);
	uhalWrite("daq_ro.DAQro2.ext_ro",2);
	
	// Output of Stage 1 block 0
	uhalWrite("daq_ro.DAQro3.latency",3);
	uhalWrite("daq_ro.DAQro3.event_size",8);
	uhalWrite("daq_ro.DAQro3.ext_ro",1);
	
	// Output of Stage 1 block 1
	uhalWrite("daq_ro.DAQro4.latency",3);
	uhalWrite("daq_ro.DAQro4.event_size",8);
	uhalWrite("daq_ro.DAQro4.ext_ro",1);
	
      } else if(flag==1) {

	// Large packet
	
	// ECON-T output
	uhalWrite("daq_ro.DAQro0.latency",1+4*3);
	uhalWrite("daq_ro.DAQro0.event_size",4);
	uhalWrite("daq_ro.DAQro0.ext_ro",4);
	
	// Unpacker outputs
	uhalWrite("daq_ro.DAQro1.latency",0+6*3);
	uhalWrite("daq_ro.DAQro1.event_size",6);
	uhalWrite("daq_ro.DAQro1.ext_ro",4);
	
	// Scintillators
	unsigned latSci(28); // Aug 2023
	//unsigned latSci(24); // Aidan test
	uhalWrite("daq_ro.DAQro2.latency",3+5*latSci);
	uhalWrite("daq_ro.DAQro2.event_size",5);
	uhalWrite("daq_ro.DAQro2.ext_ro",4);
	//uhalWrite("daq_ro.DAQro2.ext_ro",7); // Aidan test
	
	// Output of Stage 1 block 0
	uhalWrite("daq_ro.DAQro3.latency",3);
	uhalWrite("daq_ro.DAQro3.event_size",8);
	uhalWrite("daq_ro.DAQro3.ext_ro",1);
	
	// Output of Stage 1 block 1
	uhalWrite("daq_ro.DAQro4.latency",3);
	uhalWrite("daq_ro.DAQro4.event_size",8);
	uhalWrite("daq_ro.DAQro4.ext_ro",1);

      } else {

	// Testing
	unsigned bxLatency(18);
	unsigned scintLatency(39);

	// ECON-T output
	uhalWrite("daq_ro.DAQro0.latency",0+4*(bxLatency  ));
	uhalWrite("daq_ro.DAQro0.event_size",4);
	uhalWrite("daq_ro.DAQro0.ext_ro",7);
	
	// Unpacker outputs
	uhalWrite("daq_ro.DAQro1.latency",0+6*(bxLatency-2));
	uhalWrite("daq_ro.DAQro1.event_size",6);
	uhalWrite("daq_ro.DAQro1.ext_ro",7);
	
	// Scintillators
	uhalWrite("daq_ro.DAQro2.latency",3+5*scintLatency);
	uhalWrite("daq_ro.DAQro2.event_size",5);
	uhalWrite("daq_ro.DAQro2.ext_ro",4);
	
	// Output of Stage 1 block 0
	uhalWrite("daq_ro.DAQro3.latency",3+8*(bxLatency-3));
	uhalWrite("daq_ro.DAQro3.event_size",8);
	uhalWrite("daq_ro.DAQro3.ext_ro",0);
	
	// Output of Stage 1 block 1
	uhalWrite("daq_ro.DAQro4.latency",3+8*(bxLatency-3));
	uhalWrite("daq_ro.DAQro4.event_size",8);
	uhalWrite("daq_ro.DAQro4.ext_ro",0);
      }
      
      // Unused

      uhalWrite("daq_ro.DAQro5.latency",3);
      uhalWrite("daq_ro.DAQro5.event_size",8);
      uhalWrite("daq_ro.DAQro5.ext_ro",1);

      uhalWrite("daq_ro.DAQro6.latency",0);
      uhalWrite("daq_ro.DAQro6.event_size",0);
      uhalWrite("daq_ro.DAQro6.ext_ro",0);

      uhalWrite("daq_ro.DAQro7.latency",0);
      uhalWrite("daq_ro.DAQro7.event_size",0);
      uhalWrite("daq_ro.DAQro7.ext_ro",0);
      
      return true;
    }  

    void reset() {
      sendReg320Pulse("ctrl0.trig_readout_rst");
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

    void monitoring(YAML::Node &m) {
      m=YAML::Node();

      m["reg_320.stat3.ch0_trg_backpressure"]=uhalReg320Read("stat3.ch0_trg_backpressure");
    }

    void print(std::ostream &o=std::cout) {
      o << "SerenityTrgDaq::print()" << std::endl;
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
