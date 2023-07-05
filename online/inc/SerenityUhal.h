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


#include "emp/ChannelManager.hpp"
#include "emp/Controller.hpp"
#include "emp/CtrlNode.hpp"
#include "emp/TTCNode.hpp"
#include "emp/exception.hpp"
#include "emp/utilities/misc.hpp"
#include "emp/BoardData.hpp"

#include "emp/logger/logger.hpp"
#include "/home/cmx/rshukla/emp-toolbox/core/include/emp/Controller.hpp"
#include "emp/DatapathNode.hpp"
#include "emp/GbtMGTRegionNode.hpp"
#include "/home/cmx/rshukla/emp-toolbox/core/include/emp/LpGbtMGTRegionNode.hpp"
#include "emp/SCCNode.hpp"
#include "emp/SCCICNode.hpp"
#include "emp/utilities/misc.hpp"

using namespace uhal;
using namespace emp;

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
      //lDeviceId("x0"),
      lDeviceId("x0_current"),
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
#ifdef ProcessorHardware
      const uhal::Node& lNode = lHW.getNode("info.versions.payload");
      uhal::ValWord<uint32_t> lReg = lNode.read();
      lHW.dispatch();
      return lReg.value();
#else
      return 999999999;
#endif
  }

    bool makeTable(const std::string &s="payload") {
      _uhalTopString=s;
      
#ifdef ProcessorHardware
      //uhal::ValWord<uint32_t> ver = lHW.getNode("FIRMWARE_REVISION").read();
      //std::cout << "Firmware revision = " << ver.value() << std::endl;

      std::vector<std::string> temp;
      if(s=="TOP") temp=lHW.getNodes();
      else temp=lHW.getNode(s).getNodes();
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

  std::vector<unsigned int> i2c_read(/*HwInterface hi,*/ unsigned char ch, unsigned int gbtx_addr, unsigned int target_addr, unsigned int len=1){
    std::vector<unsigned int> output;

#ifdef ProcessorHardware

    HwInterface &hi(lHW);

    // assume M0 for now i.e. test
    // Updating registes for LpGBTv1 
    //  i2c_map = {"M0":(0x100,0x16f),
    //             "M1":(0x107,0x184),
    //             "M2":(0x10e,0x199)}
    // ** To do: RMW

    unsigned int i2c_master_base = 0x100;
    unsigned int i2c_master_status_base = 0x16f;
    //unsigned int i2c_speed = 0x3;
    unsigned int i2c_speed = 0x2;

    //emp controller
    Controller  co(hi); 

    //setting region of datapath
    co.getDatapath().selectLink(ch);
    co.getSCC().reset();
  
    if (gbtx_addr == 0x70){
      //Its IC
      hi.getNode("datapath.region.fe_mgt.data_framer.ctrl.ec_ic").write(0);
      hi.dispatch();
    }else{
      // Its EC
      hi.getNode("datapath.region.fe_mgt.data_framer.ctrl.ec_ic").write(1);
      hi.dispatch();
    }
  
    //Write command word for reading 
    co.getSCCIC().icWrite(i2c_master_base+1, target_addr, gbtx_addr);
  
    if (len == 1){
      co.getSCCIC().icWrite(i2c_master_base+6, 0x3 , gbtx_addr);
    }
    else{
      //configure number of words
      co.getSCCIC().icWrite(i2c_master_base+2, (len<<2)+i2c_speed , gbtx_addr);
      co.getSCCIC().icWrite(i2c_master_base+6, 0x0 , gbtx_addr);
      // multi-byte read
      co.getSCCIC().icWrite(i2c_master_base+6, 0xd , gbtx_addr);
    }  
  
   
    //Check the status of the transaction  
    unsigned int status = co.getSCCIC().icRead(i2c_master_status_base+2, gbtx_addr);
    //std::cout << "Status of I2C read transaction : " << std::hex << status <<std::endl;
    int retry = 0;
    while (not (status & 0x4)){
      if (retry > 5) 
	break;
      if (status & 0x40){
	std::cout << "Problem: I2C NACK" << std::endl;
	//raise I2CException("I2C NACK encountered")
      }
      else if (status & 0x8) {
	std::cout << "Problem: SDA low before starting transaction" << std::endl;
	//raise I2CException("SDA low before starting transaction")
      }
      else{
	//std::cout << "Status of I2C read transaction : " << std::hex << status <<std::endl;
      }
      retry ++;
    }
    // Now try reading 
    // Write command word for reading 
  
    unsigned int temp_read=0;
  
    if (len == 1){
      output.push_back(co.getSCCIC().icRead(i2c_master_status_base+4, gbtx_addr));
    }
    else{
      for (unsigned i=0; i<len; i++){
	temp_read = co.getSCCIC().icRead(i2c_master_status_base+20-i, gbtx_addr);
	output.push_back(temp_read); // copy?
      }
    }


#endif

    return output;
  }

  
  void captureTx(uint32_t nTx, std::vector<uint64_t> &packet) {
    packet.resize(0);
    
#ifdef ProcessorHardware

    HwInterface &hi(lHW);
    Controller  aController(hi); 
    
    //setting region of datapath
    //aController.getDatapath().selectRegion(ch/4);
    
    const std::vector<uint32_t> aTxChannels = {nTx};
    const uint32_t aCaptureLength = 1024;
    
    aController.getTTC().forceBTest();
    // And check it's done
    ChannelManager(aController).waitCaptureDone();
    // read out the Rx buffers
    //BoardData aBoardData = ChannelManager(aController, aRxChannels).readBuffers(kRx, aCaptureLength);
    BoardData aBoardData = ChannelManager(aController, aTxChannels).readBuffers(kTx, aCaptureLength);
    
    uint64_t word64;
    unsigned nWord(0);
    
    for (size_t i = 0; i < aBoardData.depth(); i++) {
      std::cout << "Frame " << std::setw(4) << i << "  ";
      bool l56(true);
      bool pack(true);
      
      for (BoardData::const_iterator linkIt = aBoardData.begin(); linkIt != aBoardData.end(); linkIt++) {
	
        const emp::Frame& dataWord = linkIt->second.at(i);
	
	uint32_t v=dataWord.valid;
	if(l56) {
	  word64=dataWord.data;
	  pack=(v==1);
	  if(pack) nWord++;
	}
	
	std::cout << " " << l56 << pack << v << " ";
	
        if (linkIt->second.strobed()) {
	  std::cout << std::setw(1) << (dataWord.strobe);
        }
	std::cout << std::setw(1) << dataWord.startOfOrbit << dataWord.startOfPacket << dataWord.endOfPacket << dataWord.valid;
	std::cout << " " << std::setw(16) << std::hex << (dataWord.data) << std::dec;
	
	l56=false;
      }
      std::cout << std::endl;
      if(pack && nWord!=2 && nWord!=0x7a) packet.push_back(word64);
    }
    
    std::cout << std::hex << std::setfill('0');
    for(unsigned i(0);i<packet.size();i++) {
      std::cout << std::setw(2) << i << " " 
		<< std::setw(16) << packet[i]
		<< std::endl;
    }
    std::cout << std::dec << std::setfill(' ');

#endif

    return;
  }


  int i2c_write(/*HwInterface hi,*/ unsigned char ch, unsigned int gbtx_addr, unsigned int target_addr, std::vector<unsigned int> data) {

#ifdef ProcessorHardware
    HwInterface &hi(lHW);

    // assume M0 for now i.e. test
    // Updating registes for LpGBTv1 
    //  i2c_map = {"M0":(0x100,0x16f),
    //             "M1":(0x107,0x184),
    //             "M2":(0x10e,0x199)}
    // ** To do: RMW
  
    unsigned char len = data.size();

    if (len>16){
      std::cout<<"Error: I2C write can only write 16 bytes in one transaction" << std::endl;
      return -1;
    } 
  
    unsigned int i2c_master_base = 0x100;
    unsigned int i2c_master_status_base = 0x16f;
    //unsigned int i2c_speed = 0x1;
    unsigned int i2c_speed = 0x2;

    //emp controller
    Controller  co(hi); 

    //setting region of datapath
    co.getDatapath().selectLink(ch);
    co.getSCC().reset();
  
    if (gbtx_addr == 0x70){
      //Its IC
      hi.getNode("datapath.region.fe_mgt.data_framer.ctrl.ec_ic").write(0);
      hi.dispatch();
    }else{
      // Its EC
      hi.getNode("datapath.region.fe_mgt.data_framer.ctrl.ec_ic").write(1);
      hi.dispatch();
    }
  
    //set speed, and write nbytes 
    co.getSCCIC().icWrite(i2c_master_base+2, (len<<2)+i2c_speed , gbtx_addr);
    // go
    co.getSCCIC().icWrite(i2c_master_base+6, 0x0 , gbtx_addr);
  
    // Configure write address
    co.getSCCIC().icWrite(i2c_master_base+1, target_addr , gbtx_addr);
    if(len > 1){
      for (int i=0; i<len; i++ ){
	//std::cout << "DEbug: in I2C write : write data byte " << i << " val = " << std::hex << data[i] << std::endl;
	co.getSCCIC().icWrite(i2c_master_base+2+(i%4), data[i] , gbtx_addr);
	if ( (i % 4) == 3 || i == len-1){
	  //end of set of 4
	  std::cout << i %4 << "   " << int(i/4) << std::endl;
	  co.getSCCIC().icWrite(i2c_master_base+6, 0x8+int(i/4) , gbtx_addr);  
	}
                  
      }
  
      //initiate actual write 
      co.getSCCIC().icWrite(i2c_master_base+6, 0xC , gbtx_addr);
    }
    else{
      //std::cout << "DEbug: in I2C write : single write data byte  = " << std::hex << data[0] << std::endl;
      co.getSCCIC().icWrite(i2c_master_base+2, data[0] , gbtx_addr);
      co.getSCCIC().icWrite(i2c_master_base+6, 0x2 , gbtx_addr);
    }

    
    //Check the status of the transaction  
    unsigned int status = co.getSCCIC().icRead(i2c_master_status_base+2, gbtx_addr);
    //std::cout << "Status of I2C write transaction : " << std::hex << status <<std::endl;
    int retry = 0;
    while (not (status & 0x4)){
      if (retry > 5) 
	return -1;
      if (status & 0x40){
	std::cout << "Problem: I2C NACK" << std::endl;
	//raise I2CException("I2C NACK encountered")
      }
      else if (status & 0x8) {
	std::cout << "Problem: SDA low before starting transaction" << std::endl;
	//raise I2CException("SDA low before starting transaction")
      }
      else{
	//std::cout << "Status of I2C write transaction : " << std::hex << status <<std::endl;
      }
        
      retry ++;
    }
  
#endif

    return 0;
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
