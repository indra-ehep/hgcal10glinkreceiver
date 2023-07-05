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

#include "SerenityUhal.h"
#include "I2cInstruction.h"
#include "UhalInstruction.h"

#ifdef ProcessorHardware
#include "uhal/uhal.hpp"
#include "uhal/ValMem.hpp"
#endif

namespace Hgcal10gLinkReceiver {

  class SerenityMiniDaq : public SerenityUhal {
    
  public:
  
    SerenityMiniDaq() {
    }
    
    virtual ~SerenityMiniDaq() {
    }
    
    bool makeTable() {
      return SerenityUhal::makeTable("payload.miniDAQ");
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
    
    bool setDefaults() {
      uhalWrite("rstn",0);
      /*
      for(unsigned i(0);i<14;i++) {
	std::ostringstream sout;
	//sout << "Elink_mapping.Elink" << std::setfill('0')
	//   << std::setw(2) << i << ".";
	sout << "Elink_mapping.Elink" << i << ".";

	uhalWrite(sout.str()+"ID",i<6?0x0:0xf);
	uhalWrite(sout.str()+"start_addr",i<1?0x0000:0x3fff);
	uhalWrite(sout.str()+"end_addr",i<1?0x37ff:0x3fff);
      }
      */
      uhalWrite("Elink_mapping.Elink_config2",1);

      uhalWrite("ECOND_pkt_conf.Header",0x154);
      //uhalWrite("ECOND_pkt_conf.Header",0xac);
      //uhalWrite("ECOND_pkt_conf.Idle",0x555555);
      uhalWrite("ECOND_pkt_conf.Idle",0xaaaa);

      uhalWrite("rstn",1);

      return true;
    }  

    void reset() {
      uhalWrite("rstn",0);
      uhalWrite("rstn",1);
    }

    void print(std::ostream &o=std::cout) {
      o << "SerenityMiniDaq::print()" << std::endl;
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
