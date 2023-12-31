#include <iostream>
#include <bitset>

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TCanvas.h"

#include "TFileHandlerLocal.h"
#include "common/inc/FileReader.h"

#include <deque>

// Author : Indranil Das
// Email : Indranil.Das@cern.ch
// Affiliation : Imperial College, London
//
// Purpose : To reproduce the ECONT results using pass though data


//Command to execute : 1. ./compile.sh
//                     2. ./emul_econt.exe $Relay $rname

using namespace std;

const uint64_t maxEvent = 1e8;

typedef struct{

  uint64_t eventId;
  uint32_t sequenceId;
  uint16_t l1aType;
  uint16_t ECONT_packet_status[2][12];               //2 : LSB/MSB, 12 : STC
  bool ECONT_packet_validity[2][12];                 //2 : LSB/MSB, 12 : STC
  uint16_t OC[2],EC[2];                              //2 : LSB/MSB
  uint16_t BC[2];                                    //2 : LSB/MSB
  uint16_t bxId;                                     //bxId
  
  uint16_t daq_data[5];                              //5 : data blocks separated by 0xfecafe
  uint16_t daq_nbx[5];                               //5 : data blocks separated by 0xfecafe
  uint16_t size_in_cafe[5];                          //5 : data blocks separated by 0xfecafe
  
  uint32_t energy_raw[2][15][12];                    //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t loc_raw[2][15][12];                       //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t bx_raw[2][15][12];                        //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t energy_unpkd[2][15][12];                  //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t loc_unpkd[2][15][12];                     //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t bx_unpkd[2][15][12];                      //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  
} econt_event;

typedef struct{
  uint64_t eventId,event;
  uint16_t chip,half,channel,adc,toa,tot,totflag,bxcounter,eventcounter,orbitcounter;
} rocdata;

typedef struct{

  uint64_t event;
  uint64_t eventId;
  uint32_t sequenceId;
  uint16_t bxId;                                     //bxId

  uint16_t daq_data[5];                              //5 : data blocks separated by 0xfecafe
  uint16_t daq_nbx[5];                               //5 : data blocks separated by 0xfecafe
  uint16_t size_in_cafe[5];                          //5 : data blocks separated by 0xfecafe

  // uint16_t OC[2],EC[2];                              //2 : LSB/MSB
  // uint16_t BC[2];                                    //2 : LSB/MSB  

  uint16_t modsum[2][15];
  uint32_t energy_raw[2][15][12];                    //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t loc_raw[2][15][12];                       //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t bx_raw[2][15];                        //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC

  uint32_t energy_unpkd[2][15][12];                  //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t loc_unpkd[2][15][12];                     //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint32_t bx_unpkd[2][15];                      //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  // int event,chip,channelsumid,rawsum,compressed;
  // float decompresssum;

} econtdata;


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
    std::cout << "EventId : " << std::setw(3) << rEvent->slinkBoe()->eventId() ;
    std::cout << "\t Word " << std::setw(3) << i << " ";
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

