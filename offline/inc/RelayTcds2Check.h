#ifndef Hgcal10gLinkReceiver_RelayTcds2Check_h
#define Hgcal10gLinkReceiver_RelayTcds2Check_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "RecordYaml.h"


namespace Hgcal10gLinkReceiver {

  class RelayTcds2Check {

  public:
  
    RelayTcds2Check() { 
      nRunningRecords_[0]=0;
      nRunningRecords_[1]=0;
      nRunningRecords_[2]=0;
    }
    
    virtual ~RelayTcds2Check() {
    }
    
    void nonRunningRecord(const Record *r) {
      if(rxxx->state()==Hgcal10gLinkReceiver::FsmState::Starting) {
        //if(relayUtc==0) relayUtc=rxxx->utc();
        //runUtc=rxxx->utc();
        //oldBx=0;
        //runCount++;
      }

      if(rxxx->state()==Hgcal10gLinkReceiver::FsmState::Configuration) {
	Hgcal10gLinkReceiver::RecordYaml *rCfg((Hgcal10gLinkReceiver::RecordYaml*)r);
        YAML::Node n(YAML::Load(rCfg->string()));
        
        if(n["Source"].as<std::string>()=="Serenity") {
          std::cout << n << std::endl;
        }
        if(n["Source"].as<std::string>()=="TCDS2") {
          std::cout << n << std::endl;
        }
      }
      
      if(rxxx->state()==Hgcal10gLinkReceiver::FsmState::Status) {
	Hgcal10gLinkReceiver::RecordYaml *rCfg((Hgcal10gLinkReceiver::RecordYaml*)r);
        YAML::Node n(YAML::Load(rCfg->string()));
        
        if(n["Source"].as<std::string>()=="Serenity") {
          std::cout << n << std::endl;
        }
        if(n["Source"].as<std::string>()=="TCDS2") {
          std::cout << n << std::endl;
        }
      }



    }
    
    void runningRecord(const std::vector<const Record*> &r) {
      
      for(unsigned i(0);i<r.size();i++) {
	if(r[i]!=nullptr) {
	  nRunningRecords[i]_++;

	  const Hgcal10gLinkReceiver::SlinkBoe *b(r[i]->slinkBoe());
	  assert(b!=nullptr);
	  b->print();
	  
	  const Hgcal10gLinkReceiver::SlinkEoe *e(r[i]->slinkEoe());
	  assert(e!=nullptr);
	  e->print();
	}
      }
    }

  protected:
    unsigned nRunningRecords_[3];
  };

}

#endif
