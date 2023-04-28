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
  
    const Record& record() {
      return _record;
    }

    void setCommand(FsmCommand::Command c) {
      _command=c;
      //assert(_record.state()==FsmCommand::staticStateAfterCommand(c));
      //_record.setPayloadLength(0);
      //_record.setUtc();
    }

    void setRecord(const Record &h) {
      _record.deepCopy(h);
    }

    void print(std::ostream &o=std::cout) const {
      o << "FsmCommandPacket::print()" << std::endl;
      o << " Command " << FsmCommand::commandName(_command) << std::endl;
      _record.print(o);
    }

  private:
    FsmCommand::Command _command;
    RecordT<14> _record;
  };

}
#endif
