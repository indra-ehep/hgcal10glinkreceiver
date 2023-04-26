#ifndef Hgcal10gLinkReceiver_Record_h
#define Hgcal10gLinkReceiver_Record_h

#include <iostream>

#include "RecordHeader.h"

namespace Hgcal10gLinkReceiver {

  template<unsigned NumberOfPayloadWords> class Record : public RecordHeader {
  
  public:
    Record() {
    }

    void setPayloadLength() {
      RecordHeader::setPayloadLength(NumberOfPayloadWords);
    }

    void setPayloadLength(uint16_t l) {
      RecordHeader::setPayloadLength(l);
    }
    
    uint64_t* payload() {
      return _payload;
    }

    void copy(const RecordHeader &h) {
      std::memcpy(this,&h,8*h.totalLength());
    }
    
    void print(std::ostream &o=std::cout) const {
      o << "Record::print()" << std::endl;
      RecordHeader::print(o," ");
      
      for(unsigned i(0);i<payloadLength();i++) {
	o << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(16) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
    }
  protected:
    uint64_t _payload[NumberOfPayloadWords];
  private:
  };

}

#endif
