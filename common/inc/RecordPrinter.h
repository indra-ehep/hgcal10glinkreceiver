#ifndef Hgcal10gLinkReceiver_RecordPrinter_h
#define Hgcal10gLinkReceiver_RecordPrinter_h

#include <iostream>
#include <iomanip>

#include "RecordConfigured.h"
#include "RecordConfiguring.h"
#include "RecordReconfiguring.h"
#include "RecordContinuing.h"
#include "RecordEnding.h"
#include "RecordHalting.h"
#include "RecordInitializing.h"
#include "RecordPausing.h"
#include "RecordResetting.h"
#include "RecordResuming.h"
#include "RecordRunning.h"
#include "RecordStarting.h"
#include "RecordStopping.h"

namespace Hgcal10gLinkReceiver {

  void RecordPrinter(const Record *h, std::ostream &o=std::cout, std::string s="") {
    if     (h->state()==FsmState::Configured   ) ((const RecordConfigured*   )h)->print(o,s);
    else if(h->state()==FsmState::Configuring  ) ((const RecordConfiguring*  )h)->print(o,s);
    else if(h->state()==FsmState::Reconfiguring) ((const RecordReconfiguring*)h)->print(o,s);
    else if(h->state()==FsmState::Continuing   ) ((const RecordContinuing*   )h)->print(o,s);
    else if(h->state()==FsmState::Ending       ) ((const RecordEnding*       )h)->print(o,s);
    else if(h->state()==FsmState::Halting      ) ((const RecordHalting*      )h)->print(o,s);
    else if(h->state()==FsmState::Initializing ) ((const RecordInitializing* )h)->print(o,s);
    else if(h->state()==FsmState::Pausing      ) ((const RecordPausing*      )h)->print(o,s);
    else if(h->state()==FsmState::Resetting    ) ((const RecordResetting*    )h)->print(o,s);
    else if(h->state()==FsmState::Resuming     ) ((const RecordResuming*     )h)->print(o,s);
    else if(h->state()==FsmState::Running      ) ((const RecordRunning*      )h)->print(o,s);
    else if(h->state()==FsmState::Starting     ) ((const RecordStarting*     )h)->print(o,s);
    else if(h->state()==FsmState::Stopping     ) ((const RecordStopping*     )h)->print(o,s);
    else h->print(o,s);
  }

  void RecordPrinter(const Record &h, std::ostream &o=std::cout, std::string s="") {
    RecordPrinter(&h,o,s);
  }
}

#endif