uint32_t decode_tc_val(uint32_t value)
{
  uint32_t mant = value & 0x7;
  uint32_t pos  = (value >> 3) & 0xf;

  if(pos==0) return mant << 1 ;

  pos += 2;

  uint32_t decompsum = 1 << pos;
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

void set_packet_tcs_bc(bool packet_tcs[48], uint32_t* packet) {
  //The first 19 bits are in the first word
  // for (int i=0; i<=19; i++) {
  //   int j = 47 - i;
  //   packet_tcs[j] = (pick_bits32(packet[0], 12+i, 1)==1)?true:false;
  // }
  // for (int i=0; i<=27; i++) {
  //   int j = 27 - i;
  //   packet_tcs[j] = (pick_bits32(packet[1], i, 1)==1)?true:false;
  // }
  for (int i=0; i<=19; i++) {
    packet_tcs[i] = (pick_bits32(packet[0], 12+i, 1)==1)?true:false;
  }
  for (int i=0; i<=27; i++) {
    int j = 20 + i;
    packet_tcs[j] = (pick_bits32(packet[1], i, 1)==1)?true:false;
  }
}

// 9 energies, 7 bits long, immediately after the packet energies
void set_packet_energies_bc(uint64_t packet_energies[9], uint32_t* packet) {
  
  uint64_t packet64[4];
  for (int i=0; i<4; i++) {
    packet64[i] = packet[i];
  }
  
  // need one 64 bit words since all of the energies are 9*7 = 63 bits long
  uint64_t word1 = (packet64[1] << (28+32)) + (packet64[2] << 28) + (packet64[3] >> 4);
  
  for (int i=0; i<9; i++) 
    packet_energies[i] = pick_bits64(word1, i*7, 7);
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

bool is_ffsep_word(const uint64_t word, int& isMSB) {

  if( (word & 0xFFFFFFFF) == 0xFFFFFFFF){
    isMSB = 0;
    return true; //0xFFFF 0xFFFF
  }else if((word >> 32) == 0xFFFFFFFF){
    isMSB = 1;
    return true; //0xFECAFE                                                      // 0xFF FFFF masking to be there to compare with 16698110 (could have used the hex of this)
  }else{
    isMSB = -1;
    return false;
  }
}


// returns location in this event of the n'th 0xfecafe... line
int find_ffsep_word(const Hgcal10gLinkReceiver::RecordRunning *rEvent, int& isMSB, int n, int ffsep_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent)+1);

  if (ffsep_word_loc > 0) {
    if (is_ffsep_word(p64[ffsep_word_loc], isMSB)) {
      return ffsep_word_loc;
    }else
      return ffsep_word_loc;
  } 
  else {
    ;//std::cout << "oops" << std::endl;
  }
 
  int cafe_counter = 0;
  int cafe_word_idx = -1;
  for(unsigned i(0);i<rEvent->payloadLength();i++){
    const uint64_t word = p64[i];                                                                                // Is it advisable to convert 64 bit to 32 this way without masking ?
    if (is_ffsep_word(word,isMSB)) { // if word == 0xfeca
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


bool is_empty_word(const uint64_t word, int& isMSB) {

  // std::cout << std::hex << std::setfill('0');
  // std::cout << "0x" << std::setw(16) << word << std::endl;
  // std::cout << std::dec << std::setfill(' ');

  if( (word & 0xFFFFFFFF) == 0x0){
    isMSB = 0;
    return true; //0xFFFF 0xFFFF
  }else if((word >> 32) == 0x0){
    isMSB = 1;
    return true; //0xFECAFE                                                      // 0xFF FFFF masking to be there to compare with 16698110 (could have used the hex of this)
  }else{
    isMSB = -1;
    return false;
  }
}

bool found_empty_word(const Hgcal10gLinkReceiver::RecordRunning *rEvent, int& isMSB, int ffsep_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent));

  if (ffsep_word_loc > 0) {
    if (is_empty_word(p64[ffsep_word_loc], isMSB)) {
      return true;
    }else
      return false;
  } 
  else {
    ;//std::cout << "oops" << std::endl;
    return false;
  }
 
}

int read_roc_data(vector<rocdata>& rocarray, unsigned relayNumber, unsigned runNumber, unsigned linkNumber)
{

  //Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;
  
  //Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);
  
  //Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  _fileReader.setDirectory(std::string("dat/Relay")+std::to_string(relayNumber));
  _fileReader.openRun(runNumber,linkNumber);

  rocdata rdata;
  uint64_t nEvents = 0;
  
  //Use the fileReader to read the records
  while(_fileReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	// rStart->print();
  	// std::cout << std::endl;
  	continue;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
  	// rStop->print();
  	// std::cout << std::endl;
  	continue;
      }
    }
    //Else we have an event record 
    else{

      const Hgcal10gLinkReceiver::SlinkBoe *boe = rEvent->slinkBoe();      
      const Hgcal10gLinkReceiver::SlinkEoe *eoe = rEvent->slinkEoe();

      if (nEvents < 1){ 
       	event_dump(rEvent);
      	rEvent->RecordHeader::print();
      	boe->print();
      	eoe->print();
      }
      
      //Increment event counter
      nEvents++;

      int isMSB[6];
      int ffsep_word_loc[6];
      bool is_ff_sep_notfound = false;
      for(int iloc=0;iloc<6;iloc++){
	isMSB[iloc] = -1;
	ffsep_word_loc[iloc] = find_ffsep_word(rEvent, isMSB[iloc], iloc+1);
	if(isMSB[iloc] == -1) is_ff_sep_notfound = true;
	// if(iloc==0)
	//   cout << "\nEvent : " << nEvents << ": " <<iloc+1 <<"-st 0xffff 0xffff separator word at location : " << ffsep_word_loc[iloc] << " word and isMSB : " << isMSB[iloc] << endl;
	// else if(iloc==1)
	//   cout << "Event : " << nEvents << ": " <<iloc+1 <<"-nd 0xffff 0xffff separator word at location : " << ffsep_word_loc[iloc] << " word and isMSB : " << isMSB[iloc] << endl;
	// else if(iloc==2)
	//   cout << "Event : " << nEvents << ": " <<iloc+1 <<"-rd 0xffff 0xffff separator word at location : " << ffsep_word_loc[iloc] << " word and isMSB : " << isMSB[iloc] << endl;
	// else
	//   cout << "Event : " << nEvents << ": " <<iloc+1 <<"-th 0xffff 0xffff separator word at location : " << ffsep_word_loc[iloc] << " word and isMSB : " << isMSB[iloc] << endl;
      }
      
      int expctd_empty_word_loc = ffsep_word_loc[5]+20;
      int empty_isMSB = -1;
      bool hasfound_emptry_word = found_empty_word(rEvent, empty_isMSB, expctd_empty_word_loc) ;
      // cout << "Event : " << nEvents << ": 0x0000 0x0000 closing separator word at location : " << expctd_empty_word_loc << " hasfound_emptry_word : "<< hasfound_emptry_word << ",  and isMSB : " << empty_isMSB << endl;

      if(!hasfound_emptry_word) continue;
      if(is_ff_sep_notfound) continue;
      //if(nEvents>=500000) continue;
      if(nEvents>maxEvent) continue;
      int ichip = 0;
      const uint64_t *p64(((const uint64_t*)rEvent)+1);
      for(int iloc=0;iloc<6;iloc++){
	unsigned max_sep_word = (iloc==5) ? expctd_empty_word_loc : ffsep_word_loc[iloc+1] ;
	bool ishalf = (iloc%2==0) ? false : true ;
	if(isMSB[iloc]==1 and iloc!=5) max_sep_word++;
	int index = 0;
	int ch = 0;
	for(unsigned i = ffsep_word_loc[iloc]+1 ; i< max_sep_word ; i++){
	  // std::cout << "chip "<<ichip<<", half : "<<ishalf<<", index  : "<< index << " word : " <<i <<" \t";
	  // std::cout << std::hex << std::setfill('0');
	  // //cout<<" : 0x" << std::setw(16) << p64[i] << std::endl;
	  // std::cout << std::dec << std::setfill(' ');
	  
	  uint32_t wordL = 0;
	  uint32_t wordM = 0;
	  if(isMSB[iloc]==0){
	    wordL = (p64[i] & 0xFFFFFFFF) ;
	    wordM = ((p64[i] >> 32) & 0xFFFFFFFF) ;
	  }else if(isMSB[iloc]==1){
	    wordL = (p64[i-1] & 0xFFFFFFFF) ;	  
	    wordM = ((p64[i] >> 32) & 0xFFFFFFFF) ;
	  }
	  const uint16_t trigflagL = (wordL>>30) & 0x3;
	  const uint16_t adcL = (wordL>>20) & 0x3FF;
	  const uint16_t totL = (wordL>>10) & 0x3FF;
	  const uint16_t toaL = wordL & 0x3FF;
	  
	  const uint16_t trigflagM = (wordM>>30) & 0x3;
	  const uint16_t adcM = (wordM>>20) & 0x3FF;
	  const uint16_t totM = (wordM>>10) & 0x3FF;
	  const uint16_t toaM = wordM & 0x3FF;
	  
	  // std::cout << std::hex << std::setfill('0');
	  // if(isMSB[iloc]==0)
	  //   cout<<"WordM : 0x" << std::setw(8) << wordM <<", wordL : 0x" << std::setw(8) << wordL<< std::endl;
	  // else
	  //   cout<<"WordL : 0x" << std::setw(8) << wordL<<", wordM : 0x" << std::setw(8) << wordM << std::endl;
	  // std::cout << std::dec << std::setfill(' ');
	  // cout<<"L:(flag,adc,tot,toa) : (" << trigflagL <<", " << adcL << ", " << totL <<", " << toaL << "), \t"
	  //     <<"M:(flag,adc,tot,toa) : (" << trigflagM <<", " << adcM << ", " << totM <<", " << toaM << ")" << endl;
	  if(isMSB[iloc]==0){
	    rdata.event = nEvents;
	    rdata.eventId = boe->eventId();
	    rdata.chip = uint16_t(ichip);
	    rdata.half = uint16_t(ishalf);
	    rdata.channel = ch ;
	    rdata.adc = adcM;
	    rdata.tot = totM;
	    rdata.toa = toaM;
	    rdata.totflag = trigflagM;
	    rdata.bxcounter = eoe->bxId();
	    rdata.eventcounter = rEvent->RecordHeader::sequenceCounter();
	    rdata.orbitcounter = eoe->orbitId();
	    ch++;
	    rocarray.push_back(rdata);

	    rdata.event = nEvents;
	    rdata.eventId = boe->eventId();
	    rdata.chip = uint16_t(ichip);
	    rdata.half = uint16_t(ishalf);
	    rdata.channel = ch ;
	    rdata.adc = adcL;
	    rdata.tot = totL;
	    rdata.toa = toaL;
	    rdata.totflag = trigflagL;
	    rdata.bxcounter = eoe->bxId();
	    rdata.eventcounter = rEvent->RecordHeader::sequenceCounter();
	    rdata.orbitcounter = eoe->orbitId();
	    ch++;
	    rocarray.push_back(rdata);
	  }else{
	    
	    rdata.event = nEvents;
	    rdata.eventId = boe->eventId();
	    rdata.chip = uint16_t(ichip);
	    rdata.half = uint16_t(ishalf);
	    rdata.channel = ch ;
	    rdata.adc = adcL;
	    rdata.tot = totL;
	    rdata.toa = toaL;
	    rdata.totflag = trigflagL;
	    rdata.bxcounter = eoe->bxId();
	    rdata.eventcounter = rEvent->RecordHeader::sequenceCounter();
	    rdata.orbitcounter = eoe->orbitId();
	    ch++;
	    rocarray.push_back(rdata);

	    rdata.event = nEvents;
	    rdata.eventId = boe->eventId();
	    rdata.chip = uint16_t(ichip);
	    rdata.half = uint16_t(ishalf);
	    rdata.channel = ch ;
	    rdata.adc = adcM;
	    rdata.tot = totM;
	    rdata.toa = toaM;
	    rdata.totflag = trigflagM;
	    rdata.bxcounter = eoe->bxId();
	    rdata.eventcounter = rEvent->RecordHeader::sequenceCounter();
	    rdata.orbitcounter = eoe->orbitId();
	    ch++;
	    rocarray.push_back(rdata);
	  }
	  index++;
	}
	//cout<<endl;
	if(iloc%2==1)ichip++;
      }
    }
  }//file reader

  return true;
}

