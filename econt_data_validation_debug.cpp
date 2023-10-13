#include <iostream>

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TSystem.h"

#include "TFileHandlerLocal.h"
#include "common/inc/FileReader.h"

#include <deque>

// Author : Indranil Das
// Email : Indranil.Das@cern.ch
// Affiliation : Imperial College, London

// Disclaimer : The main framework has been developped by Paul Dauncey and
// a significant part of the current code is collected from Charlotte's development.

//Command to execute : 1. ./compile.sh
//                     2. ./econt_data_validation.exe $Relay $rname

using namespace std;

typedef struct{

  uint64_t eventId;
  uint32_t sequenceId;
  uint16_t l1aType;
  uint16_t ECONT_packet_status[2][12];                //2 : LSB/MSB, 12 : STC
  bool ECONT_packet_validity[2][12];                 //2 : LSB/MSB, 12 : STC
  uint16_t OC[2],EC[2];                              //2 : LSB/MSB
  uint16_t BC[2];                                    //2 : LSB/MSB
  uint16_t bxId;                                     //bxId
  
  uint16_t daq_data[5];                              //5 : data blocks separated by 0xfecafe
  uint16_t daq_nbx[5];                               //5 : data blocks separated by 0xfecafe
  uint16_t size_in_cafe[5];                          //5 : data blocks separated by 0xfecafe
  
  uint32_t energy_raw[2][15][12];                    //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t loc_raw[2][15][12];                       //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t bx_raw[2][15][12];                       //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t energy_unpkd[2][15][12];                  //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t loc_unpkd[2][15][12];                     //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t bx_unpkd[2][15][12];                       //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  
} econt_event;


uint64_t Abs32(uint32_t a, uint32_t b)
{
  if(a>b)
    return (a-b);
  else if(b>a)
    return (b-a);
  else
    return 0;
}

uint64_t Abs64(uint64_t a, uint64_t b)
{
  if(a>b)
    return (a-b);
  else if(b>a)
    return (b-a);
  else
    return 0;
}

int read_Payload_Version(unsigned refRelay)
{
  const char* inDir = "/eos/cms/store/group/dpg_hgcal/tb_hgcal/2023/BeamTestSep/HgcalBeamtestSep2023";
  
  char* dir = gSystem->ExpandPathName(inDir);
  void* dirp = gSystem->OpenDirectory(dir);
  
  const char* entry;
  Int_t n = 0;
  TString str;
  
  int prevRelay = 0;
  string configname;
  string prevConfigname = "";
  while((entry = (char*)gSystem->GetDirEntry(dirp))) {
    str = entry;
    if(str.EndsWith(".yaml") and str.Contains("Serenity")){
      string str1 = str.Data();
      string s_pat = "Constants";
      TString str2 = str1.substr( s_pat.size(), str1.find_first_of("_")-s_pat.size());
      //cout<< "str : " << str << ", relay : " << str2.Atoi() << " prevRelay : " << prevRelay << endl;
      int relay = str2.Atoi();
      configname = gSystem->ConcatFileName(dir, entry);
      if(relay>int(refRelay) and int(refRelay)>=prevRelay) {
	//cout<<"Found config : " << relay << endl;
	break;
	//return;
      }
      prevRelay = relay;
      prevConfigname = configname;
    }
  }
  cout<<"Prev config name : "<<prevConfigname<<endl;
  cout<<"Config name : "<<configname<<endl;

  ifstream fin(prevConfigname);
  string s;
  string s_pat = "PayloadVersion: ";
  int version = 0;
  while(getline(fin,s)){
    str = s;
    if(str.Contains(s_pat)){
      TString str2 = s.substr( s_pat.size(), s.size()-s_pat.size());
      //cout << "Version : " << str2.Atoi() << endl;
      version  = str2.Atoi();
    }
  }
  
  return version;
}

// print all (64-bit) words in event
void event_dump(const Hgcal10gLinkReceiver::RecordRunning *rEvent){
  const uint64_t *p64(((const uint64_t*)rEvent)+1);
  for(unsigned i(0);i<rEvent->payloadLength();i++){
    std::cout << "Word " << std::setw(3) << i << " ";
    std::cout << std::hex << std::setfill('0');
    std::cout << "0x" << std::setw(16) << p64[i] << std::endl;
    std::cout << std::dec << std::setfill(' ');
  }
  std::cout << std::endl;
}

// print all (32-bit) words from first or second column in the event
void event_dump_32(const Hgcal10gLinkReceiver::RecordRunning *rEvent, bool second){
  const uint64_t *p64(((const uint64_t*)rEvent)+1);
  for(unsigned i(0);i<rEvent->payloadLength();i++){
    std::cout << "Word " << std::setw(3) << i << " ";
    std::cout << std::hex << std::setfill('0');
    const uint32_t word = second ? p64[i] : p64[i] >> 32;                                                      // Is it advisable to convert 64 bit to 32 this way without masking ?
    std::cout << "0x" << std::setw(8) << word << std::endl;
    std::cout << std::dec << std::setfill(' ');
  }
  std::cout << std::endl;
}

bool is_cafe_word(const uint32_t word) {
  if(((word >> 8) & 0xFFFFFF) == 16698110)
    return true; //0xFECAFE                                                      // 0xFF FFFF masking to be there to compare with 16698110 (could have used the hex of this)
  else
    return false;
}
   

// returns location in this event of the n'th 0xfecafe... line
int find_cafe_word(const Hgcal10gLinkReceiver::RecordRunning *rEvent, int n, int cafe_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent)+1);

  if (cafe_word_loc > 0) {
    if (is_cafe_word(p64[cafe_word_loc])) {
      return cafe_word_loc;
    }else
      return cafe_word_loc;
  } 
  else {
    ;//std::cout << "oops" << std::endl;
  }
 
  int cafe_counter = 0;
  int cafe_word_idx = -1;
  for(unsigned i(0);i<rEvent->payloadLength();i++){
    const uint32_t word = p64[i];                                                                                // Is it advisable to convert 64 bit to 32 this way without masking ?
    if (is_cafe_word(word)) { // if word == 0xfeca
      cafe_counter++;       
      if (cafe_counter == n){                                                                                    
        cafe_word_idx = i;
        break;
      }
    }
  }

  if (cafe_word_idx == -1) {
    //std::cerr << "Could not find cafe word" << std::endl;
    return 0;
  }else {
    return cafe_word_idx;
  }
}

