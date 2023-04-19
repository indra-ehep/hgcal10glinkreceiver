#include "PreConfiguringRecord.h"
#include "EventRecord.h"

int main(int argc, char *argv[]) {
  Hgcal10gLinkReceiver::RecordHeader rh;
  rh.setIdentifier(Hgcal10gLinkReceiver::RecordHeader::FsmStateData);
  rh.setFsmState(RunControlFsmShm::PreConfiguring);
  rh.setLength(7);
  rh.setUtc();

  rh.print();

  Hgcal10gLinkReceiver::Record<4> r;
  
  r.setIdentifier(Hgcal10gLinkReceiver::RecordHeader::EventData);
  r.setFsmState(RunControlFsmShm::Running);
  r.RecordHeader::setLength(4);
  r.setUtc();

  r.payload()[0]=1;
  r.payload()[1]=2;
  r.payload()[2]=3;
  r.payload()[3]=4;

  r.print();

  Hgcal10gLinkReceiver::PreConfiguringRecord pcr;
  pcr.setHeader();
  pcr.setSuperRunNumber();
  pcr.print();

  Hgcal10gLinkReceiver::EventRecord er;
  er.setHeader();
  er.print();
  
  return 0;
}
