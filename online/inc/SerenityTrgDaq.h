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

#include "SerenityUhal.h"
#include "I2cInstruction.h"
#include "UhalInstruction.h"

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityTrgDaq : public SerenityUhal {
    
  public:
  
    SerenityTrgDaq() {
    }
    
    virtual ~SerenityTrgDaq() {
    }
    
    bool makeTable() {
      SerenityUhal::makeTable("payload.trigger_ro");
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
      
      // Set in ProcessorFastControl
      //unsigned daqBoard(3);
      //unsigned slink(0)
      //uhalWrite("SLink.source_id",0xce000000|daqBoard<<4|slink);

      if(false) {
      unsigned bxLatency(0);

      // ECON-T output
      uhalWrite("daq_ro.DAQro0.latency",5+4*bxLatency);
      uhalWrite("daq_ro.DAQro0.event_size",4);
      uhalWrite("daq_ro.DAQro0.ext_ro",2);

      // Unpacker outputs
      uhalWrite("daq_ro.DAQro1.latency",4+6*bxLatency);
      uhalWrite("daq_ro.DAQro1.event_size",6);
      uhalWrite("daq_ro.DAQro1.ext_ro",2);

      // Scintillators
      uhalWrite("daq_ro.DAQro2.latency",5+5*bxLatency);
      uhalWrite("daq_ro.DAQro2.event_size",5);
      uhalWrite("daq_ro.DAQro2.ext_ro",2);

      // Output of Stage 1 block
      uhalWrite("daq_ro.DAQro3.latency",20);
      uhalWrite("daq_ro.DAQro3.event_size",8);
      uhalWrite("daq_ro.DAQro3.ext_ro",2);

      } else {
	// Large packet

      // ECON-T output
      uhalWrite("daq_ro.DAQro0.latency",5);
      uhalWrite("daq_ro.DAQro0.event_size",4);
      uhalWrite("daq_ro.DAQro0.ext_ro",4);

      // Unpacker outputs
      uhalWrite("daq_ro.DAQro1.latency",4);
      uhalWrite("daq_ro.DAQro1.event_size",6);
      uhalWrite("daq_ro.DAQro1.ext_ro",4);

      // Scintillators
      uhalWrite("daq_ro.DAQro2.latency",5);
      uhalWrite("daq_ro.DAQro2.event_size",5);
      uhalWrite("daq_ro.DAQro2.ext_ro",4);

      // Output of Stage 1 block
      uhalWrite("daq_ro.DAQro3.latency",20);
      uhalWrite("daq_ro.DAQro3.event_size",8);
      uhalWrite("daq_ro.DAQro3.ext_ro",4);
      }
      // Unused

      uhalWrite("daq_ro.DAQro4.latency",0);
      uhalWrite("daq_ro.DAQro4.event_size",0);
      uhalWrite("daq_ro.DAQro4.ext_ro",0);

      uhalWrite("daq_ro.DAQro5.latency",0);
      uhalWrite("daq_ro.DAQro5.event_size",0);
      uhalWrite("daq_ro.DAQro5.ext_ro",0);

      uhalWrite("daq_ro.DAQro6.latency",0);
      uhalWrite("daq_ro.DAQro6.event_size",0);
      uhalWrite("daq_ro.DAQro6.ext_ro",0);

      uhalWrite("daq_ro.DAQro7.latency",0);
      uhalWrite("daq_ro.DAQro7.event_size",0);
      uhalWrite("daq_ro.DAQro7.ext_ro",0);

      return true;
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