// find the packet_no'th packet (4 words) after finding 0xfeca in a word (which denotes the start of the set of packets)
// only useful for accessing the first set of packets (between the first two 0xfeca)
void set_packet(uint32_t packet[4], const Hgcal10gLinkReceiver::RecordRunning *rEvent, bool second, int packet_no, int cafe_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent)+1);

  int start_word_idx = find_cafe_word(rEvent, 1, cafe_word_loc);

  for (unsigned j=0; j<4; j++) {
    int word_idx = start_word_idx + 1 + packet_no*4 + j;
    const uint32_t pack_word = second ? p64[word_idx] : p64[word_idx] >> 32;
    packet[j] = pack_word;
  }
}

// scintillator triggers during BX, find the time within bunch crosssing
// possible output is 0->31 (inclusive)
int get_scintillator_trigger_loc(const Hgcal10gLinkReceiver::RecordRunning *rEvent, int cafe_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent)+1);

  int timing_word_start_idx = find_cafe_word(rEvent, 3, cafe_word_loc) + 6;

  int trigger_loc = 0;
  for(unsigned i(timing_word_start_idx);i<rEvent->payloadLength();i=i+5){
    const uint32_t timing_word = p64[i];

    if (timing_word != 0) {
      // use built in function to find number of leading zeros
      trigger_loc = trigger_loc + __builtin_clz(timing_word); 
      break;
    }
    trigger_loc = trigger_loc + 32;
  }   

  // if could not find trigger location -> exit
  if (trigger_loc == -1) {
    exit(1);
  }

  return trigger_loc;
}

uint32_t pick_bits32(uint32_t number, int start_bit, int number_of_bits) {
  // Create a mask to extract the desired bits.
  uint32_t mask = (1 << number_of_bits) - 1;
  // Shift the number to the start bit position.
  number = number >> (32 - start_bit - number_of_bits);
  // Mask the number to extract the desired bits.
  uint32_t picked_bits = number & mask;

  return picked_bits;
}

uint64_t pick_bits64(uint64_t number, int start_bit, int number_of_bits) {
  // Create a mask to extract the desired bits.
  uint64_t mask = (1 << number_of_bits) - 1;
  // Shift the number to the start bit position.
  number = number >> (64 - start_bit - number_of_bits);
  // Mask the number to extract the desired bits.
  uint64_t picked_bits = number & mask;

  return picked_bits;
}

// packet counter is the first 4 bits
uint32_t get_packet_counter(uint32_t* packet) {
  return pick_bits32(packet[0], 0, 4);
}

// 12 locations, 2 bits long, immediately after the packet counter
void set_packet_locations(uint32_t packet_locations[12], uint32_t* packet) {
  for (int i=0; i<12; i++) {
    packet_locations[i] = pick_bits32(packet[0], 4+i*2, 2);
  }
}

uint64_t decode_tc_val(uint64_t value)
{
  uint64_t mant = value & 0x7;
  uint64_t pos  = (value >> 3) & 0xf;

  if(pos==0) return mant << 1 ;

  pos += 2;

  uint64_t decompsum = 1 << pos;
  decompsum |= mant << (pos-3);
  return decompsum << 1 ;
}

uint64_t decode_tc_val_mod(uint64_t value)
{
  uint64_t mant = value & 0x7;
  uint64_t pos  = (value >> 3) & 0xf;
  
  uint64_t decompsum = mant << (pos-3);
  return decompsum ;
}

// 12 energies, 7 bits long, immediately after the packet energies
void set_packet_energies(uint64_t packet_energies[12], uint32_t* packet) {
  uint64_t packet64[4];
  for (int i=0; i<4; i++) {
    packet64[i] = packet[i];
  }

  // need two 64 bit words since all of the energies are 12*7 = 84 bits long
  // word 1 starts with the beginning of the energies
  uint64_t word1 = (packet64[0] << (28+32)) + (packet64[1] << 28) + (packet64[2] >> 4);
  // word 2 are the last 64 bits of the packet (which is 128 bits long)
  uint64_t word2 = (packet64[2] << 32) + packet64[3];

  for (int i=0; i<12; i++) {
    if (i < 9) {
      // first 9 (0->8) energies fit in first word
      packet_energies[i] = pick_bits64(word1, i*7, 7);
    } 
    else {
      // 9th energy starts 27 bits into the second word
      //packet_energies[i] = pick_bits64(word2, 27+(9-i)*7, 7); //by Charlotte (problem ???????)
      packet_energies[i] = pick_bits64(word2, 27+(i-9)*7, 7); //modification by Indra
    } 
  }
}

void PrintLastEvents(deque<econt_event> econt_events)
{
  int nofEventPrint = 10;
  int nofPop = (econt_events.size() > 10) ? (econt_events.size() - nofEventPrint) : 0 ;
  for(int ievent = 0; ievent < nofPop ; ievent++ )
    econt_events.pop_front();
  
  while (!econt_events.empty()) {
    econt_event ev = econt_events.back();
    cout << "\tPrev :: Event ID :  " << ev.eventId
	 << ", OC[LSB] : " << ev.OC[0] << ", OC[MSB] : " << ev.OC[1]
      	 << ", BC[LSB] : " << ev.BC[0] << ", BC[MSB] : " << ev.BC[1]
	 <<endl;
    econt_events.pop_back();
  }
  cout<<endl;

  
}

