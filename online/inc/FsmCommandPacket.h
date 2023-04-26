#ifndef FsmCommandPacket_h
#define FsmCommandPacket_h

#include <iostream>
#include <iomanip>

#include "FsmCommand.h"
#include "Record.h"

namespace Hgcal10gLinkReceiver {

  class FsmCommandPacket {
  public:

    FsmCommandPacket() {
    }

    void initialize() {
      _command=FsmCommand::EndOfCommandEnum;
      _record.setState(FsmState::Initial);
      _record.setPayloadLength(0);
      _record.setUtc(0);
    }
    
    FsmCommand::Command command() const {
      return _command;
    }
  
    const RecordHeader& recordHeader() {
      return _record;
    }

    void setCommand(FsmCommand::Command c) {
      _command=c;
      _record.setState(FsmCommand::staticStateBeforeCommand(c));
      _record.setPayloadLength(0);
      _record.setUtc();
    }

    void setRecord(const RecordHeader &h) {
      _record.copy(h);
    }

    void print(std::ostream &o=std::cout) const {
      o << "FsmCommandPacket::print()" << std::endl;
      o << " Command " << FsmCommand::commandName(_command) << std::endl;
      _record.print(o);
    }

  private:
    FsmCommand::Command _command;
    Record<14> _record;
  };

}
#endif