int read_econt_data_stc4(vector<econtdata>& econtarray, unsigned relayNumber, unsigned runNumber)
{

  unsigned linkNumber(0);
  
  //Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;
  
  //Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);
  
  //Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  _fileReader.setDirectory(std::string("dat/Relay")+std::to_string(relayNumber));
  _fileReader.openRun(runNumber,linkNumber);
  
  econtdata edata;
  uint64_t nEvents = 0;
  uint64_t prevEvent = 0;
  uint32_t prevSequence = 0;
  uint32_t packet[4];
  uint32_t packet_counter;
  uint32_t packet_locations[12];
  uint64_t packet_energies[12];
  
  //Use the fileReader to read the records
  while(_fileReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	// rStart->print();
  	// std::cout << std::endl;
  	continue;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
  	// rStop->print();
  	// std::cout << std::endl;
  	continue;
      }
    }
    //Else we have an event record 
    else{

      const Hgcal10gLinkReceiver::SlinkBoe *boe = rEvent->slinkBoe();      
      const Hgcal10gLinkReceiver::SlinkEoe *eoe = rEvent->slinkEoe();

      // if (nEvents < 1){ 
      //  	event_dump(rEvent);
      // 	rEvent->RecordHeader::print();
      // 	boe->print();
      // 	eoe->print();
      // }
      
      //Increment event counter
      nEvents++;
      
      if(boe->boeHeader()!=boe->BoePattern) continue;
      if(eoe->eoeHeader()!=eoe->EoePattern) continue;
      uint16_t l1atype = boe->l1aType();      
      if(l1atype==0) continue;
      
      const uint64_t *p64(((const uint64_t*)rEvent)+1);


      edata.event = nEvents;
      edata.eventId = boe->eventId();
      edata.bxId = eoe->bxId();
      edata.sequenceId = rEvent->RecordHeader::sequenceCounter(); 

      if((Abs64(edata.eventId,prevEvent) != Abs32(edata.sequenceId, prevSequence)) and Abs64(edata.eventId,prevEvent)>=2){
	prevEvent = boe->eventId();
	prevSequence = rEvent->RecordHeader::sequenceCounter(); 
	continue;
      }
      prevEvent = edata.eventId;
      prevSequence = edata.sequenceId;

      edata.daq_data[0] = p64[2] & 0xF;
      edata.daq_nbx[0] = p64[2]>>4 & 0x7;
      uint64_t daq0_event_size = (2*edata.daq_nbx[0] + 1)*edata.daq_data[0];
      int first_cafe_word_loc = find_cafe_word(rEvent, 1);
      edata.size_in_cafe[0] = p64[first_cafe_word_loc] & 0xFF;      
      
      edata.daq_data[1] = p64[2]>>7 & 0xF;
      edata.daq_nbx[1] = p64[2]>>11 & 0x7;
      uint64_t daq1_event_size = (2*edata.daq_nbx[1] + 1)*edata.daq_data[1];
      int second_cafe_word_loc = find_cafe_word(rEvent, 2);
      edata.size_in_cafe[1] = p64[second_cafe_word_loc] & 0xFF;
      
      int sixth_cafe_word_loc = find_cafe_word(rEvent, 6);
      
      if(sixth_cafe_word_loc!=0) continue;
      if(first_cafe_word_loc != 3) continue;
      if(daq0_event_size != edata.size_in_cafe[0]) continue;	
      if(daq1_event_size != edata.size_in_cafe[1]) continue;	
      
      if(nEvents>maxEvent) continue;
      
      int bx_index = -1.0*int(edata.daq_nbx[0]);
      const int maxnbx = (2*edata.daq_nbx[0] + 1);
      uint64_t energy_raw[2][maxnbx][12];
      for(int iect=0;iect<2;iect++)
  	for(int ibx=0;ibx<maxnbx;ibx++){
	  edata.bx_raw[iect][ibx] = 0;
  	  for(int istc=0;istc<12;istc++){
  	    energy_raw[iect][ibx][istc] = 0;
  	    edata.energy_raw[iect][ibx][istc] = 0;
  	    edata.loc_raw[iect][ibx][istc] = 0;
  	  }
	}
      for(unsigned i(first_cafe_word_loc+1);i<daq0_event_size+first_cafe_word_loc+1;i=i+4){
	
      	const uint32_t word = (p64[i] & 0xFFFFFFFF) ;
      	const uint32_t bx_counter = ( word >> 28 ) & 0xF;
	
      	packet[0] = (p64[i] & 0xFFFFFFFF) ;
  	packet[1] = (p64[i+1] & 0xFFFFFFFF) ;
  	packet[2] = (p64[i+2] & 0xFFFFFFFF) ; 
  	packet[3] = (p64[i+3] & 0xFFFFFFFF) ;
	
      	set_packet_locations(packet_locations, packet);
      	set_packet_energies(packet_energies, packet);
	
	uint64_t totEnergy = 0;
	for(int istc=0;istc<12;istc++) totEnergy += packet_energies[istc] ;
	
      	// if (nEvents < 4) {
	//   std::cout<<" EventId : "<<boe->eventId()
	// 	   << ", bx_index : " << (bx_index+int(edata.daq_nbx[0])) 
	// 	   <<", bx(LSB) :"  << bx_counter 
	// 	   <<", edata.bxId : "<<edata.bxId <<", modulo8 : "<< (edata.bxId%8)
	// 	   <<", totEnergy : "<<totEnergy
	// 	   <<std::endl;
  	//   std::cout<<"E(LSB) : \t";
      	//   for(int istc=0;istc<12;istc++){
      	//     std::cout<<packet_energies[istc]<<" ";
      	//   }
      	//   std::cout<<std::endl;
	//   std::cout<<"L(LSB) : \t";
      	//   for(int istc=0;istc<12;istc++){
	//     std::cout<<packet_locations[istc]<<" ";
      	//   }
      	//   std::cout<<std::endl;
      	// }
	
	edata.modsum[0][(bx_index+int(edata.daq_nbx[0]))] = totEnergy;
	edata.bx_raw[0][(bx_index+int(edata.daq_nbx[0]))] = bx_counter;
      	for(int istc=0;istc<12;istc++){
      	  energy_raw[0][(bx_index+int(edata.daq_nbx[0]))][istc] = packet_energies[istc];
  	  edata.energy_raw[0][(bx_index+int(edata.daq_nbx[0]))][istc] = packet_energies[istc];
  	  edata.loc_raw[0][(bx_index+int(edata.daq_nbx[0]))][istc] = packet_locations[istc];
      	}
	
  	const uint32_t word1 = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	const uint32_t bx_counter1 = ( word1 >> 28 ) & 0xF;
	
      	packet[0] = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	packet[1] = ((p64[i+1] >> 32) & 0xFFFFFFFF) ;
      	packet[2] = ((p64[i+2] >> 32) & 0xFFFFFFFF) ;
      	packet[3] = ((p64[i+3] >> 32) & 0xFFFFFFFF) ;
	
      	set_packet_locations(packet_locations, packet);
      	set_packet_energies(packet_energies, packet);

	uint64_t totEnergy1 = 0;
	for(int istc=0;istc<12;istc++) totEnergy1 += packet_energies[istc] ;

      	if (nEvents < 4) {
	  std::cout<<" EventId : "<<boe->eventId()
		   << ", bx_index : " << (bx_index+int(edata.daq_nbx[0])) 
		   <<", bx(MSB) :"  << bx_counter 
		   <<", edata.bxId : "<<edata.bxId <<", modulo8 : "<< (edata.bxId%8)
		   <<", totEnergy : "<<totEnergy1
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
	
	edata.modsum[1][(bx_index+int(edata.daq_nbx[0]))] = totEnergy1;
	edata.bx_raw[1][(bx_index+int(edata.daq_nbx[0]))] = bx_counter1;
      	for(int istc=0;istc<12;istc++){
      	  energy_raw[1][(bx_index+int(edata.daq_nbx[0]))][istc] = packet_energies[istc];
  	  edata.energy_raw[1][(bx_index+int(edata.daq_nbx[0]))][istc] = packet_energies[istc];
  	  edata.loc_raw[1][(bx_index+int(edata.daq_nbx[0]))][istc] = packet_locations[istc];
      	}
	
      	bx_index++;
      }//loop over unpacked

      econtarray.push_back(edata);
      if (nEvents < 4) std::cout<<std::endl;
      
    }
  }//file reader
  
  return true;
}

