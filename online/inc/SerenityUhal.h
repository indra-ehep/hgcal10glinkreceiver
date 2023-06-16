#ifndef Hgcal10gLinkReceiver_SerenityUhal_h
#define Hgcal10gLinkReceiver_SerenityUhal_h

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
    
    virtual void setPrintEnable(bool p) {
      _printEnable=p;
    }

    uint32_t payloadVersion() {
      const uhal::Node& lNode = lHW.getNode("info.versions.payload");
      uhal::ValWord<uint32_t> lReg = lNode.read();
      lHW.dispatch();
      return lReg.value();
    }

    bool makeTable(const std::string &s="payload") {
      _uhalTopString=s;
      
#ifdef ProcessorHardware
      //uhal::ValWord<uint32_t> ver = lHW.getNode("FIRMWARE_REVISION").read();
      //std::cout << "Firmware revision = " << ver.value() << std::endl;

      std::vector<std::string> temp;
      temp=lHW.getNode(s).getNodes();
      //temp=lHW.getNodes();
      
      if(_printEnable) {
	for(unsigned i(0);i<temp.size();i++) {
	  std::cout << "ALL string " << temp[i]
		    << " 0x" << std::hex
		    << lHW.getNode(s+"."+temp[i]).getAddress()
		    << std::endl;
	}
	std::cout << std::dec << std::endl;
      }

      _uhalString.resize(0);
      for(unsigned i(0);i<temp.size();i++) {
	/*
	if(s=="payload") {
	  if(temp[i].substr(0,8)=="fc_ctrl.") {
	    std::cout << "Substr = " << temp[i].substr(8,17) << std::endl;
	    //if(temp[i].substr(8,17)!="tcds2_emu") {
	    std::cout << "UHAL string " << std::setw(3) << " = " 
		      << temp[i] << std::endl;
	    _uhalString.push_back(s+"."+temp[i]);
	    //}
	  }
	} else {
	  _uhalString.push_back(s+"."+temp[i]);
        }
	*/
	  _uhalString.push_back(temp[i]);
      }
    
      if(_printEnable) {
	for(unsigned i(0);i<_uhalString.size();i++) {
	  std::cout << "RO UHAL string " << std::setw(3) << " = " 
		    << _uhalString[i] << std::endl;
	
	  const uhal::Node& lNode = lHW.getNode(_uhalTopString+"."+_uhalString[i]);
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
  
#ifdef ProcessorHardware
  uint32_t uhalRead(const std::string &s) {
    //const uhal::Node& lNode = lHW.getNode(std::string("payload.")+s);
    const uhal::Node& lNode = lHW.getNode(_uhalTopString+"."+s);
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
    const uhal::Node& lNode = lHW.getNode(_uhalTopString+"."+s);
    lNode.write(v);
    lHW.dispatch();
    
    //return uhalRead(s)==v;
    return true;
  }

#else

  virtual uint32_t uhalRead(const std::string &s) {
    return 999;
  }

  virtual bool uhalWrite(const std::string &s, uint32_t v) {
    std::cout << "uhalWrite: setting " << s << " to  0x"
	      << std::hex << std::setfill('0')
	      << std::setw(8) << v
	      << std::dec << std::setfill(' ')
	      << std::endl;
    return true;
  }

#endif

  void csv(std::ostream &o=std::cout) {
    o << _uhalTopString << std::endl;
    
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
    o << "SerenityUhal::print() Top string " << _uhalTopString << std::endl;
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
  bool _printEnable;
  
  std::string _uhalTopString;
  std::vector<std::string> _uhalString;

#ifdef ProcessorHardware
  const std::string lConnectionFilePath;
  const std::string lDeviceId;
  uhal::ConnectionManager lConnectionMgr;
  uhal::HwInterface lHW;
#else
  std::vector<uint32_t> _uhalData;
#endif

};

}

#endif
