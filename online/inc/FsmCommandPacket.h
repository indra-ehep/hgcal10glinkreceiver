#ifndef FsmCommandPacket_h
#define FsmCommandPacket_h

#include <iostream>
#include <iomanip>

#include "FsmCommand.h"
#include "RecordPrinter.h"

namespace Hgcal10gLinkReceiver {

  class FsmCommandPacket {
  public:

    FsmCommandPacket() {
      coldStart();
    }

    void coldStart() {
      _command=FsmCommand::EndOfCommandEnum;
      _record.setState(FsmState::Initial);
      _record.setPayloadLength(0);
      _record.setUtc(0);
      _record.reset();
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

    void resetRecord() {
      _record.reset();
    }

    void print(std::ostream &o=std::cout) const {
      o << "FsmCommandPacket::print()" << std::endl;
      o << " Command " << FsmCommand::commandName(_command) << std::endl;
      RecordPrinter(&_record,o," ");
    }

  private:
    FsmCommand::Command _command;
    RecordT<14> _record;
  };

}
#endif