int read_econt_data_bc(vector<econtdata>& econtarray, unsigned relayNumber, unsigned runNumber)
{

  unsigned linkNumber(0);
  
  //Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;
  
  //Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);
  
  //Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  _fileReader.setDirectory(std::string("dat/Relay")+std::to_string(relayNumber));
  _fileReader.openRun(runNumber,linkNumber);
  
  econtdata edata;
  uint64_t nEvents = 0;
  uint64_t prevEvent = 0;
  uint32_t prevSequence = 0;
  uint32_t packet[4];
  uint32_t packet_counter;
  bool packet_tcs[48];
  uint64_t packet_energies[9];
  
  //Use the fileReader to read the records
  while(_fileReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	// rStart->print();
  	// std::cout << std::endl;
  	continue;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
  	// rStop->print();
  	// std::cout << std::endl;
  	continue;
      }
    }
    //Else we have an event record 
    else{

      const Hgcal10gLinkReceiver::SlinkBoe *boe = rEvent->slinkBoe();      
      const Hgcal10gLinkReceiver::SlinkEoe *eoe = rEvent->slinkEoe();

      // if (nEvents < 1){ 
      //  	event_dump(rEvent);
      // 	rEvent->RecordHeader::print();
      // 	boe->print();
      // 	eoe->print();
      // }
      
      //Increment event counter
      nEvents++;
      
      if(boe->boeHeader()!=boe->BoePattern) continue;
      if(eoe->eoeHeader()!=eoe->EoePattern) continue;
      uint16_t l1atype = boe->l1aType();      
      if(l1atype==0) continue;
      
      const uint64_t *p64(((const uint64_t*)rEvent)+1);

      edata.event = nEvents;
      edata.eventId = boe->eventId();
      edata.bxId = eoe->bxId();
      edata.sequenceId = rEvent->RecordHeader::sequenceCounter(); 

      if((Abs64(edata.eventId,prevEvent) != Abs32(edata.sequenceId, prevSequence)) and Abs64(edata.eventId,prevEvent)>=2){
	prevEvent = boe->eventId();
	prevSequence = rEvent->RecordHeader::sequenceCounter(); 
	continue;
      }
      prevEvent = edata.eventId;
      prevSequence = edata.sequenceId;

      edata.daq_data[0] = p64[2] & 0xF;
      edata.daq_nbx[0] = p64[2]>>4 & 0x7;
      uint64_t daq0_event_size = (2*edata.daq_nbx[0] + 1)*edata.daq_data[0];
      int first_cafe_word_loc = find_cafe_word(rEvent, 1);
      edata.size_in_cafe[0] = p64[first_cafe_word_loc] & 0xFF;      
      
      edata.daq_data[1] = p64[2]>>7 & 0xF;
      edata.daq_nbx[1] = p64[2]>>11 & 0x7;
      uint64_t daq1_event_size = (2*edata.daq_nbx[1] + 1)*edata.daq_data[1];
      int second_cafe_word_loc = find_cafe_word(rEvent, 2);
      edata.size_in_cafe[1] = p64[second_cafe_word_loc] & 0xFF;

      edata.daq_data[2] = p64[2]>>14 & 0xF;
      edata.daq_nbx[2] = p64[2]>>18 & 0x7;
      uint64_t daq2_event_size = (2*edata.daq_nbx[2] + 1)*edata.daq_data[2];
      int third_cafe_word_loc = find_cafe_word(rEvent, 3);
      edata.size_in_cafe[2] = p64[third_cafe_word_loc] & 0xFF;

      int sixth_cafe_word_loc = find_cafe_word(rEvent, 6);
      
      if(sixth_cafe_word_loc!=0) continue;
      if(first_cafe_word_loc != 3) continue;
      if(daq0_event_size != edata.size_in_cafe[0]) continue;	
      if(daq1_event_size != edata.size_in_cafe[1]) continue;
      if(daq2_event_size != edata.size_in_cafe[2]) continue;	

      if (nEvents < 2) {
	event_dump(rEvent);
	rEvent->RecordHeader::print();
      	boe->print();
      	eoe->print();
      }
      if(nEvents>maxEvent) continue;

      int bx_index = -1.0*int(edata.daq_nbx[0]);
      const int maxnbx = (2*edata.daq_nbx[0] + 1);
      uint64_t energy_raw[2][maxnbx][12];
      for(int iect=0;iect<2;iect++)
  	for(int ibx=0;ibx<maxnbx;ibx++){
	  edata.bx_raw[iect][ibx] = 0;
  	  for(int istc=0;istc<12;istc++){
  	    energy_raw[iect][ibx][istc] = 0;
  	    edata.energy_raw[iect][ibx][istc] = 0;
  	    edata.loc_raw[iect][ibx][istc] = 0;
  	  }
	}
      
      for(unsigned i(first_cafe_word_loc+1);i<daq0_event_size+first_cafe_word_loc+1;i=i+4){
	
      	const uint32_t word = (p64[i] & 0xFFFFFFFF) ;
      	const uint32_t bx_counter = ( word >> 28 ) & 0xF;
	const uint16_t modsum = ( word >> 20 ) & 0xFF;
	
      	packet[0] = (p64[i] & 0xFFFFFFFF) ;
  	packet[1] = (p64[i+1] & 0xFFFFFFFF) ;
  	packet[2] = (p64[i+2] & 0xFFFFFFFF) ; 
  	packet[3] = (p64[i+3] & 0xFFFFFFFF) ;
	
      	set_packet_tcs_bc(packet_tcs, packet);
      	set_packet_energies_bc(packet_energies, packet);

	if (nEvents < 10) {
	  std::cout<<" EventId : "<<boe->eventId()
		   <<", bx(LSB) :"  << bx_counter 
		   <<", edata.bxId : "<<edata.bxId <<", modulo8 : "<< (edata.bxId%8)
		   <<", modsum : "<<modsum
		   <<std::endl;
  	  std::cout<<"E(LSB) : \t";
      	  for(int itc=0;itc<9;itc++){
      	    std::cout<<packet_energies[itc]<<" ";
      	  }
      	  std::cout<<std::endl;
	  std::cout<<"L(LSB) : \t";
      	  for(int itc=0;itc<48;itc++){
	    if(packet_tcs[itc])
	      std::cout<<itc<<" ";
      	  }
      	  std::cout<<std::endl;
      	}
	
	edata.modsum[0][(bx_index+int(edata.daq_nbx[0]))] = modsum;
	edata.bx_raw[0][(bx_index+int(edata.daq_nbx[0]))] = bx_counter;
      	for(int itc=0;itc<9;itc++){	  
      	  energy_raw[0][(bx_index+int(edata.daq_nbx[0]))][itc] = packet_energies[itc];
  	  edata.energy_raw[0][(bx_index+int(edata.daq_nbx[0]))][itc] = packet_energies[itc];
  	  edata.loc_raw[0][(bx_index+int(edata.daq_nbx[0]))][itc] = packet_tcs[itc];
      	}
	int itrigcell = 0;
	for(int itc=0;itc<48;itc++)
	  if(packet_tcs[itc]) edata.loc_raw[0][(bx_index+int(edata.daq_nbx[0]))][itrigcell++] = itc;

  	const uint32_t word1 = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	const uint32_t bx_counter1 = ( word1 >> 28 ) & 0xF;
	const uint32_t modsum1 = ( word1 >> 20 ) & 0xFF;
	
      	packet[0] = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	packet[1] = ((p64[i+1] >> 32) & 0xFFFFFFFF) ;
      	packet[2] = ((p64[i+2] >> 32) & 0xFFFFFFFF) ;
      	packet[3] = ((p64[i+3] >> 32) & 0xFFFFFFFF) ;

      	set_packet_tcs_bc(packet_tcs, packet);
      	set_packet_energies_bc(packet_energies, packet);

      	if (nEvents < 10) {
	  std::cout<<" EventId : "<<boe->eventId()
		   <<", bx(MSB) :"  << bx_counter1 
		   <<", edata.bxId : "<<edata.bxId <<", modulo8 : "<< (edata.bxId%8)
		   <<", modsum : "<<modsum1
		   <<std::endl;
  	  std::cout<<"E(MSB) : \t";
      	  for(int itc=0;itc<9;itc++){
      	    std::cout<<packet_energies[itc]<<" ";
      	  }
      	  std::cout<<std::endl;
	  std::cout<<"L(MSB) : \t";
      	  for(int itc=0;itc<48;itc++){
	    if(packet_tcs[itc])
	      std::cout<<itc<<" ";
      	  }
      	  std::cout<<std::endl;
      	}

	edata.modsum[1][(bx_index+int(edata.daq_nbx[0]))] = modsum1;
	edata.bx_raw[1][(bx_index+int(edata.daq_nbx[0]))] = bx_counter1;
      	for(int itc=0;itc<9;itc++){	  	  
      	  energy_raw[1][(bx_index+int(edata.daq_nbx[0]))][itc] = packet_energies[itc];
  	  edata.energy_raw[1][(bx_index+int(edata.daq_nbx[0]))][itc] = packet_energies[itc];
  	  edata.loc_raw[1][(bx_index+int(edata.daq_nbx[0]))][itc] = packet_tcs[itc];
      	}
	itrigcell = 0;
	for(int itc=0;itc<48;itc++)
	  if(packet_tcs[itc]) edata.loc_raw[1][(bx_index+int(edata.daq_nbx[0]))][itrigcell++] = itc;
	
      	bx_index++;
      }

      // std::cout << std::endl;
      uint32_t energy_unpkd[2][maxnbx][12];
      for(int iect=0;iect<2;iect++)
  	for(int ibx=0;ibx<maxnbx;ibx++){
	  edata.bx_unpkd[iect][ibx] = 0;	    
  	  for(int istc=0;istc<12;istc++){
  	    energy_unpkd[iect][ibx][istc] = 0;
  	    edata.energy_unpkd[iect][ibx][istc] = 0;
  	    edata.loc_unpkd[iect][ibx][istc] = 0;
  	  }
	}
      
      int index_ibx = 0;
      int index_tc = 0;
      for(unsigned i(second_cafe_word_loc+1);i<daq1_event_size+second_cafe_word_loc+1;i++){
	
      	const uint32_t word = (p64[i] & 0xFFFFFFFF) ;
  	const uint32_t word1 = ((p64[i] >> 32) & 0xFFFFFFFF) ;
	
	if((word >> 8) == 0xaccdea){
	  index_ibx++ ;
	  index_tc = 0;
	  continue;
	}else{
	  // std::cout << std::hex << std::setfill('0');
	  // cout<<"Word : 0x" << std::setw(8) << word <<", Word1 : 0x" << std::setw(8) << word1 << std::endl;
	  // std::cout << std::dec << std::setfill(' ');

	  const uint32_t energy0 = word & 0x7F ;
	  const uint32_t loc0 = (word >> 7) & 0x3F ; 
	  const uint32_t energy1 = (word >> 13 ) & 0x7F ;
	  const uint32_t loc1 = (word >> 20) & 0x3F ;
	  const uint32_t bx0 = (word >> 26) & 0xF ;

	  // if (nEvents < 2) {
	  //   cout<<"EventId : "<< boe->eventId()<<", index_ibx : "<< index_ibx <<", bx0 :  "<< bx0
	  // 	<<", loc1 : "<<loc1<<", energy1 : "<<energy1
	  // 	<<", loc0 : "<<loc0<<", energy0 : "<<energy0
	  // 	<< std::endl;	  
	  // }
	  
	  const uint32_t energy2 = word1 & 0x7F ;
	  const uint32_t loc2 = (word1 >> 7) & 0x3F ; 
	  const uint32_t energy3 = (word1 >> 13 ) & 0x7F ;
	  const uint32_t loc3 = (word1 >> 20) & 0x3F ;
	  const uint32_t bx1 = (word1 >> 26) & 0xF ;
	  const uint32_t bit31 = (word1 >> 30) & 0x1 ;
	  const uint32_t bit32 = (word1 >> 31) & 0x1 ;
	  
	  if (nEvents < 10) {
	    cout<<"EventId : "<< boe->eventId()<<", index_ibx : "<< index_ibx << ", bx1 : " << bx1
		<<", b31 : "<<bit31<<", b32 : "<<bit32
		<<", loc3 : "<<loc3<<", energy3 : "<<energy3
		<<", loc2 : "<<loc2<<", energy2 : "<<energy2
		<< std::endl;
	  }
	  
	  energy_unpkd[0][index_ibx-1][index_tc] = energy0 ;
	  edata.energy_unpkd[0][index_ibx-1][index_tc] = energy0 ;
	  edata.loc_unpkd[0][index_ibx-1][index_tc] = loc0;
	  edata.bx_unpkd[0][index_ibx-1] = bx0;
	  if((index_tc+1)<=8){
	    energy_unpkd[0][index_ibx-1][index_tc+1] = energy1 ;
	    edata.energy_unpkd[0][index_ibx-1][index_tc+1] = energy1 ;
	    edata.loc_unpkd[0][index_ibx-1][index_tc+1] = loc1;
	  }
	  
	  energy_unpkd[1][index_ibx-1][index_tc] = energy2 ;
	  edata.energy_unpkd[1][index_ibx-1][index_tc] = energy2 ;
	  edata.loc_unpkd[1][index_ibx-1][index_tc] = loc2;
	  edata.bx_unpkd[1][index_ibx-1] = bx1;
	  if((index_tc+1)<=8){
	    energy_unpkd[1][index_ibx-1][index_tc+1] = energy3 ;
	    edata.energy_unpkd[1][index_ibx-1][index_tc+1] = energy3 ;
	    edata.loc_unpkd[1][index_ibx-1][index_tc+1] = loc3;
	  }
	  
	  index_tc += 2;	  
	}//if valid word
	  
      }//loop over unpacked

      econtarray.push_back(edata);
      
    }
  }//file reader

  return true;
}