int main(int argc, char** argv){
  //int econt_data_validation_debug(unsigned relayNumber=1695822887, unsigned runNumber=1695822887){
  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return 1;
  }

  //Command line arg assignment
  //Assign relay and run numbers
  unsigned linkNumber(0);
  bool skipMSB(true);
  
  // ./econt_data_validation.exe $Relay $rname
  unsigned relayNumber(0);
  unsigned runNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  
  int runNumber1 = int(runNumber);

  int payload_version = 0;
  payload_version = read_Payload_Version(relayNumber);
  //cout << "payload_version : " << payload_version << endl;

  //Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;

  //Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);

  //Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  _fileReader.setDirectory(std::string("dat/Relay")+argv[1]);
  //_fileReader.setDirectory(std::string("dat/Relay")+Form("%u",relayNumber));
  _fileReader.openRun(runNumber,linkNumber);
  
  uint64_t prevEvent = 0;
  uint32_t  prevSequence = 0;
  uint64_t nEvents = 0;
  uint32_t packet[4];
  uint32_t packet_counter;
  uint32_t packet_locations[12];
  uint64_t packet_energies[12];
  
  // make histograms 10 trigger types, two ECONTs, and 12 STC 
  TH1I** hloc = 0x0; // locations of most energetic TC in STC
  hloc = new TH1I*[12];        // 12 STCs
  for (int istc=0;istc<12;istc++) {
    hloc[istc] = new TH1I(Form("hloc_%d",istc),Form("ECONT0 STC : %d",istc),6,0,6);
    hloc[istc]->SetMinimum(0.0);
    hloc[istc]->GetXaxis()->SetTitle("TC");
    hloc[istc]->GetYaxis()->SetTitle("Entries");
  }//istc  

  TH1I** hloc1 = 0x0; // locations of most energetic TC in STC
  hloc1 = new TH1I*[12];        // 12 STCs
  for (int istc=0;istc<12;istc++) {
    hloc1[istc] = new TH1I(Form("hloc1_%d",istc),Form("ECONT1 STC : %d",istc),6,0,6);
    hloc1[istc]->SetMinimum(0.0);
    hloc1[istc]->GetXaxis()->SetTitle("TC");
    hloc1[istc]->GetYaxis()->SetTitle("Entries");
  }//istc  
  int nTrigType = 0;
  
  uint64_t nofRStartErrors = 0, nofRStopErrors = 0;
  TH2I *hErrEcont0Status = new TH2I("hErrEcont0Status", "Errors related to ECONT0 packet status",12,0,12,8,0,8);
  TH2I *hErrEcont1Status = new TH2I("hErrEcont1Status", "Errors related to ECONT1 packet status",12,0,12,8,0,8);
  TH1I *hDaqEvtMisMatch = new TH1I("hDaqEvtMisMatch", "Event size mismatch between RO and cafe header",5,0,5);
  uint64_t nofEventIdErrs = 0;
  uint64_t nofFirstFECAFEErrors = 0;
  uint64_t nofExcessFECAFEErrors = 0;
  uint64_t nofNbxMisMatches = 0;
  uint64_t nofSTCNumberingErrors = 0;
  uint64_t nofSTCLocErrors = 0;
  uint64_t nofEnergyMisMatches = 0; 
  uint64_t nofEmptyTCs = 0;
  uint64_t nofBxRawUnpkMM = 0;
  uint64_t nofBxCentralMM = 0;

  uint64_t nofSTCNumberingErrors1 = 0;
  uint64_t nofSTCLocErrors1 = 0;
  uint64_t nofEnergyMisMatches1 = 0; 
  uint64_t nofEmptyTCs1 = 0;
  uint64_t nofBxRawUnpkMM1 = 0;
  uint64_t nofBxCentralMM1 = 0;
  
  // to cache where the cafe separators are
  int scintillator_cafe_word_loc;
  int econt_cafe_word_loc;
  uint64_t total_phys_events = 0;
  uint64_t total_coinc_events = 0;
  uint64_t total_calib_events = 0;
  uint64_t total_random_events = 0;
  uint64_t total_soft_events = 0;
  uint64_t total_regular_events = 0;
  
  //Keep a record of status of last 100 events
  uint64_t  max_event_entries = 100;
  deque<econt_event> econt_events;
  
  bool isGood = true;

  bool isSTCNE = false;
  bool isSTCLE = false;
  bool isEngE = false;
  bool isBxE = false;
  bool isBxCE = false;
  bool isSTCNE1 = false;
  bool isSTCLE1 = false;
  bool isEngE1 = false;
  bool isBxE1 = false;
  bool isBxCE1 = false;

  uint64_t nofEvcSTCNE = 0;
  uint64_t nofEvcSTCLE = 0;
  uint64_t nofEvcEngE = 0;
  uint64_t nofEvcBxE = 0;
  uint64_t nofEvcBxCE = 0;
  uint64_t nofEvcSTCNE1 = 0;
  uint64_t nofEvcSTCLE1 = 0;
  uint64_t nofEvcEngE1 = 0;
  uint64_t nofEvcBxE1 = 0;
  uint64_t nofEvcBxCE1 = 0;

  //Use the fileReader to read the records
  while(_fileReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	isGood = false;
  	rStart->print();
  	std::cout << std::endl;
	nofRStartErrors++;
	continue;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
  	isGood = false;
  	rStop->print();
  	std::cout << std::endl;
	nofRStopErrors++;
	continue;
      }
    }
    //Else we have an event record 
    else{
      
      //if(nEvents>=2) continue;
      
      // if (nEvents >= 150 && nEvents <= 153)
      // if (nEvents < 2) 
      // 	event_dump(rEvent);
      
      const Hgcal10gLinkReceiver::SlinkBoe *boe = rEvent->slinkBoe();      
      const Hgcal10gLinkReceiver::SlinkEoe *eoe = rEvent->slinkEoe();
      
      if (nEvents < 2) {
  	// // const uint64_t *p64(((const uint64_t*)rEvent)+1);
  	// // std::cout << std::hex << std::setfill('0');
  	// // std::cout << "Word 0 0x" << std::setw(16) << p64[0] << std::endl;
  	// // std::cout << std::dec << std::setfill(' ');
	
  	// // cout<<"Event : " <<nEvents<<" and boe::eventID : " <<boe->eventId()<<endl;
  	// // cout<<"\n MiniDAQ header for Slink 0 LSB : " << endl;
  	// // for(int istc = 0 ; istc < 12 ; istc++){
  	// //   int shift = istc*3;
  	// //   cout<<istc<< ", packet status : " << (p64[0]>>shift & 0x7) <<endl;
  	// // }
	
  	// // std::cout << std::hex << std::setfill('0');
  	// // cout<<"OC : 0x"<< (p64[0]>>(12*3) & 0x7) << endl;
  	// // cout<<"EC : 0x"<< (p64[0]>>39 & 0x3F) << endl;
  	// // cout<<"BC : 0x"<< (p64[0]>>45 & 0xFFF) << endl;
  	// // std::cout << std::dec << std::setfill(' ');

  	// // 	std::cout << std::hex << std::setfill('0');
  	// // 	std::cout << "Word 0 0x" << std::setw(16) << p64[1] << std::endl;
  	// // 	std::cout << std::dec << std::setfill(' ');
	

  	// // 	cout<<"MiniDAQ header for Slink 1 MSB : " << endl;
  	// // 	for(int istc = 0 ; istc < 12 ; istc++){
  	// // 	  int shift = istc*3;
  	// // 	  cout<<istc<< ", packet status : " << (p64[1]>>shift & 0x7) <<endl;
  	// // false;
  	// // 	}
  	// // 	std::cout << std::hex << std::setfill('0');
  	// // 	cout<<"OC : 0x"<< (p64[1]>>(12*3) & 0x7) << endl;
  	// // 	cout<<"EC : 0x"<< (p64[1]>>39 & 0x3F) << endl;
  	// // 	cout<<"BC : 0x"<< (p64[1]>>45 & 0xFFF) << endl;
  	// // 	std::cout << std::dec << std::setfill(' ');

	rEvent->RecordHeader::print();
	boe->print();
  	// // const Hgcal10gLinkReceiver::BePacketHeader *beheader = rEvent->bePacketHeader();
  	// // beheader->print();
  	eoe->print();
  	// //std::cout << std::endl;
  	// //event_dump(rEvent);
  	// // event_dump_32(rEvent, false);
  	// // event_dump_32(rEvent, true);
      }
      if(boe->boeHeader()!=boe->BoePattern) continue;
      if(eoe->eoeHeader()!=eoe->EoePattern) continue;
      
      uint16_t l1atype = boe->l1aType();      
      if(l1atype==0) continue;

      //Reset bool for event counters
      isSTCNE = false;
      isSTCLE = false;
      isEngE = false;
      isBxE = false;
      isBxCE = false;
      isSTCNE1 = false;
      isSTCLE1 = false;
      isEngE1 = false;
      isBxE1 = false;
      isBxCE1 = false;

      //Increment event counter and reset error state
      nEvents++;
      const uint64_t *p64(((const uint64_t*)rEvent)+1);

      if(l1atype==0x0001)
      	total_phys_events++;
      if(l1atype==0x0002)
      	total_calib_events++;
      if(l1atype==0x0003)
      	total_coinc_events++;
      if(l1atype==0x0004)
      	total_random_events++;
      if(l1atype==0x0008)
      	total_soft_events++;
      if(l1atype==0x0010)
      	total_regular_events++;
      nTrigType = 4; //0 inclusive, 1: physics, 2: coincident, 3: regular
      
      econt_event ev;
      ev.eventId = boe->eventId();
      ev.l1aType = boe->l1aType();
      ev.bxId = eoe->bxId();
      ev.sequenceId = rEvent->RecordHeader::sequenceCounter(); 

      if(ev.bxId==3564 and (nEvents < 30)){
	std::cerr<<"Found bx 3564"<<std::endl;
      }

      if(nEvents%1000000==0)
	//if(nEvents%1000==0)
	std::cerr << "Before:Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", and prevEvent  "<< prevEvent << ", nEvents : " << nEvents << " differs by "<< Abs64(ev.eventId,prevEvent) <<" (sequence differs by [ "<< ev.sequenceId << " - "<< prevSequence << " ] = " << Abs32(ev.sequenceId, prevSequence) << "), EventID_Diff is more than 2 " << std::endl;
      
      if((Abs64(ev.eventId,prevEvent) != Abs32(ev.sequenceId, prevSequence)) and Abs64(ev.eventId,prevEvent)>2){
      //if( TMath::Abs(Long64_t(ev.eventId - prevEvent)) >100){
	isGood = false;
	std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", and prevEvent  "<< prevEvent << ", nEvents : " << nEvents << " differs by "<< Abs64(ev.eventId,prevEvent) <<" (sequence differs by [ "<< ev.sequenceId << " - "<< prevSequence << " ] = " << Abs32(ev.sequenceId, prevSequence) << "), EventID_Diff is more than 2 " << std::endl;
	event_dump(rEvent);
	rEvent->RecordHeader::print();
	boe->print();
	eoe->print();
	nofEventIdErrs++;
	prevEvent = boe->eventId();
	prevSequence = rEvent->RecordHeader::sequenceCounter(); 
	continue;
	//break;
      }

      prevEvent = ev.eventId;
      prevSequence = ev.sequenceId;
      
      // for(int istc = 0 ; istc < 12 ; istc++){
      // 	int shift = istc*3;
      // 	ev.ECONT_packet_status[0][istc] = (p64[0]>>shift & 0x7);
      // 	ev.ECONT_packet_status[1][istc] = (p64[1]>>shift & 0x7);
      // 	ev.ECONT_packet_validity[0][istc] = (ev.ECONT_packet_status[0][istc]==0x0) ? true : false;
      // 	ev.ECONT_packet_validity[1][istc] = (ev.ECONT_packet_status[1][istc]==0x0) ? true : false;
      // 	if(ev.ECONT_packet_validity[0][istc] == false){
      // 	  isGood = false;
      // 	  hErrEcont0Status->Fill(istc, ev.ECONT_packet_status[0][istc]);
      // 	  //std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", LSB ECONT packet validity failed for STC  "<< istc << " with status value " << ev.ECONT_packet_status[0][istc] << std::endl;
      // 	  //PrintLastEvents(econt_events);
      // 	  continue;
      // 	}
      // 	if(ev.ECONT_packet_validity[1][istc] == false){
      // 	  isGood = false;
      // 	  hErrEcont1Status->Fill(istc, ev.ECONT_packet_status[1][istc]);
      // 	  //std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", MSB ECONT packet validity failed for STC  "<< istc << " with status value " << ev.ECONT_packet_status[1][istc] << std::endl;
      // 	  //PrintLastEvents(econt_events);
      // 	  continue;
      // 	}
	
      // }
      // ev.OC[0] = (p64[0]>>(12*3) & 0x7) ;
      // ev.OC[1] = (p64[1]>>(12*3) & 0x7) ;
      // ev.EC[0] = (p64[0]>>39 & 0x3F) ;
      // ev.EC[1] = (p64[1]>>39 & 0x3F) ;
      // ev.BC[0] = (p64[0]>>45 & 0xFFF) ;
      // ev.BC[1] = (p64[1]>>45 & 0xFFF) ;



      //std::cout << std::endl;
      //const uint64_t *p64(((const uint64_t*)rEvent)+1);
      ev.daq_data[0] = p64[2] & 0xF;
      ev.daq_nbx[0] = p64[2]>>4 & 0x7;
      uint64_t daq0_event_size = (2*ev.daq_nbx[0] + 1)*ev.daq_data[0];
      int first_cafe_word_loc = find_cafe_word(rEvent, 1);
      ev.size_in_cafe[0] = p64[first_cafe_word_loc] & 0xFF;      
									
      ev.daq_data[1] = p64[2]>>7 & 0xF;
      ev.daq_nbx[1] = p64[2]>>11 & 0x7;
      uint64_t daq1_event_size = (2*ev.daq_nbx[1] + 1)*ev.daq_data[1];
      int second_cafe_word_loc = find_cafe_word(rEvent, 2);
      ev.size_in_cafe[1] = p64[second_cafe_word_loc] & 0xFF;
      
      ev.daq_data[2] = p64[2]>>14 & 0xF;
      ev.daq_nbx[2] = p64[2]>>18 & 0x7;
      uint64_t daq2_event_size = (2*ev.daq_nbx[2] + 1)*ev.daq_data[2];
      int third_cafe_word_loc = find_cafe_word(rEvent, 3);
      ev.size_in_cafe[2] = p64[third_cafe_word_loc] & 0xFF;
      
      ev.daq_data[3] = p64[2]>>21 & 0xF;
      ev.daq_nbx[3] = p64[2]>>25 & 0x7;
      uint64_t daq3_event_size = (2*ev.daq_nbx[3] + 1)*ev.daq_data[3];
      int fourth_cafe_word_loc = find_cafe_word(rEvent, 4);
      ev.size_in_cafe[3] = p64[fourth_cafe_word_loc] & 0xFF;

      ev.daq_data[4] = p64[2]>>28 & 0xF;
      ev.daq_nbx[4] = p64[2]>>32 & 0x7;
      uint64_t daq4_event_size = (2*ev.daq_nbx[4] + 1)*ev.daq_data[4];
      int fifth_cafe_word_loc = find_cafe_word(rEvent, 5);
      ev.size_in_cafe[4] = p64[fifth_cafe_word_loc] & 0xFF;
      

      int sixth_cafe_word_loc = find_cafe_word(rEvent, 6);
      if(sixth_cafe_word_loc!=0){
	nofExcessFECAFEErrors++ ;
	continue;
      }


      if(first_cafe_word_loc != 3){
  	std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", Corrupted header need to skip event as the first 0xfecafe word is at  "<< first_cafe_word_loc << " instead of ideal location 3." << std::endl;
  	isGood = false;
	//event_dump(rEvent);
	// rEvent->RecordHeader::print();
	// boe->print();
	// eoe->print();
	nofFirstFECAFEErrors++ ;
	//PrintLastEvents(econt_events);
	//break;	
	continue;
      }

      if(daq0_event_size != ev.size_in_cafe[0]){
  	std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", Event size do not match between trigger RO header "<< daq0_event_size << " and first 0xfecafe word " << ev.size_in_cafe[0] << std::endl;
  	isGood = false;
	hDaqEvtMisMatch->Fill(0);
	//PrintLastEvents(econt_events);
	continue;	
      }
      if(daq1_event_size != ev.size_in_cafe[1]){
  	std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", Event size do not match between trigger RO header "<< daq1_event_size << " and second 0xfecafe word " << ev.size_in_cafe[1] << std::endl;
  	isGood = false;
	hDaqEvtMisMatch->Fill(1);
	//PrintLastEvents(econt_events);
	continue;	
      }
      if(daq2_event_size != ev.size_in_cafe[2]){
  	std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", Event size do not match between trigger RO header "<< daq2_event_size << " and third 0xfecafe word " << ev.size_in_cafe[2] << std::endl;
  	isGood = false;
	hDaqEvtMisMatch->Fill(2);
	//PrintLastEvents(econt_events);
	continue;
      }
      if(daq3_event_size != ev.size_in_cafe[3]){
  	std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", Event size do not match between trigger RO header "<< daq3_event_size << " and fourth 0xfecafe word " << ev.size_in_cafe[3] << std::endl;
  	isGood = false;
	hDaqEvtMisMatch->Fill(3);
	//PrintLastEvents(econt_events);
	continue;
      }
      if(daq4_event_size != ev.size_in_cafe[4]){
  	std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", Event size do not match between trigger RO header "<< daq4_event_size << " and fifth 0xfecafe word " << ev.size_in_cafe[4] << std::endl;
  	isGood = false;
	hDaqEvtMisMatch->Fill(4);
	//PrintLastEvents(econt_events);
	continue;
      }


      if(ev.daq_nbx[0]!=ev.daq_nbx[1]){
  	//std::cerr << "Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", Bx size do not match between packed " << ev.daq_nbx[0] << " and unpacked data " << ev.daq_nbx[1] << std::endl;
  	isGood = false;
	nofNbxMisMatches++;
	//PrintLastEvents(econt_events);
	continue;
      }
      
      
      int bx_index = -1.0*int(ev.daq_nbx[0]);
      const int maxnbx = (2*ev.daq_nbx[0] + 1);
      uint64_t energy_raw[2][maxnbx][12];
      for(int iect=0;iect<2;iect++)
  	for(int ibx=0;ibx<maxnbx;ibx++)
  	  for(int istc=0;istc<12;istc++){
  	    energy_raw[iect][ibx][istc] = 0;
  	    ev.energy_raw[iect][ibx][istc] = 0;
  	    ev.loc_raw[iect][ibx][istc] = 0;
	    ev.bx_raw[iect][ibx][istc] = 0;
  	  }
      if (nEvents < 2)
       	event_dump(rEvent);

      for(unsigned i(first_cafe_word_loc+1);i<daq0_event_size+first_cafe_word_loc+1;i=i+4){
	
      	const uint32_t word = (p64[i] & 0xFFFFFFFF) ;
      	const uint32_t bx_counter = ( word >> 28 ) & 0xF;
	
      	packet[0] = (p64[i] & 0xFFFFFFFF) ;
  	packet[1] = (p64[i+1] & 0xFFFFFFFF) ;
  	packet[2] = (p64[i+2] & 0xFFFFFFFF) ; 
  	packet[3] = (p64[i+3] & 0xFFFFFFFF) ;
	
      	set_packet_locations(packet_locations, packet);
      	set_packet_energies(packet_energies, packet);
      	//if(boe->l1aType()==0x3){
      	//std::cout<<" boe->l1aType() : " << boe->l1aType() << std::endl;
      	//boe->print();

      	if (nEvents < 2) {
	  std::cout<< "\nword index  : " <<i<< ", ev.daq_nbx[0] : " << ev.daq_nbx[0] << ", ev.daq_nbx[1] : " << ev.daq_nbx[1] 
		   << ", bx_index : " << (bx_index+int(ev.daq_nbx[0])) 
		   << std::hex << std::setfill('0')
		   <<", \nbx(LSB) : 0x" << std::setw(1) << bx_counter 
		   << std::dec << std::setfill(' ') 
		   <<std::endl;
  	  std::cout<<"E(LSB) : \t";
      	  for(int istc=0;istc<12;istc++){
      	    std::cout<<packet_energies[istc]<<" ";
      	  }
      	  std::cout<<std::endl;
	  std::cout<<"L(LSB) : \t";
      	  for(int istc=0;istc<12;istc++){
	    std::cout<<packet_locations[istc]<<" ";
      	  }
      	  std::cout<<std::endl;
      	}
	
      	for(int istc=0;istc<12;istc++){
	  // cout<<"istc : " << istc 
	  //     << ", packet_locations[istc] " << packet_locations[istc] 
	  //     <<endl;
	  // cout<< ", hist : " << hloc[istc] 
	  //     << ", hist entries : " << hloc[istc]->GetEntries() 
	  //     << endl;
	  hloc[istc]->Fill(packet_locations[istc]);
	  
      	  energy_raw[0][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_energies[istc];
  	  ev.energy_raw[0][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_energies[istc];
  	  ev.loc_raw[0][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_locations[istc];
	  ev.bx_raw[0][(bx_index+int(ev.daq_nbx[0]))][istc] = bx_counter;
      	}
	
  	const uint32_t word1 = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	const uint32_t bx_counter1 = ( word1 >> 28 ) & 0xF;
	
      	packet[0] = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	packet[1] = ((p64[i+1] >> 32) & 0xFFFFFFFF) ;
      	packet[2] = ((p64[i+2] >> 32) & 0xFFFFFFFF) ;
      	packet[3] = ((p64[i+3] >> 32) & 0xFFFFFFFF) ;
	
      	set_packet_locations(packet_locations, packet);
      	set_packet_energies(packet_energies, packet);

      	if (nEvents < 2) {
	  std::cout<< std::hex << std::setfill('0')
		   <<"bx(MSB) : 0x" << std::setw(1) << bx_counter1 
		   << std::dec << std::setfill(' ') 
		   <<std::endl;
  	  std::cout<<"E(MSB) : \t";
      	  for(int istc=0;istc<12;istc++){
      	    std::cout<<packet_energies[istc]<<" ";
      	  }
      	  std::cout<<std::endl;
	  std::cout<<"L(MSB) : \t";
      	  for(int istc=0;istc<12;istc++){
	    std::cout<<packet_locations[istc]<<" ";
      	  }
      	  std::cout<<std::endl;
      	}
	
      	for(int istc=0;istc<12;istc++){
	  
      	  //hloc[0][1][istc]->Fill(packet_locations[istc]);
	  hloc1[istc]->Fill(packet_locations[istc]);
	  
      	  energy_raw[1][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_energies[istc];
  	  ev.energy_raw[1][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_energies[istc];
  	  ev.loc_raw[1][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_locations[istc];
	  ev.bx_raw[1][(bx_index+int(ev.daq_nbx[0]))][istc] = bx_counter1;
      	}
	
      	bx_index++;
      }
      

      // std::cout << std::endl;
      uint32_t energy_unpkd[2][maxnbx][12];
      for(int iect=0;iect<2;iect++)
  	for(int ibx=0;ibx<maxnbx;ibx++)
  	  for(int istc=0;istc<12;istc++){
  	    energy_unpkd[iect][ibx][istc] = 0;
  	    ev.energy_unpkd[iect][ibx][istc] = 0;
  	    ev.loc_unpkd[iect][ibx][istc] = 0;
  	    ev.bx_unpkd[iect][ibx][istc] = 0;
  	  }
      
      int index_stc = 0;
      int index_ibx = 0;
      uint32_t prev_bx_counter = 0xFF;

      // if (nEvents < 2){
      // 	event_dump(rEvent);
      // 	cout << "second_cafe_word_loc : " << second_cafe_word_loc <<endl;
      // }

      for(unsigned i(second_cafe_word_loc+1);i<daq1_event_size+second_cafe_word_loc+1;i++){
	  
      	const uint32_t word = (p64[i] & 0xFFFFFFFF) ;
      	const uint32_t word1 = ((p64[i] >> 32) & 0xFFFFFFFF) ;
	  
      	const uint32_t energy1 = word & 0x7F;
      	const uint32_t energy1_loc = (word >> 7) & 0x3F;
      	const uint32_t energy2 = (word >> 13) & 0x7F;
      	const uint32_t energy2_loc = (word >> (13+7)) & 0x3F;
      	const uint32_t bx_counter = ( word >> 26 ) & 0xF;

      	const uint32_t energy3 = word1 & 0x7F;
      	const uint32_t energy3_loc = (word1 >> 7) & 0x3F;
      	const uint32_t energy4 = (word1 >> 13) & 0x7F;
      	const uint32_t energy4_loc = (word1 >> (13+7)) & 0x3F;
      	const uint32_t bx_counter1 = ( word1 >> 26 ) & 0xF;
	
      	// if(bx_counter != prev_bx_counter){
      	//   prev_bx_counter = bx_counter;
      	//   if(index_stc!=0)
      	//     index_ibx++;
	//   index_stc = 0;
      	// }

      	energy_unpkd[0][index_ibx][index_stc] = energy1;
      	energy_unpkd[0][index_ibx][index_stc+6] = energy2;
      	energy_unpkd[1][index_ibx][index_stc] = energy3;
      	energy_unpkd[1][index_ibx][index_stc+6] = energy4;
	
      	ev.energy_unpkd[0][index_ibx][index_stc] = energy1;
      	ev.energy_unpkd[0][index_ibx][index_stc+6] = energy2;
      	ev.energy_unpkd[1][index_ibx][index_stc] = energy3;
      	ev.energy_unpkd[1][index_ibx][index_stc+6] = energy4;	

  	ev.loc_unpkd[0][index_ibx][index_stc] = energy1_loc;
      	ev.loc_unpkd[0][index_ibx][index_stc+6] = energy2_loc;
      	ev.loc_unpkd[1][index_ibx][index_stc] = energy3_loc;
      	ev.loc_unpkd[1][index_ibx][index_stc+6] = energy4_loc;
	
	
	if (nEvents < 2){
	  std::cout <<"\nnEvents : " << nEvents << ", i : " << i
		    << std::hex << std::setfill('0')
		    << ", Unpackaer MSBLSB  0x" << std::setw(16) << p64[i]
		    << std::dec << std::setfill(' ') 
		    << endl;

	  std::cout<< std::hex << std::setfill('0')
	           //<< ", Unpackaer LSB  0x" << std::setw(8) << word 
		   << " bx_counter_LSB 0x" << std::setw(1) << bx_counter
		   << std::dec << std::setfill(' ') 
		    << ", index_ibx : " << index_ibx
		    << ", index_stc : " << index_stc
		    <<endl;
	  std::cout << "E1(LSB) :" << energy1 << ", L1 : " << (energy1_loc & 0x3) << ", STC_index : " << (energy1_loc>>2 & 0xF) << endl;
	  std::cout << "E2(LSB) :" << energy2 << ", L2 : " << (energy2_loc & 0x3) << ", STC_index : " << (energy2_loc>>2 & 0xF) << endl;
	  std::cout<< std::hex << std::setfill('0')
	           //<< ", Unpackaer MSB  0x" << std::setw(8) << word1
		   << " bx_counter_MSB 0x" << std::setw(1) << bx_counter1
		   << std::dec << std::setfill(' ') 
		    << ", index_ibx : " << index_ibx
		    << ", index_stc : " << index_stc
		    <<endl;
	  std::cout << "E1(MSB) :" << energy3 << ", L1 : " << (energy3_loc & 0x3) << ", STC_index : " << (energy3_loc>>2 & 0xF) << endl;
	  std::cout << "E2(MSB) :" << energy4 << ", L2 : " << (energy4_loc & 0x3) << ", STC_index : " << (energy4_loc>>2 & 0xF) << endl;
	}
	ev.bx_unpkd[0][index_ibx][index_stc] = bx_counter;
	ev.bx_unpkd[0][index_ibx][index_stc+6] = bx_counter;
      	ev.bx_unpkd[1][index_ibx][index_stc] = bx_counter1;
      	ev.bx_unpkd[1][index_ibx][index_stc+6] = bx_counter1;	
	
	if(index_stc==5)
	  index_stc = 0;
	else
	  index_stc++;
	if(index_stc==0)
	  index_ibx++;
      }

      //std::cerr << "After6:Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", and prevEvent  "<< prevEvent << ", nEvents : " << nEvents << " differs by "<< Abs64(ev.eventId,prevEvent) <<" (sequence differs by [ "<< ev.sequenceId << " - "<< prevSequence << " ] = " << Abs32(ev.sequenceId, prevSequence) << "), EventID_Diff is more than 2 " << std::endl;
      // if (nEvents < 3) {
	
      // 	std::cout << std::hex << std::setfill('0');
      // 	std::cout << "l1atype : 0x" << l1atype << std::endl ;
      // 	std::cout << std::dec << std::setfill(' ');
	
      // 	for(int ibx=0;ibx<maxnbx;ibx++){
      // 	  for(int istc=0;istc<12;istc++){
      // 	    std::cout<<energy_raw[ibx][istc]<<" ";
      // 	  }
      // 	  std::cout<< std::endl ;
      // 	  for(int istc=0;istc<12;istc++){
      // 	    std::cout<<energy_unpkd[ibx][istc]<<" ";
      // 	  }
      // 	  std::cout<< std::endl << std::endl ;
      // 	}
      // 	std::cout<< std::endl ;
	
      // }

      //check bx
      for(int iect=0;iect<2;iect++){
      	for(int ibx=0;ibx<maxnbx;ibx++){
      	  for(int istc=0;istc<12;istc++){
      	    if( ev.bx_unpkd[iect][ibx][istc] != ev.bx_raw[iect][ibx][istc]){
      	      // std::cout << std::dec << std::setfill(' ');
      	      // std::cerr << " Unpacked location value do not match with the packed one for (Run, event, iecont, bx, stc, bx_unpacked, bx_packed ) : (" << runNumber << "," << ev.eventId <<"," << iect << "," << ibx <<","<< istc <<","<
	      //< (ev.bx_unpkd[iect][ibx][istc] & 0x3)  << "," << ev.bx_raw[iect][ibx][istc] << ") "<< std::endl;
      	      isGood = false;
	      if(iect==0){ 
		nofBxRawUnpkMM++;
		isBxE = true;
	      }else{
		nofBxRawUnpkMM1++;
		isBxE1 = true;
	      }
      	    }  
      	  }
      	}
	//modulo test
	if(ev.bxId==3564){
	  if(ev.bx_raw[iect][ev.daq_nbx[0]][0]!=15 || ev.bx_unpkd[iect][ev.daq_nbx[1]][0]!=15){
	    //std::cerr << "Bx Module test failed for iect : "<< iect <<" since ev.bxId%8 : " << ev.bxId <<" and ev.bx_raw : "<< ev.bx_raw[iect][ev.daq_nbx[0]][0] << " and ev.bx_unpkd : "<< ev.bx_unpkd[iect][ev.daq_nbx[1]][0] << std::endl ;
	    isGood = false;
	    if(iect==0){ 
	      nofBxCentralMM++;
	      isBxCE = true;
	    }else{
	      nofBxCentralMM1++;
	      isBxCE1 = true;
	    }
	  }
	}else{
	  if((ev.bxId%8 != ev.bx_raw[iect][ev.daq_nbx[0]][0]) || (ev.bxId%8 != ev.bx_unpkd[iect][ev.daq_nbx[1]][0])){
	    //std::cerr << "Bx Module test failed for iect : "<< iect <<" since ev.bxId%8 : " << (ev.bxId%8) <<" and ev.bx_raw : "<< ev.bx_raw[iect][ev.daq_nbx[0]][0] << " and ev.bx_unpkd : "<< ev.bx_unpkd[iect][ev.daq_nbx[1]][0] << std::endl ;
	    isGood = false;
	    if(iect==0) {
	      nofBxCentralMM++;
	      isBxCE = true;
	    }else{
	      nofBxCentralMM1++;
	       isBxCE1 = true;
	    }
	  }
	}
      }

      
      //Check Locations
      for(int iect=0;iect<2;iect++){
      	for(int ibx=0;ibx<maxnbx;ibx++){
      	  for(int istc=0;istc<12;istc++){
      	    if((ev.loc_unpkd[iect][ibx][istc]>>2  & 0xF) != unsigned(istc)){
      	      // std::cout << std::dec << std::setfill(' ');
      	      // std::cerr << " Unpacked location index do not match with the STC for (Run, event, iecont, bx, stc, istc_from_unpacked) : (" << runNumber << "," << ev.eventId <<"," << iect << "," << ibx <<","<< istc <<","<< (ev.loc_unpkd[iect][ibx][istc]>>2  & 0xF)  <<") "<< std::endl;
      	      isGood = false;
	      if(iect==0){ 
		nofSTCNumberingErrors++;
		isSTCNE = true;
	      }else{
		nofSTCNumberingErrors1++;
		isSTCNE1 = true;
	      }
      	    }
      	    if( (ev.loc_unpkd[iect][ibx][istc] & 0x3) != ev.loc_raw[iect][ibx][istc]){
      	      // std::cout << std::dec << std::setfill(' ');
      	      // std::cerr << " Unpacked location value do not match with the packed one for (Run, event, iecont, bx, stc, loc_unpacked, loc_packed ) : (" << runNumber << "," << ev.eventId <<"," << iect << "," << ibx <<","<< istc <<","<
	      //< (ev.loc_unpkd[iect][ibx][istc] & 0x3)  << "," << ev.loc_raw[iect][ibx][istc] << ") "<< std::endl;
      	      isGood = false;
	      if(iect==0){ 
		nofSTCLocErrors++;
		isSTCLE = true;
	      }else{
		nofSTCLocErrors1++;
		isSTCLE1 = true;
	      }
      	    }    
      	  }
      	}
      }

      //std::cerr << "After7:Event : "<< ev.eventId << ", l1aType : " << ev.l1aType << ", and prevEvent  "<< prevEvent << ", nEvents : " << nEvents << " differs by "<< Abs64(ev.eventId,prevEvent) <<" (sequence differs by [ "<< ev.sequenceId << " - "<< prevSequence << " ] = " << Abs32(ev.sequenceId, prevSequence) << "), EventID_Diff is more than 2 " << std::endl;

      //Check Energy
      for(int iect=0;iect<2;iect++){
      	for(int ibx=0;ibx<maxnbx;ibx++){
      	  for(int istc=0;istc<12;istc++){
      	    if(energy_raw[iect][ibx][istc]!=energy_unpkd[iect][ibx][istc]){
      	      // std::cout << std::dec << std::setfill(' ');
      	      // std::cerr << " Packed and unpacked energies does not match for (Run, event,iecont,bx.stc,raw_energy,unpacked_energy) : (" << runNumber << "," << ev.eventId <<"," << iect << "," << ibx <<","<< istc <<","<< energy_raw[iec
	      //																										      t][ibx][istc] <<","<< energy_unpkd[iect][ibx][istc] <<") "<< std::endl;
      	      isGood = false;
	      if(iect==0){ 
		nofEnergyMisMatches++;
		isEngE = true;
	      }else{
		nofEnergyMisMatches1++;
		isEngE1 = true;
	      }
      	    }
      	  }
      	}
      }
      
      
      if(nEvents<max_event_entries){
      	econt_events.push_back(ev);
      }else{
      	econt_events.pop_front();
      	econt_events.push_back(ev);
      }
      
      //if(nEvents>16583000)
      // if(nEvents>100)
      // 	break;
      

      if(isSTCNE) nofEvcSTCNE++;
      if(isSTCLE) nofEvcSTCLE++;
      if(isEngE) nofEvcEngE++;
      if(isBxE) nofEvcBxE++;
      if(isBxCE) nofEvcBxCE++;
      if(isSTCNE1) nofEvcSTCNE1++;
      if(isSTCLE1) nofEvcSTCLE1++;
      if(isEngE1) nofEvcEngE1++;
      if(isBxE1) nofEvcBxE1++;
      if(isBxCE1) nofEvcBxCE1++;

    }//loop event  
  }
  
  delete r;
  
  for (int istc=0;istc<12;istc++) {
    for(int ibin=1;ibin<=hloc[istc]->GetNbinsX();ibin++){
      if(hloc[istc]->GetBinContent(ibin)==0.0){
	//std::cerr << "No Trig: Empty TC " << ibin <<" for STC "<< istc <<" for ECONT0."<< std::endl;
	isGood = false;
	nofEmptyTCs++;
      }
    }
  }
  for (int istc=0;istc<12;istc++) {
    for(int ibin=1;ibin<=hloc1[istc]->GetNbinsX();ibin++){
      if(hloc1[istc]->GetBinContent(ibin)==0.0){
	//std::cerr << "No Trig: Empty TC " << ibin <<" for STC "<< istc <<" for ECONT 1."<< std::endl;
	isGood = false;
	nofEmptyTCs1++;
      }
    }
  }

  runNumber = unsigned(runNumber1);

  cout<<endl;
  for(int i=0;i<80;i++) cout<<"=";
  cout<<endl;
  cout << "Summary of Relay " << relayNumber << " and Run " << runNumber << endl;
  cout<<endl;
  for(int i=0;i<80;i++) cout<<"=";
  cout<<endl;

  
  cout <<"Relay| Run| NofEvts| NofPhysT| NofCalT| NofCoinT| NofRandT| NofSoftT| NofRegT| RStrtE| RStpE| EvtIdE| xscafeE| 1stcafeE| daqHE| NbxE| STCNumE| STCLocE| EngE| BxMME| BxCMME| EmptyTCs| STCNumE1| STCLocE1| EngE1| BxMME1| BxCMME1| EmptyTCs1| PV|"<<endl;
  cout << relayNumber << "|"
       << runNumber << "|"
       << nEvents << "|"
       << total_phys_events << "|"
       << total_calib_events << "|"
       << total_coinc_events << "|"
       << total_random_events << "|"
       << total_soft_events << "|"
       << total_regular_events << "|"
       << nofRStartErrors << "|"
       << nofRStopErrors << "|"
       << nofEventIdErrs << "|"
       << nofExcessFECAFEErrors << "|" 
       << nofFirstFECAFEErrors << "|"
       << hDaqEvtMisMatch->GetEntries() << "|"
       << nofNbxMisMatches << "|"
       << nofSTCNumberingErrors << "|"
       << nofSTCLocErrors << "|"
       << nofEnergyMisMatches << "|"
       << nofBxRawUnpkMM << "|"
       << nofBxCentralMM << "|"
       << nofEmptyTCs << "|"
       << nofSTCNumberingErrors1 << "|"
       << nofSTCLocErrors1 << "|"
       << nofEnergyMisMatches1 << "|"
       << nofBxRawUnpkMM1 << "|"
       << nofBxCentralMM1 << "|"
       << nofEmptyTCs1 << "|"
       << std::hex << std::setfill('0')
       << "0x" << std::setw(4) << payload_version << "|"
       << std::dec << std::setfill(' ')
       <<endl;

  cout <<"Relay| Run| NofEvts| NofPhysT| NofCalT| NofCoinT| NofRandT| NofSoftT| NofRegT| RStrtE| RStpE| EvtIdE| xscafeE| 1stcafeE| daqHE| NbxE| STCNumE| STCLocE| EngE| BxMME| BxCMME| EmptyTCs| STCNumE1| STCLocE1| EngE1| BxMME1| BxCMME1| EmptyTCs1| PV|"<<endl;
  cout << relayNumber << "|"
       << runNumber << "|"
       << nEvents << "|"
       << total_phys_events << "|"
       << total_calib_events << "|"
       << total_coinc_events << "|"
       << total_random_events << "|"
       << total_soft_events << "|"
       << total_regular_events << "|"
       << nofRStartErrors << "|"
       << nofRStopErrors << "|"
       << nofEventIdErrs << "|"
       << nofExcessFECAFEErrors << "|" 
       << nofFirstFECAFEErrors << "|"
       << hDaqEvtMisMatch->GetEntries() << "|"
       << nofNbxMisMatches << "|"
       << nofEvcSTCNE << "|"
       << nofEvcSTCLE << "|"
       << nofEvcEngE << "|"
       << nofEvcBxE << "|"
       << nofEvcBxCE << "|"
       << nofEmptyTCs << "|"
       << nofEvcSTCNE1 << "|"
       << nofEvcSTCLE1 << "|"
       << nofEvcEngE1 << "|"
       << nofEvcBxE1 << "|"
       << nofEvcBxCE1 << "|"
       << nofEmptyTCs1 << "|"
       << std::hex << std::setfill('0')
       << "0x" << std::setw(4) << payload_version << "|"
       << std::dec << std::setfill(' ')
       <<endl;
  
  // if(isGood){
  //   std::cout << "All fine, no errors found." << std::endl;
  // }

  cout<<endl;
  for(int i=0;i<80;i++) cout<<"=";
  cout<<endl;
  
  // delete hDaqEvtMisMatch;
  // delete hErrEcont1Status;
  // delete hErrEcont0Status;
  econt_events.clear();
  delete hloc;
  delete hloc1;

  return 0;
}
