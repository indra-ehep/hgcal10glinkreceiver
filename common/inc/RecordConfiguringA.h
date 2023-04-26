#ifndef Hgcal10gLinkReceiver_RecordConfiguringA_h
#define Hgcal10gLinkReceiver_RecordConfiguringA_h

#include <iostream>
#include <iomanip>

#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class RecordConfiguringA : public Record<6> {
  
  public:
    RecordConfiguringA() {
      _payload[0]=0x0000000200000000;
      _payload[1]=0x0000000a00000064;
      _payload[2]=0x0000000000000000;
      _payload[3]=0x0000000200000001;
      _payload[4]=0x0000000400000003;
      _payload[5]=0x0000000600000005;
    }
    
    void setHeader(uint32_t t=time(0)) {
      //setIdentifier(Hgcal10gLinkReceiver::RecordHeader::StateData);
      setState(FsmState::ConfiguringA);
      setPayloadLength();
      setUtc(t);
    }

    uint32_t superRunNumber() const {
      return _payload[0]&0xffffffff;
    }

    uint32_t maxNumberOfRuns() const {
      return _payload[0]>>32;
    }

    uint32_t maxEventsPerRun() const {
      return _payload[1]&0xffffffff;
    }

    uint32_t maxSecondsPerRun() const {
      return _payload[1]>>32;
    }

    uint32_t maxSpillsPerRun() const {
      return _payload[2]&0xffffffff;
    }

    uint32_t processorKey(unsigned k) const {
      if(k>=6) return 0;
      if((k%2)==0) return _payload[3+(k/2)]&0xffffffff;
      else         return _payload[3+(k/2)]>>32; 
    }

    void setSuperRunNumber(uint32_t t=time(0)) {
      _payload[0]&=0xffffffff00000000;
      _payload[0]|=t;
    }
   
    void print(std::ostream &o=std::cout, std::string s="") {
      o << s << "RecordConfiguringA::print()" << std::endl;
      RecordHeader::print(o,s+" ");
      
      for(unsigned i(0);i<payloadLength();i++) {
	o << s << "   Payload word " << std::setw(5) << " = 0x"
	  << std::hex << std::setfill('0')
	  << std::setw(16) << _payload[i]
	  << std::dec << std::setfill(' ') << std::endl;
	if(i==0) {
	  o << s << "    SuperRun number = "
	    << std::setw(10) << superRunNumber() << std::endl;
	  o << s << "    Maximum number of runs = "
	    << std::setw(10) << maxNumberOfRuns() << std::endl;
	}
	o << s << std::dec << std::setfill(' ') << std::endl;
	if(i==1) {
	  o << s << "    Max number of events per run = "
	    << std::setw(10) << maxEventsPerRun() << std::endl;
	  o << s << "    Max seconds per run = "
	    << std::setw(10) << maxSecondsPerRun() << std::endl;
	}
	if(i==2) {
	  o << s << "    Max spills per run = "
	    << std::setw(10) << maxSpillsPerRun() << std::endl;
	}
	if(i>=3) {
	  o << s << "    Process " << i-3 << " key = "
	    << std::setw(10) << processorKey(2*(i-2)  ) << std::endl;
	  o << s << "    Process " << i-3 << " key = "
	    << std::setw(10) << processorKey(2*(i-2)+1) << std::endl;
	}
      }
    }
  private:
  };

}

#endif
