#ifndef Hgcal10gLinkReceiver_Record_h
#define Hgcal10gLinkReceiver_Record_h

#include <iostream>

#include "RecordHeader.h"

namespace Hgcal10gLinkReceiver {

  template<unsigned NumberOfPayloadWords> class Record : public RecordHeader {
  
  public:
    Record() {
    }

    void setLength() {
      RecordHeader::setLength(NumberOfPayloadWords);
    }
    
    uint64_t* payload() {
      return _payload;
    }
   
    void print(std::ostream &o=std::cout) {
      o << "Record::print()" << std::endl;
      RecordHeader::print(o," ");
      
      for(unsigned i(0);i<length();i++) {
	o << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(8) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
      }
    }
  protected:
    uint64_t _payload[NumberOfPayloadWords];
  private:
  };

}

#endif