//int main(int argc, char** argv){
int read_pedestal(unsigned relayNumber=1695716961, unsigned runNumber=1695716961){
  
  // if(argc < 3){
  //   std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
  //   return 1;
  // }
  // // ./emul_econt.exe $Relay $rname
  // unsigned relayNumber(0);
  // unsigned runNumber(0);
  // std::istringstream issRelay(argv[1]);
  // issRelay >> relayNumber;
  // std::istringstream issRun(argv[2]);
  // issRun >> runNumber;

  //Assign relay and run numbers
  unsigned linkNumber(0);
  bool skipMSB(true);
  
  vector<econtdata> econtarray;
  //read_econt_data_stc4(econtarray,relayNumber,runNumber);
  //read_econt_data_bc(econtarray,relayNumber,runNumber);
  cout<<"Link0 size : " << econtarray.size() <<endl;
  
  vector<rocdata> rocarray1;
  read_roc_data(rocarray1,relayNumber,runNumber,1);
  cout<<"Link1 size : " << rocarray1.size() <<endl;

  vector<rocdata> rocarray2;
  read_roc_data(rocarray2,relayNumber,runNumber,2);
  cout<<"Link1 size : " << rocarray2.size() <<endl;


  // for(const auto& roc : rocarray1){
  //   if(roc.eventId==2)
  //     printf("\troc Event : : %05d, Chip : %02d, Half : %02d, Channel : %02d, ADC : %5d, TOA : %5d, TOT : %5d, TOTflag : %2d, BXCounter : %d, EventCounter : %d, OrbitCounter : %d\n",
  // 	     roc.event, roc.chip, roc.half, roc.channel, roc.adc, roc.toa, roc.tot, roc.totflag, roc.bxcounter, roc.eventcounter, roc.orbitcounter);
  //   if(roc.totflag==3)
  //     hTOTFlag->Fill(roc.channel);
  // }
  
  for(const auto& econt : econtarray){
    if(econt.event<2){
      printf("\tecont Event : : %05ldd, bxId : %u, bxId_8_modulo : %u, Energy_STC[0][7][0] : %07d, Energy_STC[1][7][0] : %07d, Loc[0][7][0] : %2d\n",
	     econt.event, econt.bxId, econt.bxId%8, econt.energy_raw[0][7][0], econt.energy_raw[1][7][0], econt.loc_raw[0][7][0]);
    }
  }
  
  for(const auto& roc : rocarray2){
    if(roc.eventId<2)
      printf("\troc Event : : %05ldd, Chip : %02d, Half : %02d, Channel : %02d, ADC : %5d, TOA : %5d, TOT : %5d, TOTflag : %2d, BXCounter : %d, EventCounter : %d, OrbitCounter : %d\n",
  	     roc.event, roc.chip, roc.half, roc.channel, roc.adc, roc.toa, roc.tot, roc.totflag, roc.bxcounter, roc.eventcounter, roc.orbitcounter);

  }
  
  
  TH1F *hADCPed = new TH1F("hADCPed","hADCPed",300,0,300);
  TH1F *hTOTPed = new TH1F("hTOTPed","hTOTPed",300,0,300);

  TH1F *hADCPed_Ref = new TH1F("hADCPed_Ref","hADCPed_Ref",300,0,300);
  TH1F *hTOTPed_Ref = new TH1F("hTOTPed_Ref","hTOTPed_Ref",300,0,300);
  TH1F *hADCPed_Ref_high = new TH1F("hADCPed_Ref_high","hADCPed_Ref_high",300,0,300);
  TH1F *hTOTPed_Ref_high = new TH1F("hTOTPed_Ref_high","hTOTPed_Ref_high",300,0,300);
  TH1F *hADCPed_Ref_low = new TH1F("hADCPed_Ref_low","hADCPed_Ref_low",300,0,300);
  TH1F *hTOTPed_Ref_low = new TH1F("hTOTPed_Ref_low","hTOTPed_Ref_low",300,0,300);

  int ped_adc_lsb[3][2][38], noise_adc_lsb[3][2][38];
  int ped_tot_lsb[3][2][38], noise_tot_lsb[3][2][38];
  ofstream fout_lsb("log/ped_link1.txt");
  for(int ichip=0;ichip<3;ichip++)
    for(int ihalf=0;ihalf<2;ihalf++)
      for(int ichannel=0;ichannel<38;ichannel++) {
	ped_adc_lsb[ichip][ihalf][ichannel] = 0;
	noise_adc_lsb[ichip][ihalf][ichannel] = 0;
	ped_tot_lsb[ichip][ihalf][ichannel] = 0;
	noise_tot_lsb[ichip][ihalf][ichannel] = 0;
	hADCPed->Reset("ICESM");
	hTOTPed->Reset("ICESM");
	for(const auto& roc : rocarray1){
	  if(roc.chip==ichip and roc.half==ihalf and roc.channel==ichannel){
	    hADCPed->Fill(roc.adc);
	    hTOTPed->Fill(roc.tot);
	    if(roc.chip==0 and roc.half==0 and roc.channel==0) { hADCPed_Ref->Fill(roc.adc); hTOTPed_Ref->Fill(roc.tot); }
	    if(roc.chip==0 and roc.half==0 and roc.channel==10) { hADCPed_Ref_high->Fill(roc.adc); hTOTPed_Ref_high->Fill(roc.tot); }
	    if(roc.chip==0 and roc.half==1 and roc.channel==11) { hADCPed_Ref_low->Fill(roc.adc); hTOTPed_Ref_low->Fill(roc.tot); }
	  }
	}
	ped_adc_lsb[ichip][ihalf][ichannel] = TMath::Nint(hADCPed->GetMean());
	noise_adc_lsb[ichip][ihalf][ichannel] = TMath::Nint(hADCPed->GetRMS());
	ped_tot_lsb[ichip][ihalf][ichannel] = TMath::Nint(hTOTPed->GetMean());
	noise_tot_lsb[ichip][ihalf][ichannel] = TMath::Nint(hTOTPed->GetRMS());
	fout_lsb << ichip << "\t" << ihalf << "\t" << ichannel << "\t" << TMath::Nint(hADCPed->GetMean())  << "\t" << TMath::Nint(hADCPed->GetRMS())
	     << "\t" << TMath::Nint(hTOTPed->GetMean()) << "\t" << TMath::Nint(hTOTPed->GetRMS())<< endl;
      }

  fout_lsb.close();
  rocarray1.clear();
  
  int ped_adc_msb[3][2][38], noise_adc_msb[3][2][38];
  int ped_tot_msb[3][2][38], noise_tot_msb[3][2][38];
  ofstream fout_msb("log/ped_link2.txt");
  for(int ichip=0;ichip<3;ichip++)
    for(int ihalf=0;ihalf<2;ihalf++)
      for(int ichannel=0;ichannel<38;ichannel++) {
	ped_adc_msb[ichip][ihalf][ichannel] = 0;
	noise_adc_msb[ichip][ihalf][ichannel] = 0;
	ped_tot_msb[ichip][ihalf][ichannel] = 0;
	noise_tot_msb[ichip][ihalf][ichannel] = 0;
	hADCPed->Reset("ICESM");
	hTOTPed->Reset("ICESM");
	for(const auto& roc : rocarray2){
	  if(roc.chip==ichip and roc.half==ihalf and roc.channel==ichannel) {hADCPed->Fill(roc.adc); hTOTPed->Fill(roc.tot);}
	}
	ped_adc_msb[ichip][ihalf][ichannel] = TMath::Nint(hADCPed->GetMean());
	noise_adc_msb[ichip][ihalf][ichannel] = TMath::Nint(hADCPed->GetRMS());
	ped_tot_msb[ichip][ihalf][ichannel] = TMath::Nint(hTOTPed->GetMean());
	noise_tot_msb[ichip][ihalf][ichannel] = TMath::Nint(hTOTPed->GetRMS());
	fout_msb << ichip << "\t" << ihalf << "\t" << ichannel << "\t" << TMath::Nint(hADCPed->GetMean())  << "\t" << TMath::Nint(hADCPed->GetRMS())
	     << "\t" << TMath::Nint(hTOTPed->GetMean()) << "\t" << TMath::Nint(hTOTPed->GetRMS())<< endl;
	cout << ichip << "\t" << ihalf << "\t" << ichannel << "\t" << TMath::Nint(hADCPed->GetMean())  << "\t" << TMath::Nint(hADCPed->GetRMS())
	     << "\t" << TMath::Nint(hTOTPed->GetMean()) << "\t" << TMath::Nint(hTOTPed->GetRMS())<< "\t" << hTOTPed->GetEntries() << endl;
      }

  fout_msb.close();
  rocarray2.clear();

  TH1F *hPedADC_LSB[3][2], *hNoiseADC_LSB[3][2], *hPedTOT_LSB[3][2], *hNoiseTOT_LSB[3][2];
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      hPedADC_LSB[ichip][ihalf] = new TH1F(Form("hPedADC_LSB_%d_%d",ichip,ihalf),Form("hPedADC_LSB_%d_%d",ichip,ihalf),40,-1,39);
      hNoiseADC_LSB[ichip][ihalf] = new TH1F(Form("hNoiseADC_LSB_%d_%d",ichip,ihalf),Form("hNoiseADC_LSB_%d_%d",ichip,ihalf),40,-1,39);
      hPedTOT_LSB[ichip][ihalf] = new TH1F(Form("hPedTOT_LSB_%d_%d",ichip,ihalf),Form("hPedTOT_LSB_%d_%d",ichip,ihalf),40,-1,39);
      hNoiseTOT_LSB[ichip][ihalf] = new TH1F(Form("hNoiseTOT_LSB_%d_%d",ichip,ihalf),Form("hNoiseTOT_LSB_%d_%d",ichip,ihalf),40,-1,39);
      for(int ichannel=0;ichannel<37;ichannel++) {
	hPedADC_LSB[ichip][ihalf]->SetBinContent(hPedADC_LSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), ped_adc_lsb[ichip][ihalf][ichannel]);
	hNoiseADC_LSB[ichip][ihalf]->SetBinContent(hNoiseADC_LSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), noise_adc_lsb[ichip][ihalf][ichannel]);
	hPedTOT_LSB[ichip][ihalf]->SetBinContent(hPedTOT_LSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), ped_tot_lsb[ichip][ihalf][ichannel]);
	hNoiseTOT_LSB[ichip][ihalf]->SetBinContent(hNoiseTOT_LSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), noise_tot_lsb[ichip][ihalf][ichannel]);
      }
    }
  }
  
  TH1F *hPedADC_MSB[3][2], *hNoiseADC_MSB[3][2], *hPedTOT_MSB[3][2], *hNoiseTOT_MSB[3][2];
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      hPedADC_MSB[ichip][ihalf] = new TH1F(Form("hPedADC_MSB_%d_%d",ichip,ihalf),Form("hPedADC_MSB_%d_%d",ichip,ihalf),40,-1,39);
      hNoiseADC_MSB[ichip][ihalf] = new TH1F(Form("hNoiseADC_MSB_%d_%d",ichip,ihalf),Form("hNoiseADC_MSB_%d_%d",ichip,ihalf),40,-1,39);
      hPedTOT_MSB[ichip][ihalf] = new TH1F(Form("hPedTOT_MSB_%d_%d",ichip,ihalf),Form("hPedTOT_MSB_%d_%d",ichip,ihalf),40,-1,39);
      hNoiseTOT_MSB[ichip][ihalf] = new TH1F(Form("hNoiseTOT_MSB_%d_%d",ichip,ihalf),Form("hNoiseTOT_MSB_%d_%d",ichip,ihalf),40,-1,39);
      for(int ichannel=0;ichannel<37;ichannel++) {
	hPedADC_MSB[ichip][ihalf]->SetBinContent(hPedADC_MSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), ped_adc_msb[ichip][ihalf][ichannel]);
	hNoiseADC_MSB[ichip][ihalf]->SetBinContent(hNoiseADC_MSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), noise_adc_msb[ichip][ihalf][ichannel]);
	hPedTOT_MSB[ichip][ihalf]->SetBinContent(hPedTOT_MSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), ped_tot_msb[ichip][ihalf][ichannel]);
	hNoiseTOT_MSB[ichip][ihalf]->SetBinContent(hNoiseTOT_MSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), noise_tot_msb[ichip][ihalf][ichannel]);
      }
    }
  }

  TCanvas *c1_ped_adc_lsb = new TCanvas("c1_ped_adc_lsb","c1_ped_adc_lsb",1200,800);
  c1_ped_adc_lsb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c1_ped_adc_lsb->cd(icanvas);
      hPedADC_LSB[ichip][ihalf]->Draw();
    }
  }
  
  TCanvas *c2_noise_adc_lsb = new TCanvas("c2_noise_adc_lsb","c2_noise_adc_lsb",1200,800);
  c2_noise_adc_lsb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c2_noise_adc_lsb->cd(icanvas);
      hNoiseADC_LSB[ichip][ihalf]->Draw();
    }
  }
  
  TCanvas *c3_ped_tot_lsb = new TCanvas("c3_ped_tot_lsb","c3_ped_tot_lsb",1200,800);
  c3_ped_tot_lsb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c3_ped_tot_lsb->cd(icanvas);
      hPedTOT_LSB[ichip][ihalf]->Draw();
    }
  }
  
  TCanvas *c4_noise_tot_lsb = new TCanvas("c4_noise_tot_lsb","c4_noise_tot_lsb",1200,800);
  c4_noise_tot_lsb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c4_noise_tot_lsb->cd(icanvas);
      hNoiseADC_LSB[ichip][ihalf]->Draw();
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  TCanvas *c1_ped_adc_msb = new TCanvas("c1_ped_adc_msb","c1_ped_adc_msb",1200,800);
  c1_ped_adc_msb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c1_ped_adc_msb->cd(icanvas);
      hPedADC_MSB[ichip][ihalf]->Draw();
    }
  }
  
  TCanvas *c2_noise_adc_msb = new TCanvas("c2_noise_adc_msb","c2_noise_adc_msb",1200,800);
  c2_noise_adc_msb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c2_noise_adc_msb->cd(icanvas);
      hNoiseADC_MSB[ichip][ihalf]->Draw();
    }
  }
  
  TCanvas *c3_ped_tot_msb = new TCanvas("c3_ped_tot_msb","c3_ped_tot_msb",1200,800);
  c3_ped_tot_msb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c3_ped_tot_msb->cd(icanvas);
      hPedTOT_MSB[ichip][ihalf]->Draw();
    }
  }
  
  TCanvas *c4_noise_tot_msb = new TCanvas("c4_noise_tot_msb","c4_noise_tot_msb",1200,800);
  c4_noise_tot_msb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c4_noise_tot_msb->cd(icanvas);
      hNoiseADC_MSB[ichip][ihalf]->Draw();
    }
  }

  TFile *fcanva_out = new TFile("log/Pedestal.root","recreate");
  hADCPed_Ref->Write();
  hTOTPed_Ref->Write();
  hADCPed_Ref_high->Write();
  hTOTPed_Ref_high->Write();
  hADCPed_Ref_low->Write();
  hTOTPed_Ref_low->Write();
  c1_ped_adc_lsb->Write();
  c2_noise_adc_lsb->Write();
  c3_ped_tot_lsb->Write();
  c4_noise_tot_lsb->Write();
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      hPedADC_LSB[ichip][ihalf]->Write();
      hNoiseADC_LSB[ichip][ihalf]->Write();
      hPedTOT_LSB[ichip][ihalf]->Write();
      hNoiseTOT_LSB[ichip][ihalf]->Write();
    }
  }
  c1_ped_adc_msb->Write();
  c2_noise_adc_msb->Write();
  c3_ped_tot_msb->Write();
  c4_noise_tot_msb->Write();
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      hPedADC_MSB[ichip][ihalf]->Write();
      hNoiseADC_MSB[ichip][ihalf]->Write();
      hPedTOT_MSB[ichip][ihalf]->Write();
      hNoiseTOT_MSB[ichip][ihalf]->Write();
    }
  }
  
  return 0;
}
