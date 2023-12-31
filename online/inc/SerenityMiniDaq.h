#ifndef Hgcal10gLinkReceiver_SerenityMiniDaq_h
#define Hgcal10gLinkReceiver_SerenityMiniDaq_h

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

  class SerenityMiniDaq : public SerenityRegs {
    
  public:
  
    SerenityMiniDaq() {
    }
    
    virtual ~SerenityMiniDaq() {
    }
    
    bool makeTable(const std::string &s="") {
      _miniDaqId=s;
      _uhalTopString="payload.miniDAQ"+_miniDaqId;
      return true;
      //return SerenityUhal::makeTable("payload.miniDAQ"+_miniDaqId);
    }

    void configuration(YAML::Node &m) {
      m=YAML::Node();

      for(unsigned i(0);i<14;i++) {
	std::ostringstream sout;
	//sout << "Elink_mapping.Elink" << std::setfill('0')
	//   << std::setw(2) << i << ".";
	sout << "Elink_mapping.Elink" << i << ".";

	m[sout.str()+"ID"        ]=uhalRead(sout.str()+"ID");
	m[sout.str()+"start_addr"]=uhalRead(sout.str()+"start_addr");
	m[sout.str()+"end_addr"  ]=uhalRead(sout.str()+"end_addr");
      }
      
      m["Elink_mapping.Elink_config2"]=uhalRead("Elink_mapping.Elink_config2");

      m["ECOND_pkt_conf.Header"      ]=uhalRead("ECOND_pkt_conf.Header");
      m["ECOND_pkt_conf.Idle"        ]=uhalRead("ECOND_pkt_conf.Idle");
      m["ECOND_pkt_conf.BX_mismatch_passthrough"]=uhalRead("ECOND_pkt_conf.BX_mismatch_passthrough");

      m["DAQ_FC_config.BC_rst_value"]=uhalRead("DAQ_FC_config.BC_rst_value");
      m["DAQ_FC_config.OC_rst_value"]=uhalRead("DAQ_FC_config.OC_rst_value");
    }

    void configuration(std::unordered_map<std::string,uint32_t> &m) {
      m.clear();

      for(unsigned i(0);i<14;i++) {
	std::ostringstream sout;
	//sout << "Elink_mapping.Elink" << std::setfill('0')
	//   << std::setw(2) << i << ".";
	sout << "Elink_mapping.Elink" << i << ".";

	m[sout.str()+"ID"        ]=uhalRead(sout.str()+"ID");
	m[sout.str()+"start_addr"]=uhalRead(sout.str()+"start_addr");
	m[sout.str()+"end_addr"  ]=uhalRead(sout.str()+"end_addr");
      }
      
      m["Elink_mapping.Elink_config2"]=uhalRead("Elink_mapping.Elink_config2");

      m["ECOND_pkt_conf.Header"      ]=uhalRead("ECOND_pkt_conf.Header");
      m["ECOND_pkt_conf.Idle"        ]=uhalRead("ECOND_pkt_conf.Idle");
    }
    
    bool setNumberOfEconds(unsigned nEcond) {
      uhalWrite("rstn",0);

      unsigned memPerEcond(1024*(15/nEcond));

      for(unsigned i(0);i<14;i++) {
	std::ostringstream sout;
	//sout << "Elink_mapping.Elink" << std::setfill('0')
	//   << std::setw(2) << i << ".";
	sout << "Elink_mapping.Elink" << i << ".";

#ifdef DthHardware

	// DTH needs both ECON-Ds in same MiniDAQ=""
	if(_miniDaqId=="") {
 	  if     (i==0 || i==1) uhalWrite(sout.str()+"ID",0);
	  else if(i==2 || i==3) uhalWrite(sout.str()+"ID",1); // Cludge: duplicate
	  //else if(i==4 || i==5) uhalWrite(sout.str()+"ID",1); // Cludge: duplicate
	  else                  uhalWrite(sout.str()+"ID",0xf);
	} else {
	  assert(false);
	}
#else

	// 10G needs one ECON-D in each
	if(_miniDaqId=="") {
	  if(i<2*nEcond) {
	    uhalWrite(sout.str()+"ID",i/2);
	  } else {
	    uhalWrite(sout.str()+"ID",0xf);
	  }

	} else {
	  //if(i==4 || i==5) {
	  //uhalWrite(sout.str()+"ID",(i-4)/2);
	  if(i==2 || i==3) {
	    uhalWrite(sout.str()+"ID",(i-2)/2);
	  } else {
	    uhalWrite(sout.str()+"ID",0xf);
	  }	  
	}
#endif

	if(i<nEcond) {
	  uhalWrite(sout.str()+"start_addr",i*memPerEcond);
	  uhalWrite(sout.str()+"end_addr",(1+i)*memPerEcond-1);

	} else {
	  uhalWrite(sout.str()+"start_addr",0x3fff);
	  uhalWrite(sout.str()+"end_addr",0x3fff);
	}
      }

      uhalWrite("Elink_mapping.Elink_config2",nEcond);

      uhalWrite("rstn",1);

      return true;
    }  

    bool setDefaults() {
#ifdef DthHardware
      setNumberOfEconds(2);
#else
      setNumberOfEconds(1);
#endif

      uhalWrite("rstn",0);

      //uhalWrite("ECOND_pkt_conf.Header",0xac);
      //uhalWrite("ECOND_pkt_conf.Idle",0x555555);
      uhalWrite("ECOND_pkt_conf.Header",0x154);
      //uhalWrite("ECOND_pkt_conf.Idle",0xaaaaff);
      uhalWrite("ECOND_pkt_conf.Idle",0xccccff);

      //uhalWrite("DAQ_FC_config.BC_rst_value",4);
      uhalWrite("DAQ_FC_config.BC_rst_value",5);
      uhalWrite("DAQ_FC_config.OC_rst_value",0);

      //uhalWrite("ECOND_pkt_conf.BX_mismatch_passthrough",1);
      uhalWrite("ECOND_pkt_conf.BX_mismatch_passthrough",0);

      uhalWrite("rstn",1);

      uhalReg040Write("ctrl0.throttle_en_miniDAQ",1);

      return true;
    }  

    void status(YAML::Node &m) {
      m=YAML::Node();

      if(false) {
	m["UNCOMMENT_REAL_VALUES"]=0;

      } else {
	m["Capture_block_status"]=uhalRead("Capture_block_status");
	
	m["hdr_crc_econd00"]=uhalRead("hdr_crc_econd00");
	m["hdr_crc_econd01"]=uhalRead("hdr_crc_econd01");
	m["hdr_crc_econd02"]=uhalRead("hdr_crc_econd02");
	m["hdr_crc_econd03"]=uhalRead("hdr_crc_econd03");
	m["hdr_crc_econd04"]=uhalRead("hdr_crc_econd04");
	m["hdr_crc_econd05"]=uhalRead("hdr_crc_econd05");
	m["hdr_crc_econd06"]=uhalRead("hdr_crc_econd06");
	m["hdr_crc_econd07"]=uhalRead("hdr_crc_econd07");
	m["hdr_crc_econd08"]=uhalRead("hdr_crc_econd08");
	m["hdr_crc_econd09"]=uhalRead("hdr_crc_econd09");
	m["hdr_crc_econd10"]=uhalRead("hdr_crc_econd10");
	m["hdr_crc_econd11"]=uhalRead("hdr_crc_econd11");
      }
    }

    void monitoring(YAML::Node &m) {
      m=YAML::Node();

#ifndef DthHardware
      if(_miniDaqId=="") {
	m["reg_320.stat3.ch_daq_backpressure"   ]=uhalReg320Read("stat3.ch1_daq_backpressure");
	m["reg_320.stat3.mDAQ_evbuf_empty"      ]=uhalReg320Read("stat3.mDAQ_evbuf_empty");
	m["reg_320.stat3.mDAQ_packet_in_buffer" ]=uhalReg320Read("stat3.mDAQ_packet_in_buffer");
	m["reg_320.stat3.mDAQ_eth_pktsize_error"]=uhalReg320Read("stat3.mDAQ_eth_pktsize_error");
	m["reg_320.stat3.mDAQ_pktsize_error"    ]=uhalReg320Read("stat3.mDAQ_pktsize_error");
	m["reg_320.stat3.mDAQ_big_pkt_size"     ]=uhalReg320Read("stat3.mDAQ_big_pkt_size");

      } else if(_miniDaqId=="1") {
	m["reg_320.stat3.ch_daq_backpressure"   ]=uhalReg320Read("stat3.ch2_daq_backpressure");
	m["reg_320.stat3.mDAQ_evbuf_empty"      ]=uhalReg320Read("stat3.mDAQ1_evbuf_empty");
	m["reg_320.stat3.mDAQ_packet_in_buffer" ]=uhalReg320Read("stat3.mDAQ1_packet_in_buffer");
	m["reg_320.stat3.mDAQ_eth_pktsize_error"]=uhalReg320Read("stat3.mDAQ1_eth_pktsize_error");
      }
#endif
    }

    void reset() {
      uhalWrite("rstn",0);
      uhalWrite("rstn",1);
      sendReg320Pulse("ctrl0.daq_readout_rst");
    }
    
    void print(std::ostream &o=std::cout) {
      o << "SerenityMiniDaq::print()" << "  ID = " << _miniDaqId << std::endl;
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
    std::string _miniDaqId;
  };

}

#endif
