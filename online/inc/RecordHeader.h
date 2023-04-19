#ifndef Hgcal10gLinkReceiver_RecordHeader_h
#define Hgcal10gLinkReceiver_RecordHeader_h

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include "RunControlFsmShm.h"

namespace Hgcal10gLinkReceiver {

  class RecordHeader {
  
  public:
    enum Identifiers {
      FsmStateData=0x33,
      ConfigurationData=0xcc,
      EventData=0xdd,
      EndOfIdentifiersEnum,
    
      // For information
      FileContinuationEof=0xee,
      FileContinuationBof=0xbb,

      SlinkHeader=0xaa,
      SlinkTrailer=0x55
    };

    // Get values
    Identifiers identifier() const {
      return (Identifiers)(_header>>56);
    }

    RunControlFsmShm::FsmStates fsmState() const {
      return (RunControlFsmShm::FsmStates)((_header>>48)&0xff);
    }
  
    uint16_t length() const {
      return (_header>>32)&0xffff;
    }

    uint32_t utc() const {
      return _header&0xffffffff;
    }

    std::string utcDate() const {
      uint64_t t(utc());
      return ctime((time_t*)(&t));
    }

    static const std::string identifierName(Identifiers i) {
      if(i==FsmStateData     ) return "FsmStateData     ";
      if(i==ConfigurationData) return "ConfigurationData";
      if(i==EventData        ) return "EventData        ";
      return _unknown;
    }

    const std::string identifierName() const {
      return identifierName(identifier());
    }

    // Set values
    void setIdentifier(Identifiers i) {
      _header&=0x00ffffffffffffff;
      _header|=(uint64_t(i)<<56);
    }

    void setFsmState(RunControlFsmShm::FsmStates t) {
      _header&=0xff00ffffffffffff;
      _header|=(uint64_t(t)<<48);
    }

    void setLength(uint16_t l) {
      _header&=0xffff0000ffffffff;
      _header|=(uint64_t(l)<<32);

    }

    void setUtc(uint32_t t=time(0)) {
      _header&=0xffffffff00000000;
      _header|=t;
    }
  
    void print(std::ostream &o=std::cout, std::string s="") {
      o << s << "RecordHeader::print()  Data = 0x"
	<< std::hex << std::setfill('0')
	<< _header
	<< std::dec << std::setfill(' ') << std::endl;
      o << s << " Identifier = " << identifierName() << std::endl;
      o << s << " FsmState   = " << RunControlFsmShm::fsmStateName(fsmState())
	<< std::endl;
      o << s << " UTC = " << std::setw(10) << utc() << " = " << utcDate(); // endl already in ctime
      o << s << " Payload length = " << std::setw(5)
	<< length() << " eight-byte words " << std::endl;
    }
  
  private:
    uint64_t _header;

    static const std::string _unknown;
    //static const std::string _identifierNames[EndOfIdentifiersEnum];
  };

  const std::string RecordHeader::_unknown="Unknown";
  /*
  const std::string RecordHeader::_identifierNames[EndOfIdentifiersEnum]={
    "FsmStateData     ",
    "ConfigurationData",
    "EventData        "
  };
  */

}

#endif
