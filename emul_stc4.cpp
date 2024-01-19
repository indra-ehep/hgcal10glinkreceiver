#include <iostream>
#include <bitset>

#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TCanvas.h"

#include "TFileHandlerLocal.h"
#include "common/inc/FileReader.h"

#include "yaml-cpp/yaml.h"
#include <deque>

// Author : Indranil Das
// Email : Indranil.Das@cern.ch
// Affiliation : Imperial College, London
//
// Purpose : To reproduce the ECONT results using pass though data

//Command to execute : 1. ./compile.sh
//                     2. ./emul_econt.exe $Relay $rname

using namespace std;

const long double maxEvent = 2e2; //6e5

const uint64_t nShowEvents = 10;

const bool scanMode = false;
//const uint64_t scanEvent = 2587757;//Link2, modulesum mismatch
//const uint64_t scanEvent = 3619913;//Link2, one TC7 mismatch
const uint64_t scanEvent = 25;//Link2, one TC7 mismatch

struct daqdata{
  bitset<2> chip;
  bitset<1> half;
  bitset<6> channel;
  bitset<2> totflag;
  bitset<10> adc;
  //bitset<10> adcm;
  bitset<12> tot;
} ;

typedef struct{
  uint8_t modsum[2];
  uint8_t energy_raw[2][9];                    
  uint8_t loc_raw[2][9]; 
} bcdata;


typedef struct{
  uint8_t energy_raw[2][12];                    
  uint8_t loc_raw[2][12]; 
} stc4data;



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


bool is_ffsep_word(const uint64_t word, int& isMSB) {

  if( (word & 0xFFFFFFFF) == 0xFFFFFFFF){
    isMSB = 0;
    return true; //0xFFFF 0xFFFF
  }else if((word >> 32) == 0xFFFFFFFF){
    isMSB = 1;
    return true; //0xFFFF 0xFFFF
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
  const uint64_t *p64(((const uint64_t*)rEvent)+1);

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

int read_roc_data(map<uint64_t,vector<daqdata>>& rocarray, unsigned relayNumber, unsigned runNumber, unsigned linkNumber, uint64_t minEventDAQ, uint64_t maxEventDAQ)
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
  
  daqdata rdata;
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

      if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent)){ 
       	event_dump(rEvent);
      	rEvent->RecordHeader::print();
      	boe->print();
      	eoe->print();
	// const uint32_t *p32(((const uint32_t*)rEvent)+1);
	// for(unsigned i(0);i<2*rEvent->payloadLength();i++){
	//   std::cout << "EventId : " << std::setw(3) << rEvent->slinkBoe()->eventId() ;
	//   std::cout << "\t Word " << std::setw(3) << i << " ";
	//   std::cout << std::hex << std::setfill('0');
	//   std::cout << "0x" << std::setw(8) << p32[i] << std::endl;
	//   std::cout << std::dec << std::setfill(' ');
	// }
	// std::cout << std::endl;

	cout<<"Payload length : "<< rEvent->payloadLength() << endl;
      }
      
      int isMSB[6];
      int ffsep_word_loc[6];
      bool is_ff_sep_notfound = false;
      for(int iloc=0;iloc<6;iloc++){
      	isMSB[iloc] = -1;
      	ffsep_word_loc[iloc] = find_ffsep_word(rEvent, isMSB[iloc], iloc+1);
      	if(isMSB[iloc] == -1) is_ff_sep_notfound = true;
	if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent)){ 
	  if(iloc==0)
	    cout << "\nEvent : " << nEvents << ": " <<iloc+1 <<"-st 0xffff 0xffff separator word at location : " << ffsep_word_loc[iloc] << " word and isMSB : " << isMSB[iloc] << endl;
	  else if(iloc==1)
	    cout << "Event : " << nEvents << ": " <<iloc+1 <<"-nd 0xffff 0xffff separator word at location : " << ffsep_word_loc[iloc] << " word and isMSB : " << isMSB[iloc] << endl;
	  else if(iloc==2)
	    cout << "Event : " << nEvents << ": " <<iloc+1 <<"-rd 0xffff 0xffff separator word at location : " << ffsep_word_loc[iloc] << " word and isMSB : " << isMSB[iloc] << endl;
	  else
	    cout << "Event : " << nEvents << ": " <<iloc+1 <<"-th 0xffff 0xffff separator word at location : " << ffsep_word_loc[iloc] << " word and isMSB : " << isMSB[iloc] << endl;
	}
      }
      
      int expctd_empty_word_loc = ffsep_word_loc[5]+19;
      int empty_isMSB = -1;
      bool hasfound_emptry_word = found_empty_word(rEvent, empty_isMSB, expctd_empty_word_loc) ;
      if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent))
	cout << "Event : " << nEvents << ": 0x0000 0x0000 closing separator word at location : " << expctd_empty_word_loc << " hasfound_emptry_word : "<< hasfound_emptry_word << ",  and isMSB : " << empty_isMSB << endl;
      
      if(!hasfound_emptry_word) continue;
      if(is_ff_sep_notfound) continue;
      //if(nEvents>maxEvent) continue;
      
      int ichip = 0;
      const uint64_t *p64(((const uint64_t*)rEvent)+1);
      
      uint32_t wordH = ((p64[ffsep_word_loc[0]-1] >> 32) & 0xFFFFFFFF) ;
      int daq_header_payload_length = int(((wordH>>14) & 0x1FF));
      daq_header_payload_length -= 1 ; //1 for DAQ header
      if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent)){
	std::cout << std::hex << std::setfill('0');
	cout << "Event : " << nEvents << ", WordH : 0x" << wordH  ;
	std::cout << std::dec << std::setfill(' ');
	cout<<", Size in ECON-D header : " << daq_header_payload_length << endl;
      }
      
      int record_header_sizeinfo = int(rEvent->payloadLength()); //in 64-bit format
      int reduced_record_header_sizeinfo = record_header_sizeinfo - (4+2+1) ; //exclude 4 64-bit words for slink header/trailer + 2 DAQ header + 1 for DAQ trailer
      int record_header_sizeinfo_32bit = 2*reduced_record_header_sizeinfo ; //change to 32-bit format

      if(record_header_sizeinfo_32bit!=daq_header_payload_length) continue;
      
      if(boe->eventId()>=minEventDAQ and boe->eventId()<maxEventDAQ){
	
      for(int iloc=0;iloc<6;iloc++){
      	unsigned max_sep_word = (iloc==5) ? expctd_empty_word_loc : ffsep_word_loc[iloc+1] ;
      	bool ishalf = (iloc%2==0) ? false : true ;
      	if(isMSB[iloc]==1) max_sep_word++;
      	int index = 0;
      	int ch = 0;
	int adcmL = 0, adcL = 0, toaL = 0, totL = 0;
	int adcmM = 0, adcM = 0, toaM = 0, totM = 0;
      	for(unsigned i = ffsep_word_loc[iloc]+1 ; i< max_sep_word ; i++){
	  
	  
      	  uint32_t wordL = 0;
      	  uint32_t wordM = 0;
      	  if(isMSB[iloc]==0){
      	    wordL = (p64[i] & 0xFFFFFFFF) ;
      	    wordM = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	  }else if(isMSB[iloc]==1){
      	    wordL = (p64[i-1] & 0xFFFFFFFF) ;	  
      	    wordM = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	  }
	  adcmL = 0; adcL = 0; toaL = 0; totL = 0;
	  adcmM = 0; adcM = 0; toaM = 0; totM = 0;

      	  const uint16_t trigflagL = (wordL>>30) & 0x3;
	  if(trigflagL<=1){ //0 or 1
	    adcmL = (wordL>>20) & 0x3FF;
	    adcL = (wordL>>10) & 0x3FF;
	    toaL = wordL & 0x3FF;
	  }else if(trigflagL==2){
	    adcmL = (wordL>>20) & 0x3FF;
	    totL = (wordL>>10) & 0x3FF;
	    toaL = wordL & 0x3FF;	    
	  }else if(trigflagL==3){
	    adcmL = (wordL>>20) & 0x3FF;
	    totL = (wordL>>10) & 0x3FF;
	    toaL = wordL & 0x3FF;
	  }
	  if(totL>>0x9==1)  totL = (totL & 0x1ff) << 0x3 ; //10-bit to 12-bit conversion

      	  const uint16_t trigflagM = (wordM>>30) & 0x3;
	  if(trigflagM<=1){ //0 or 1
	    adcmM = (wordM>>20) & 0x3FF;
	    adcM = (wordM>>10) & 0x3FF;
	    toaM = wordM & 0x3FF;
	  }else if(trigflagM==2){
	    adcmM = (wordM>>20) & 0x3FF;
	    totM = (wordM>>10) & 0x3FF;
	    toaM = wordM & 0x3FF;	    
	  }else if(trigflagM==3){
	    adcmM = (wordM>>20) & 0x3FF;
	    totM = (wordM>>10) & 0x3FF;
	    toaM = wordM & 0x3FF;
	  }
	  if(totM>>0x9==1)  totM = (totM & 0x1ff) << 0x3 ; //10-bit to 12-bit conversion

	  if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent)){ 
	    if(isMSB[iloc]==0){
	      cout<<"\ti : "<<i<<", index : "<<index<<endl;
	      std::cout << std::hex << std::setfill('0');
	      cout<<"\tWordM : 0x" << std::setw(8) << wordM <<", wordL : 0x" << std::setw(8) << wordL<< std::endl;
	      std::cout << std::dec << std::setfill(' ');
	      cout<<"\tM:(flag,adc,tot,toa) : (" << trigflagM <<", " << adcM << ", " << totM <<", " << toaM << "), \t"
		  <<"L:(flag,adc,tot,toa) : (" << trigflagL <<", " << adcL << ", " << totL <<", " << toaL << ")" << endl;
	    }else{
	      cout<<"\ti : "<<i<<", index : "<<index<<endl;
	      std::cout << std::hex << std::setfill('0');
	      cout<<"\tWordL[i-1] : 0x" << std::setw(8) << wordL<<", wordM : 0x" << std::setw(8) << wordM << std::endl;
	      std::cout << std::dec << std::setfill(' ');
	      cout<<"\tL:(flag,adc,tot,toa)[i-1] : (" << trigflagL <<", " << adcL << ", " << totL <<", " << toaL << "), \t"
		  <<"M:(flag,adc,tot,toa) : (" << trigflagM <<", " << adcM << ", " << totM <<", " << toaM << ")" << endl;
	    }
	  }
	  
      	  if(isMSB[iloc]==0){
      	    rdata.chip = uint16_t(ichip);
      	    rdata.half = uint16_t(ishalf);
      	    rdata.channel = ch ;
      	    rdata.adc = adcM;
	    //rdata.adcm = adcmM ; 
      	    rdata.tot = totM;
      	    rdata.totflag = trigflagM;
      	    ch++;
      	    rocarray[boe->eventId()].push_back(rdata);
	    if(index!=18){
	      rdata.chip = uint16_t(ichip);
	      rdata.half = uint16_t(ishalf);
	      rdata.channel = ch ;
	      rdata.adc = adcL;
	      //rdata.adcm = adcmL ; 
	      rdata.tot = totL;
	      rdata.totflag = trigflagL;
	      ch++;
	      rocarray[boe->eventId()].push_back(rdata);
	    }
      	  }else{
	    
      	    rdata.chip = uint16_t(ichip);
      	    rdata.half = uint16_t(ishalf);
      	    rdata.channel = ch ;
      	    rdata.adc = adcL;
	    //rdata.adcm = adcmL ; 
      	    rdata.tot = totL;
      	    rdata.totflag = trigflagL;
      	    ch++;
      	    rocarray[boe->eventId()].push_back(rdata);
	    
	    if(index!=18){
	      rdata.chip = uint16_t(ichip);
	      rdata.half = uint16_t(ishalf);
	      rdata.channel = ch ;
	      rdata.adc = adcM;
	      //rdata.adcm = adcmM ; 
	      rdata.tot = totM;
	      rdata.totflag = trigflagM;
	      ch++;
	      rocarray[boe->eventId()].push_back(rdata);
	    }
      	  }
      	  index++;
      	}
      	//cout<<endl;
      	if(iloc%2==1)ichip++;
      }
      }
      //Increment event counter
      nEvents++;
      
    }
  }//file reader
  
  _fileReader.close();
  delete r;
  
  return true;
}


int read_econt_data_stc4(map<uint64_t,stc4data>& econtarray, unsigned relayNumber, unsigned runNumber, uint64_t minEventTrig, uint64_t maxEventTrig)
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
  
  stc4data edata;
  uint64_t nEvents = 0;
  uint64_t prevEvent = 0;
  uint32_t prevSequence = 0;
  uint32_t packet[4];
  uint32_t packet_counter;
  uint32_t packet_locations[12];
  uint64_t packet_energies[12];
  
  uint16_t daq_data[5];                              
  uint16_t daq_nbx[5];                               
  uint16_t size_in_cafe[5];                          
  uint32_t bx_raw[2][15];                        
  int refbxindex = 7;

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

      if(boe->boeHeader()!=boe->BoePattern) continue;
      if(eoe->eoeHeader()!=eoe->EoePattern) continue;
      uint16_t l1atype = boe->l1aType();      
      if(l1atype==0) continue;
      
      const uint64_t *p64(((const uint64_t*)rEvent)+1);

      // edata.event = nEvents;
      // edata.eventId = boe->eventId();
      // edata.bxId = eoe->bxId();
      // edata.sequenceId = rEvent->RecordHeader::sequenceCounter(); 

      if((Abs64(boe->eventId(),prevEvent) != Abs32(rEvent->RecordHeader::sequenceCounter(), prevSequence)) and Abs64(boe->eventId(),prevEvent)>=2){
	prevEvent = boe->eventId();
	prevSequence = rEvent->RecordHeader::sequenceCounter(); 
	continue;
      }
      prevEvent = boe->eventId();
      prevSequence = rEvent->RecordHeader::sequenceCounter(); 

      daq_data[0] = p64[2] & 0xF;
      daq_nbx[0] = p64[2]>>4 & 0x7;
      uint64_t daq0_event_size = (2*daq_nbx[0] + 1)*daq_data[0];
      int first_cafe_word_loc = find_cafe_word(rEvent, 1);
      size_in_cafe[0] = p64[first_cafe_word_loc] & 0xFF;      
      
      daq_data[1] = p64[2]>>7 & 0xF;
      daq_nbx[1] = p64[2]>>11 & 0x7;
      uint64_t daq1_event_size = (2*daq_nbx[1] + 1)*daq_data[1];
      int second_cafe_word_loc = find_cafe_word(rEvent, 2);
      size_in_cafe[1] = p64[second_cafe_word_loc] & 0xFF;
      
      int sixth_cafe_word_loc = find_cafe_word(rEvent, 6);
      
      if(sixth_cafe_word_loc!=0) continue;
      if(first_cafe_word_loc != 3) continue;
      if(daq0_event_size != size_in_cafe[0]) continue;	
      if(daq1_event_size != size_in_cafe[1]) continue;	

      if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent)) {
	event_dump(rEvent);
	rEvent->RecordHeader::print();
      	boe->print();
      	eoe->print();
      }
      //if(nEvents>maxEvent) continue;      
      if(boe->eventId()>=minEventTrig and boe->eventId()<maxEventTrig){
	
      int bx_index = -1.0*int(daq_nbx[0]);
      const int maxnbx = (2*daq_nbx[0] + 1);
      uint64_t energy_raw[2][maxnbx][12];
      for(int iect=0;iect<2;iect++)
  	for(int ibx=0;ibx<maxnbx;ibx++){
	  bx_raw[iect][ibx] = 0;
  	  for(int istc=0;istc<12;istc++){
  	    energy_raw[iect][ibx][istc] = 0;
  	    edata.energy_raw[iect][istc] = 0;
  	    edata.loc_raw[iect][istc] = 0;
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
	
      	if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent)) {
	  std::cout<<" EventId : "<<boe->eventId()
		   << ", bx_index : " << (bx_index+int(daq_nbx[0])) 
		   <<", bx(LSB) :"  << bx_counter
		   <<", edata.bxId : "<<eoe->bxId() <<", modulo8 : "<< (eoe->bxId()%8)
		   <<", totEnergy : "<<totEnergy
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
	
	bx_raw[0][(bx_index+int(daq_nbx[0]))] = bx_counter;
      	for(int istc=0;istc<12;istc++){
      	  energy_raw[0][(bx_index+int(daq_nbx[0]))][istc] = packet_energies[istc];
      	}
	if((bx_index+int(daq_nbx[0]))==refbxindex){
	  for(int istc=0;istc<12;istc++){
	    edata.energy_raw[0][istc] = packet_energies[istc];
	    edata.loc_raw[0][istc] = packet_locations[istc];
	  }
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

      	if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent)) {
	  std::cout<<" EventId : "<<boe->eventId()
		   << ", bx_index : " << (bx_index+int(daq_nbx[0])) 
		   <<", bx(MSB) :"  << bx_counter1
		   <<", edata.bxId : "<<eoe->bxId() <<", modulo8 : "<< (eoe->bxId()%8)
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
	
	bx_raw[1][(bx_index+int(daq_nbx[0]))] = bx_counter1;
      	for(int istc=0;istc<12;istc++){
      	  energy_raw[1][(bx_index+int(daq_nbx[0]))][istc] = packet_energies[istc];
      	}
	if((bx_index+int(daq_nbx[0]))==refbxindex){
	  for(int istc=0;istc<12;istc++){
	    edata.energy_raw[1][istc] = packet_energies[istc];
	    edata.loc_raw[1][istc] = packet_locations[istc];
	  }
	}
	
      	bx_index++;
      }//loop over unpacked
      
      econtarray[boe->eventId()] = edata;
      if ((nEvents < nShowEvents) or (scanMode and boe->eventId()==scanEvent)) std::cout<<std::endl;
      }
      //Increment event counter
      nEvents++;

    }
  }//file reader

  _fileReader.close();
  return true;
}

void ReadChannelMapping(map<int,tuple<int,int,int,int>>& tctorocch)
{
  string Dens;
  unsigned Wtype, ROC, HalfROC, Seq;
  string ROCpin;
  unsigned ROCCH;
  int SiCell, TrLink, TrCell, iu, iv;
  float trace;
  int t;
  //Dens   Wtype     ROC HalfROC     Seq  ROCpin  SiCell  TrLink  TrCell      iu      iv   trace       t
  ifstream inwafermap("input/WaferCellMapTraces.txt");
  stringstream ss;
  string s;
  
  map<int,pair<int,int>> rocchtoiuiv;
  //;
  rocchtoiuiv.clear();
  tctorocch.clear();
  
  int prevTrigCell = -1;
  int itc = 0;
  unsigned rocpin[4];
  while(getline(inwafermap,s)){
    //cout << s.size() << endl;
    if(s.find(" LD ")!=string::npos or s.find(" HD ")!=string::npos){
      //cout << s << endl;
      ss << s.data() << endl;
      ss >> Dens >> Wtype >> ROC >> HalfROC >> Seq >> ROCpin >> SiCell  >> TrLink  >> TrCell  >> iu  >> iv  >> trace  >> t ;
      if(ROCpin.find("CALIB")==string::npos){
	ROCCH = stoi(ROCpin);
	if(Dens.find("LD")!=string::npos){
	  //if(Wtype==0 and ROC==0 and TrLink!=-1 and TrCell!=-1){
	  if(Wtype==0 and TrLink!=-1 and TrCell!=-1){
	    int absTC = ROC*16 + TrLink*4 + TrCell;
	    //cout <<"\t"<< Dens <<"\t"<< Wtype <<"\t"<< ROC <<"\t"<< HalfROC <<"\t"<< Seq <<"\t"<< ROCpin <<"\t"<< SiCell  <<"\t"<< TrLink  <<"\t"<< TrCell  <<"\t new TC : "<< absTC <<"\t"<< iu  <<"\t"<< iv  <<"\t"<< trace  <<"\t"<< t << endl ;
	    if(itc==3) prevTrigCell = absTC;
	    rocpin[itc++] = ROC*72 + ROCCH;
	    rocchtoiuiv[ROCCH] = make_pair(iu, iv);
	  }
	  if(itc==4){
	    tctorocch[prevTrigCell] = make_tuple(rocpin[0],rocpin[1],rocpin[2],rocpin[3]);
	    itc = 0;
	  }
	}//pick lines for LD modules
      }
    }else{
      ;//cout << s << endl;
    }
  }
  inwafermap.close();
  for(const auto& tcmap : tctorocch){
    if(tcmap.first < 8 )
      cout <<"TC " <<tcmap.first<<", pins :  ("<< get<0>(tcmap.second) << ", "<< get<1>(tcmap.second) << ", "<< get<2>(tcmap.second) << ", "<< get<3>(tcmap.second) << ") "<<endl;
    else
      cout <<"TC " <<tcmap.first<<", pins :  ("<< get<0>(tcmap.second) << ", "<< get<1>(tcmap.second) << ", "<< get<2>(tcmap.second) << ", "<< get<3>(tcmap.second) << ") "<<endl;
  }

  
}

int read_Ped_from_yaml(unsigned refRelay, unsigned refRun, unsigned link, unsigned pedestal_adc[][3][2][36], unsigned threshold_adc[][3][2][36])
{
  string infile = "";
  if(link==1)
    infile = Form("dat/Relay%u/Run%u_Module00c87fff.yaml",refRelay,refRun);
  else if(link==2)
    infile = Form("dat/Relay%u/Run%u_Module00c43fff.yaml",refRelay,refRun);

  int isMSB = (link==1)?0:1;
  cout<<"Module config file : "<<infile<<endl;
  YAML::Node node(YAML::LoadFile(infile));
  //cout << node << endl;
  cout<<"============"<<endl;
  if(link==1){
    cout << node["Configuration"]["roc"]["train_1.roc0_e0"]["cfg"]["DigitalHalf"]["0"]["Adc_TH"].as<unsigned>() << endl;
    cout << node["Configuration"]["roc"]["train_1.roc0_e0"]["cfg"]["ch"]["0"]["Adc_pedestal"].as<unsigned>() << endl;
    cout << node["Configuration"]["roc"]["train_1.roc0_e0"]["cfg"]["ch"]["1"]["Adc_pedestal"].as<unsigned>() << endl;
    cout << node["Configuration"]["roc"]["train_1.roc0_e0"]["cfg"]["ch"]["2"]["Adc_pedestal"].as<unsigned>() << endl;
    cout << node["Configuration"]["roc"]["train_1.roc0_e0"]["cfg"]["ch"]["3"]["Adc_pedestal"].as<unsigned>() << endl;
  }else{
    cout << node["Configuration"]["roc"]["train_0.roc0_w0"]["cfg"]["DigitalHalf"]["0"]["Adc_TH"].as<unsigned>() << endl;
    cout << node["Configuration"]["roc"]["train_0.roc0_w0"]["cfg"]["ch"]["0"]["Adc_pedestal"].as<unsigned>() << endl;
    cout << node["Configuration"]["roc"]["train_0.roc0_w0"]["cfg"]["ch"]["1"]["Adc_pedestal"].as<unsigned>() << endl;
    cout << node["Configuration"]["roc"]["train_0.roc0_w0"]["cfg"]["ch"]["2"]["Adc_pedestal"].as<unsigned>() << endl;
    cout << node["Configuration"]["roc"]["train_0.roc0_w0"]["cfg"]["ch"]["3"]["Adc_pedestal"].as<unsigned>() << endl;
  }
  cout<<"============"<<endl;
  for(int iroc=0;iroc<3;iroc++){
    string rocname = (link==1)?Form("train_1.roc%d_e0",iroc):Form("train_0.roc%d_w0",iroc);
    unsigned th_0 = node["Configuration"]["roc"][rocname]["cfg"]["DigitalHalf"]["0"]["Adc_TH"].as<unsigned>();
    unsigned th_1 = node["Configuration"]["roc"][rocname]["cfg"]["DigitalHalf"]["1"]["Adc_TH"].as<unsigned>();
    cout<<"rocname : "<<rocname<<", th_0 : "<<th_0<<", th_1 : "<<th_1<<endl;
    for(int ich=0;ich<72;ich++){
      int ihalf = (ich<=35)?0:1;
      int chnl = ich%36;
      unsigned adc_th = (ich<=35)?th_0:th_1;
      pedestal_adc[isMSB][iroc][ihalf][chnl] = node["Configuration"]["roc"][rocname]["cfg"]["ch"][Form("%d",ich)]["Adc_pedestal"].as<unsigned>() ;
      threshold_adc[isMSB][iroc][ihalf][chnl] = adc_th;
    }
  }
  cout<<"============"<<endl;

  
  
  return true;
}

uint32_t compress_roc(uint32_t val, bool isldm)
{
  
  uint32_t maxval = 0x3FFFF ;
  uint32_t maxval_ldm = 0x7FFFF ;
  uint32_t maxval_hdm = 0x1FFFFF ;
  val = (isldm)?val>>1:val>>3;
  if(isldm){
    if(val>maxval_ldm) val = maxval_ldm;
  }else{
    if(val>maxval_hdm) val = maxval_hdm;
  }
  if(val>maxval) val = maxval;
  
  uint32_t r = 0; // r will be lg(v)
  uint32_t sub ;
  uint32_t shift ;
  uint32_t mant ;
  
  if(val>7){
    uint32_t v = val; 
    r = 0; 
    while (v >>= 1) r++;
    sub = r - 2;
    shift = r - 3;
    mant = (val>>shift) & 0x7;
  }else{
    r = 0;
    sub = 0;
    shift = 0;
    mant = val & 0x7;
  }
  
  bitset<4> expo = sub;
  bitset<3> mantissa = mant; 
  
  uint32_t packed = (sub<<3) | mant;
  
  return packed;
}

uint32_t decompress_econt(uint32_t compressed, bool isldm)
{
  uint32_t mant = compressed & 0x7;
  uint32_t expo = (compressed>>3) & 0xF;

  if(expo==0) 
    return (isldm) ? (mant<<1) : (mant<<3) ;

  uint32_t shift = expo+2;
  uint32_t decomp = 1<<shift;
  uint32_t mpdeco = 1<<(shift-4);
  decomp = decomp | (mant<<(shift-3));
  decomp = decomp | mpdeco;
  decomp = (isldm) ? (decomp<<1) : (decomp<<3) ;
  
  return decomp;
}

uint32_t compress_econt(uint32_t val, bool isldm)
{
  
  uint32_t maxval = 0x3FFFFF ; //22 bit
  uint32_t maxval_ldm = 0x7FFFF ;
  uint32_t maxval_hdm = 0x1FFFFF ;
  val = (isldm)?val>>1:val>>3;  
  // if(isldm){
  //   if(val>maxval_ldm) val = maxval_ldm;
  // }else{
  //   if(val>maxval_hdm) val = maxval_hdm;
  // }
  if(val>maxval) val = maxval;

  //Ref:https://graphics.stanford.edu/%7Eseander/bithacks.html
  uint32_t r = 0; // r will be lg(v)
  uint32_t sub ;
  uint32_t shift ;
  uint32_t mant ;
  
  if(val>7){
    uint32_t v = val; 
    r = 0; 
    while (v >>= 1) r++;
    sub = r - 2;
    shift = r - 3;
    mant = (val>>shift) & 0x7;
  }else{
    r = 0;
    sub = 0;
    shift = 0;
    mant = val & 0x7;
  }
  
  bitset<4> expo = sub;
  bitset<3> mantissa = mant; 
  
  uint32_t packed = (sub<<3) | mant;
  
  return packed;
}

uint32_t compress_econt_stc4(uint32_t val, bool isldm)
{
  
  // uint32_t maxval = 0x3FFFFF ; //22 bit
  // uint32_t maxval_ldm = 0x7FFFF ;
  // uint32_t maxval_hdm = 0x1FFFFF ;
  val = (isldm)?val>>1:val>>3;  
  // // if(isldm){
  // //   if(val>maxval_ldm) val = maxval_ldm;
  // // }else{
  // //   if(val>maxval_hdm) val = maxval_hdm;
  // // }
  // if(val>maxval) val = maxval;

  //Ref:https://graphics.stanford.edu/%7Eseander/bithacks.html
  uint32_t r = 0; // r will be lg(v)
  uint32_t sub ;
  uint32_t shift ;
  uint32_t mant ;
  
  if(val>7){
    uint32_t v = val; 
    r = 0; 
    while (v >>= 1) r++;
    sub = r - 2;
    shift = r - 3;
    if(sub<=15){
      mant = (val>>shift) & 0x7;
    }else{
      sub = 0xF;
      mant = 0x7;
    }
  }else{
    r = 0;
    sub = 0;
    shift = 0;
    mant = val & 0x7;
  }
  
  bitset<4> expo = sub;
  bitset<3> mantissa = mant; 
  
  uint32_t packed = (sub<<3) | mant;
  
  return packed;
}


uint32_t decompress_compress_econt_PD(uint32_t compressed, bool isldm, uint32_t& rawEnergy_)
{

  uint32_t inputCode_= compressed;
  
  uint32_t e(inputCode_>>3);
  uint32_t m(inputCode_&0x7);
  
  if(isldm) {
    rawEnergy_=2*m+1;
    if(e>=1) rawEnergy_+=2*8;
    if(e>=2) rawEnergy_=rawEnergy_<<(e-1);
  } else {
    rawEnergy_=8*m+4;
    if(e>=1) rawEnergy_+=8*8;
    if(e>=2) rawEnergy_=rawEnergy_<<(e-1);
  }//check ldm()
  
  uint32_t calibration = 0x800;
  
  uint32_t calEnergy_ = (rawEnergy_*calibration)>>11;
  
  uint32_t outEnergy_ = (isldm) ? (calEnergy_>>1) : (calEnergy_>>3) ;
  
  uint32_t  outputCode_ ;
  if(outEnergy_<16) outputCode_=outEnergy_;
  else {
    for(unsigned i(0);i<32;i++) {
      if((outEnergy_&(1<<(31-i)))!=0) {
	e=29-i;
	m=(outEnergy_>>(e-1))&0x7;
	
	//std::cout << "Temp i,e,m = " << i << ", " << e << ", " << m << std::endl;
	
	outputCode_=8*e+m;
	if(outputCode_>127) outputCode_=127;
	i=999;
      }
    }
  }//Check outEnergy_

  return outputCode_;

}


int CompareSTC4Energies(map<uint64_t,stc4data>& econtarray, map<uint64_t,vector<daqdata>>& rocarray, int isMSB, map<int,tuple<int,int,int,int>>& tctorocch, unsigned pedestal_adc[][3][2][36], unsigned threshold_adc[][3][2][36], TDirectory*& dir_diff)
{
  // ////./emul_econt.exe 1695829026 1695829027
  // //===============================================================================================================================
  // //BC9 New
  // //===============================================================================================================================
  TList *list = (TList *)dir_diff->GetList();
  
  int choffset = get<0>(tctorocch[8]) ; //==36, since there are 16 TC per chip
  cout<<"choffset : "<<choffset<<endl;      
  for(const auto& [eventId, econt] : econtarray){
    
    if(eventId%1000==0)  cout << "Processing event : "<<eventId<<"  .........."<< endl;
    if(eventId<=10 or (scanMode and eventId==scanEvent)){
      cout << "Event : "<<eventId<< endl;
      cout<<"E : \t"; for(int istc=0;istc<12;istc++) cout<<uint16_t(econt.energy_raw[isMSB][istc])<<" "; cout<<endl;
      cout<<"L : \t"; for(int istc=0;istc<12;istc++) cout<<setw(2)<<std::setfill('0')<<uint16_t(econt.loc_raw[isMSB][istc])<<" "; cout<<endl;  
    }

    for(unsigned istc = 0 ; istc < 12 ; istc++) ((TH1F *) list->FindObject(Form("hSTCEnergy_%d",istc)))->Fill(float(econt.energy_raw[isMSB][istc]));
    
    //for(unsigned istc = 0 ; istc < 12 ; istc++){
    for(unsigned istc = 0 ; istc < 12 ; istc++){
      
      int noftctot = 0;
      uint32_t stcsum = 0;
      bool istot1STC = false;
      bool istot2STC = false;
      bool issattotSTC = false;
      int noftot3STC = 0;
      uint32_t totadcSTC = 0;
      uint32_t compressedSTC = 0;
      uint32_t decompressedSTC = 0;
      uint32_t decompressedupSTC = 0;

      bool hasChADC[17];
      for(int ichadc=0;ichadc<17;ichadc++) hasChADC[ichadc] == false;
      
      for(unsigned itc = 4*istc ; itc < 4*istc + 4 ; itc++ ){
    
	uint32_t compressed = 0;
	uint32_t compressedup = 0;

	uint32_t decompressed = 0;
	uint32_t decompressedup = 0;

      
	bool iscp0hf0ch0adc = false;
	bool iscp0hf0ch1adc = false;
	bool iscp0hf0ch2adc = false;
	bool iscp0hf0ch3adc = false;
	uint32_t refadcch0 = 0;
	uint32_t refadcch1 = 0;	    
	uint32_t refadcch2 = 0;
	uint32_t refadcch3 = 0;
	bool iscp0hf0ch0tot = false;
	bool iscp0hf0ch1tot = false;
	bool iscp0hf0ch2tot = false;
	bool iscp0hf0ch3tot = false;
	uint32_t reftotch0 = 0;
	uint32_t reftotch1 = 0;
	uint32_t reftotch2 = 0;
	uint32_t reftotch3 = 0;

	uint32_t totadc = 0;
	uint32_t totadcup = 0;

	int noftot3 = 0;
	bool istot1 = false;
	bool istot2 = false;      
	bool issattot = false;
	uint32_t multfactor = 15 ;
	
	for(const auto& roc : rocarray[eventId]){
	  
	  if(roc.channel.to_ulong()>=37 or roc.channel.to_ulong()==18) continue;
	  //if(roc.totflag.to_ulong()==2) continue;
	  int roc_chip = int(roc.chip.to_ulong());
	  int roc_half = int(roc.half.to_ulong());
	  int roc_channel = int(roc.channel.to_ulong());
	  int roc_totflag = int(roc.totflag.to_ulong());
	  
	  int ch = (roc_half==1)?(roc_channel+choffset):roc_channel;
	  if(roc_channel>18) ch -= 1;
	  int pedch = ch%36;
	  ch += 72*roc_chip;
	  if(roc_totflag==0 or roc_totflag==1 or roc_totflag==2){
	    unsigned ped = pedestal_adc[isMSB][roc_chip][roc_half][pedch];
	    if(ped==255) ped = 1024; //pedestal value 255 has been used for masked channels
	    if(isMSB==1 and ped<10) ped = 255; //pedestal value 255 has been used for masked channels only for link2
	    unsigned thr = threshold_adc[isMSB][roc_chip][roc_half][pedch];
	    uint32_t adc = roc.adc.to_ulong() & 0x3FF;
	    adc = (adc>(ped+thr)) ? adc-ped : 0 ;
	    if(ch==get<0>(tctorocch[itc]) or ch==get<1>(tctorocch[itc]) or ch==get<2>(tctorocch[itc]) or ch==get<3>(tctorocch[itc])){
	      if(ch==get<0>(tctorocch[itc])) {iscp0hf0ch0adc = true; refadcch0 = roc.adc.to_ulong();}
	      if(ch==get<1>(tctorocch[itc])) {iscp0hf0ch1adc = true; refadcch1 = roc.adc.to_ulong();}
	      if(ch==get<2>(tctorocch[itc])) {iscp0hf0ch2adc = true; refadcch2 = roc.adc.to_ulong();}
	      if(ch==get<3>(tctorocch[itc])) {iscp0hf0ch3adc = true; refadcch3 = roc.adc.to_ulong();}
	      totadc += adc ;
	      totadcup += adc ;

	    //   if(adc>0) hasChADC[ch] = true;
	      if(roc.totflag==1) {istot1 = true; istot1STC = true; }
	      if(roc.totflag==2) {istot2 = true; istot2STC = true; }
	    //   //if(roc_totflag==0) ((TH1F *) list1->FindObject(Form("hADC_ch_flag0_%d_%d_%d",roc_chip,roc_half,pedch)))->Fill(float(roc.adc.to_ulong()));
	      if(eventId<=10 or (scanMode and eventId==scanEvent))
		cout<<"\t\tievent : " << eventId <<", chip : " << roc_chip << ", half : "<<roc_half<< ", channel : " << roc_channel<<", ch : "<<ch<<", adc_raw : "<<roc.adc.to_ulong()<<", adc : "<<adc<<", pedch : "<<pedch<<", ped : "<<ped<<", totflag : "<<roc_totflag <<", tot : "<<roc.tot.to_ulong()<<", totadc : "<<totadc<<endl;
	  }
	  }
	  if(roc.totflag==3){
	    uint32_t tot = roc.tot.to_ulong() ;
	    uint32_t totup = tot + 7;
	    
	    uint32_t totlin = tot*multfactor;
	    uint32_t totlinup = totup*multfactor;

	    if(ch==get<0>(tctorocch[itc]) or ch==get<1>(tctorocch[itc]) or ch==get<2>(tctorocch[itc]) or ch==get<3>(tctorocch[itc])){
	      if(ch==get<0>(tctorocch[itc])) {iscp0hf0ch0tot = true; reftotch0 = roc.tot.to_ulong();}
	      if(ch==get<1>(tctorocch[itc])) {iscp0hf0ch1tot = true; reftotch1 = roc.tot.to_ulong();}
	      if(ch==get<2>(tctorocch[itc])) {iscp0hf0ch2tot = true; reftotch2 = roc.tot.to_ulong();}
	      if(ch==get<3>(tctorocch[itc])) {iscp0hf0ch3tot = true; reftotch3 = roc.tot.to_ulong();}	    
	      if(isMSB==1) {
		if(ch!=137) {
		  totadc += totlin ;
		  totadcup += totlinup ;
		}
	      }else if(isMSB==0) {
		totadc += totlin ;
		totadcup += totlinup ;
	      }
	      if(roc_totflag==3) {noftot3++; noftot3STC++;}
	      if(roc.tot.to_ulong()>=512 and roc_totflag==3) {issattot = true; issattotSTC = true;}
	      //((TH1F *) list1->FindObject(Form("hADC_ch_flag3_%d_%d_%d",roc_chip,roc_half,pedch)))->Fill(float(roc.adc.to_ulong()));
	      //((TH1F *) list1->FindObject(Form("hTOT_ch_flag3_%d_%d_%d",roc_chip,roc_half,pedch)))->Fill(float(roc.tot.to_ulong()));
	      if(eventId<=10 or (scanMode and eventId==scanEvent))
		cout<<"\t\tievent : " << eventId <<", chip : " << roc_chip << ", half : "<<roc_half<< ", channel : " << roc_channel<<", ch : "<<ch<<", adc : "<<roc.adc.to_ulong()<<", totflag : "<<roc_totflag <<", tot : "<<tot<<", multfactor : "<<multfactor<<", totadc : "<<totadc<<endl;
	    }
	  
	  }
	}//roc for loop
      
	if(!istot1 and !istot2) {
	  compressed = compress_roc(totadc, 1);
	  compressedup = compress_roc(totadcup, 1);

	  decompressed = decompress_econt(compressed, 1);
	  decompressedup = decompress_econt(compressedup, 1);
	  
	  uint32_t calib = 0x800;
	  decompressed = (calib*decompressed)>>11;
	  decompressedup = (calib*decompressedup)>>11;
	}
	
	//=========
	{
	  totadcSTC += totadc ;
	  compressedSTC += compressed ;
	  decompressedSTC += decompressed ;
	  decompressedupSTC += decompressedup ;
	}
	
      }//trigger cell for loop

      decompressedSTC += 4;
      decompressedupSTC += 4;
      
      if(!istot1STC and !istot2STC){
	
	//cout<<"itc : "<<itc<<", decompressed : " << decompressed << endl;
	float diff = float(compressedSTC) - float(econt.energy_raw[isMSB][istc]);
	((TH1F *) list->FindObject("hCompressDiff"))->Fill(diff);
	
	uint32_t rawsum = 0;
	uint32_t compressed_econt_PD = decompress_compress_econt_PD(compressedSTC, 1, rawsum);
	uint32_t compressed_econt = compress_econt_stc4(decompressedSTC, 1);
	uint32_t compressedup_econt = compress_econt_stc4(decompressedupSTC, 1);
	
	if(eventId<=10 or (scanMode and eventId==scanEvent))
	  cout <<"\t  istc : "<<(istc)<<", totadcSTC : "<<totadcSTC
	       <<", compressedSTC : "<< compressedSTC
	       << ", decompressedSTC [econt-t input] : "<< decompressedSTC
	       <<", compressed_econtSTC [econ-t output] : "<< compressed_econt 
	       << endl;
	diff = float(compressed_econt) - float(econt.energy_raw[isMSB][istc]);
	float diffup = float(compressedup_econt) - float(econt.energy_raw[isMSB][istc]);

	bool isZero = false;
	if(compressed_econt==0 or econt.energy_raw[isMSB][istc]==0) isZero = true;
	if(TMath::Abs(diff)>1 and noftot3STC==0 and !isZero){
	  cout<<"ADC::Large Diff : "<<diff<< ", Event : "<<eventId<<endl;
	  cout<<"E : \t"; for(int jstc=0;jstc<12;jstc++) cout<<uint16_t(econt.energy_raw[isMSB][jstc])<<" "; cout<<endl;
	  cout<<"L : \t"; for(int jstc=0;jstc<12;jstc++) cout<<setw(2)<<std::setfill('0')<<uint16_t(econt.loc_raw[isMSB][jstc])<<" "; cout<<endl;  
	  cout <<"\t  istc : "<<(istc)<<", totadcSTC : "<<totadcSTC
	       <<", compressedSTC : "<< compressedSTC
	       << ", decompressedSTC [econt-t input] : "<< decompressedSTC
	       <<", compressed_econtSTC [econ-t output] : "<< compressed_econt 
	       << endl;
	}
	if(TMath::Abs(diff)>2 and noftot3STC!=0 and !isZero){
	  cout<<"TOT::Large Diff : "<<diff<< ", Event : "<<eventId<<endl;
	  cout<<"E : \t"; for(int jstc=0;jstc<12;jstc++) cout<<uint16_t(econt.energy_raw[isMSB][jstc])<<" "; cout<<endl;
	  cout<<"L : \t"; for(int jstc=0;jstc<12;jstc++) cout<<setw(2)<<std::setfill('0')<<uint16_t(econt.loc_raw[isMSB][jstc])<<" "; cout<<endl;  
	  cout <<"\t  istc : "<<(istc)<<", totadcSTC : "<<totadcSTC
	       <<", compressedSTC : "<< compressedSTC
	       << ", decompressedSTC [econt-t input] : "<< decompressedSTC
	       <<", compressed_econtSTC [econ-t output] : "<< compressed_econt 
	       << endl;
	}
	if(!isZero){
	  ((TH1F *) list->FindObject("hTCHits"))->Fill(istc);
	  //((TH1F *) list->FindObject(Form("hSTCEnergy_%d",istc)))->Fill(float(econt.energy_raw[isMSB][istc]));
	  ((TH1F *) list->FindObject(Form("hEmulEnergy_%d",istc)))->Fill(float(compressed_econt));
	}
	if(noftot3STC==0 and !isZero){
	  ((TH1F *) list->FindObject("hCompressDiffECONT"))->Fill(diff);
	  if(TMath::Abs(diff)==0){
	    ((TH1F *) list->FindObject("hCompressDiffMatchedRaw"))->Fill(float(econt.energy_raw[isMSB][istc]));
	    ((TH1F *) list->FindObject("hCompressDiffMatchedEmul"))->Fill(float(compressed_econt));
	    for(int ichadc=0;ichadc<17;ichadc++)
	      if(hasChADC[ichadc])
		((TH1F *) list->FindObject("hMatchedCh"))->Fill(float(ichadc));
	  }else{
	    ((TH1F *) list->FindObject("hCompressDiffUnMatchedRaw"))->Fill(float(econt.energy_raw[isMSB][istc]));
	    ((TH1F *) list->FindObject("hCompressDiffUnMatchedEmul"))->Fill(float(compressed_econt));
	    for(int ichadc=0;ichadc<17;ichadc++)
	      if(hasChADC[ichadc])
		((TH1F *) list->FindObject("hUnMatchedCh"))->Fill(float(float(ichadc)));
	  }
	  ((TH1F *) list->FindObject(Form("hCompressDiffSTCADC_%d",istc)))->Fill(diff);
	}
	if(noftot3STC!=0 and !isZero){
	  ((TH1F *) list->FindObject("hCompressDiffTOTECONT"))->Fill(diff);
	  ((TH1F *) list->FindObject(Form("hCompressDiffSTCTOT_%d",istc)))->Fill(diff);
	  if(issattotSTC){
	    ((TH1F *) list->FindObject("hCompressDiffTOTUPECONT"))->Fill(diffup);
	    ((TH1F *) list->FindObject(Form("hCompressDiffSTCTOTUP_%d",istc)))->Fill(diffup);
          }else{
	    ((TH1F *) list->FindObject("hCompressDiffTOTUPECONT"))->Fill(diff);
	    ((TH1F *) list->FindObject(Form("hCompressDiffSTCTOTUP_%d",istc)))->Fill(diff);
	  }
	}
	diff = float(compressed_econt_PD) - float(econt.energy_raw[isMSB][istc]);
	if(noftot3STC==0 and !isZero) ((TH1F *) list->FindObject("hCompressDiffECONTPD"))->Fill(diff);
	
      }else{
	// if(!isZeroMS)
	// 	((TH1F *) list->FindObject("hModSumDiffMissed"))->Fill(diffmodsum_4E3M);
	;
      }
    }//stc loop
    
  }//econt loop
  //===============================================================================================================================
  //STC4
  //===============================================================================================================================

  return true;
}


int main(int argc, char** argv){
  
  //===============================================================================================================================
  // ./emul_econt.exe $Relay $rname $link_number
  //===============================================================================================================================  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return false;
  }
  if(argc < 4){
    std::cerr << argv[1] << ": no link number (1 or 2) is specified " << std::endl;
    return false;
  }
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Assign relay,run and link numbers
  //===============================================================================================================================
  unsigned relayNumber(0);
  unsigned runNumber(0);
  unsigned linkNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  std::istringstream issLink(argv[3]);
  issLink >> linkNumber;
  if(linkNumber!=1 and linkNumber!=2){
    std::cerr << "Link number "<< argv[3] <<"is out of bound (use: 1 or 2)" << std::endl;
    return false;
  }
  int isMSB = 1;
  if(linkNumber==1) isMSB = 0;
  cout <<"isMSB : "<<isMSB << endl;
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  //map<int,tuple<chid,chid,chid,chid>> tctorocch; tctorocch.clear();
  map<int,tuple<int,int,int,int>> tctorocch; tctorocch.clear();
  ReadChannelMapping(tctorocch);
  if(tctorocch.size()==0){
    cerr << "Channel mapping array should be non-zero, but current size  is : " << tctorocch.size() << endl;
    return false;
  }
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read adc pedestal and threshold from yaml module file
  //===============================================================================================================================
  unsigned pedestal_adc[2][3][2][36], threshold_adc[2][3][2][36]; //2links,3chips,2halves,38sequences
  read_Ped_from_yaml(relayNumber, runNumber, 1, pedestal_adc, threshold_adc);
  read_Ped_from_yaml(relayNumber, runNumber, 2, pedestal_adc, threshold_adc);
  //===============================================================================================================================  
  
  //===============================================================================================================================
  //Book histograms
  //===============================================================================================================================
  TFile *fout = new TFile(Form("log/out_stc4_link%d_diff.root",linkNumber),"recreate");
  TDirectory *dir_diff = fout->mkdir("diff_plots");
  dir_diff->cd();
  TH1F *hCompressDiff = new TH1F("hCompressDiff","Difference in (Emulator - ROC) compression", 200, -99, 101);
  hCompressDiff->SetMinimum(1.e-1);
  hCompressDiff->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ROC");
  hCompressDiff->SetLineColor(kMagenta);
  hCompressDiff->SetDirectory(dir_diff);
  TH1F *hCompressDiffECONT = new TH1F("hCompressDiffECONT","Difference in (Emulator - ECONT) compression for totflag==0", 200, -99, 101);
  hCompressDiffECONT->SetMinimum(1.e-1);
  hCompressDiffECONT->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ECONT");
  hCompressDiffECONT->SetLineColor(kRed);
  hCompressDiffECONT->SetDirectory(dir_diff);
  TH1F *hCompressDiffMatchedRaw = new TH1F("hCompressDiffMatchedRaw","Raw Energy for mached cases with totflag==0", 1050, -10, 1040);
  hCompressDiffMatchedRaw->SetMinimum(1.e-1);
  hCompressDiffMatchedRaw->GetXaxis()->SetTitle("Energy");
  hCompressDiffMatchedRaw->SetLineColor(kRed);
  hCompressDiffMatchedRaw->SetDirectory(dir_diff);
  TH1F *hCompressDiffUnMatchedRaw = new TH1F("hCompressDiffUnMatchedRaw","Raw Energy for unmached cases with totflag==0", 1050, -10, 1040);
  hCompressDiffUnMatchedRaw->SetMinimum(1.e-1);
  hCompressDiffUnMatchedRaw->GetXaxis()->SetTitle("Energy");
  hCompressDiffUnMatchedRaw->SetLineColor(kRed);
  hCompressDiffUnMatchedRaw->SetDirectory(dir_diff);
  TH1F *hCompressDiffMatchedEmul = new TH1F("hCompressDiffMatchedEmul","Emul Energy for mached cases with totflag==0", 1050, -10, 1040);
  hCompressDiffMatchedEmul->SetMinimum(1.e-1);
  hCompressDiffMatchedEmul->GetXaxis()->SetTitle("Energy");
  hCompressDiffMatchedEmul->SetLineColor(kRed);
  hCompressDiffMatchedEmul->SetDirectory(dir_diff);
  TH1F *hCompressDiffUnMatchedEmul = new TH1F("hCompressDiffUnMatchedEmul","Emul Energy for unmached cases with totflag==0", 1050, -10, 1040);
  hCompressDiffUnMatchedEmul->SetMinimum(1.e-1);
  hCompressDiffUnMatchedEmul->GetXaxis()->SetTitle("Energy");
  hCompressDiffUnMatchedEmul->SetLineColor(kRed);
  hCompressDiffUnMatchedEmul->SetDirectory(dir_diff);
  TH1F *hMatchedCh = new TH1F("hMatchedCh","Channels energy matched between Raw/Emul with totflag==0", 20, -1.5, 18.5);
  hMatchedCh->SetMinimum(1.e-1);
  hMatchedCh->GetXaxis()->SetTitle("Ch");
  hMatchedCh->SetLineColor(kRed);
  hMatchedCh->SetDirectory(dir_diff);
  TH1F *hUnMatchedCh = new TH1F("hUnMatchedCh","Channels energy unmatched between Raw/Emul with totflag==0", 20, -1.5, 18.5);
  hUnMatchedCh->SetMinimum(1.e-1);
  hUnMatchedCh->GetXaxis()->SetTitle("Ch");
  hUnMatchedCh->SetLineColor(kRed);
  hUnMatchedCh->SetDirectory(dir_diff);
  TH1F *hCompressDiffTOTECONT = new TH1F("hCompressDiffTOTECONT","Difference in (Emulator - ECONT) compression for totflag==3", 200, -99, 101);
  hCompressDiffTOTECONT->SetMinimum(1.e-1);
  hCompressDiffTOTECONT->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ECONT");
  hCompressDiffTOTECONT->SetLineColor(kBlue);
  hCompressDiffTOTECONT->SetDirectory(dir_diff);
  TH1F *hCompressDiffTOTUPECONT = new TH1F("hCompressDiffTOTUPECONT","Difference in (Emulator - ECONT) compression for totflag==3", 200, -99, 101);
  hCompressDiffTOTUPECONT->SetMinimum(1.e-1);
  hCompressDiffTOTUPECONT->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ECONT");
  hCompressDiffTOTUPECONT->SetLineColor(kGreen+2);
  hCompressDiffTOTUPECONT->SetDirectory(dir_diff);
  TH1F *hCompressDiffECONTPD = new TH1F("hCompressDiffECONTPD","Difference in (Emulator - ECONT) compression", 200, -99, 101);
  hCompressDiffECONTPD->SetMinimum(1.e-1);
  hCompressDiffECONTPD->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ECONT");
  hCompressDiffECONTPD->SetLineColor(kBlack);
  hCompressDiffECONTPD->SetDirectory(dir_diff);  
  TH1F *hTCHits = new TH1F("hTCHits","Hit distribution in TCs", 50, -1, 49);
  hTCHits->SetMinimum(1.e-1);
  hTCHits->GetXaxis()->SetTitle("TC");
  hTCHits->GetYaxis()->SetTitle("Nof Hits");
  hTCHits->SetLineColor(kBlue);
  hTCHits->SetDirectory(dir_diff);
  TH1F *hSTCEnergy[48], *hEmulEnergy[48];
  TH1F *hCompressDiffSTCADC[48],*hCompressDiffSTCTOT[48],*hCompressDiffSTCTOTUP[48];
  for(int istc=0;istc<12;istc++){
    hSTCEnergy[istc] = new TH1F(Form("hSTCEnergy_%d",istc),Form("Compressed energy distribution of ECONT for STC : %d",istc), 200, -9, 191);
    hSTCEnergy[istc]->GetXaxis()->SetTitle("ECONT Energy");
    hSTCEnergy[istc]->SetLineColor(kBlack);
    hSTCEnergy[istc]->SetDirectory(dir_diff);
    hEmulEnergy[istc] = new TH1F(Form("hEmulEnergy_%d",istc),Form("Compressed energy distribution using Emulator for STC  : %d",istc), 200, -9, 191);
    hEmulEnergy[istc]->GetXaxis()->SetTitle("Emulator Energy");
    hEmulEnergy[istc]->SetLineColor(kMagenta);
    hEmulEnergy[istc]->SetDirectory(dir_diff);
    hCompressDiffSTCADC[istc] = new TH1F(Form("hCompressDiffSTCADC_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC : %d with totflag==0",istc), 200, -99, 101);
    hCompressDiffSTCADC[istc]->SetMinimum(1.e-1);
    hCompressDiffSTCADC[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
    hCompressDiffSTCADC[istc]->SetLineColor(kRed);
    hCompressDiffSTCADC[istc]->SetDirectory(dir_diff);
    hCompressDiffSTCTOT[istc] = new TH1F(Form("hCompressDiffSTCTOT_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC : %d with totflag==3",istc), 200, -99, 101);
    hCompressDiffSTCTOT[istc]->SetMinimum(1.e-1);
    hCompressDiffSTCTOT[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
    hCompressDiffSTCTOT[istc]->SetLineColor(kBlue);
    hCompressDiffSTCTOT[istc]->SetDirectory(dir_diff);
    hCompressDiffSTCTOTUP[istc] = new TH1F(Form("hCompressDiffSTCTOTUP_%d",istc),Form("Difference in (Emulator - ECONT) compression for STC : %d with totflag==3",istc), 200, -99, 101);
    hCompressDiffSTCTOTUP[istc]->SetMinimum(1.e-1);
    hCompressDiffSTCTOTUP[istc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
    hCompressDiffSTCTOTUP[istc]->SetLineColor(kGreen+2);
    hCompressDiffSTCTOTUP[istc]->SetDirectory(dir_diff);
  }
  TH1F *hModSumDiff = new TH1F("hModSumDiff","Difference in (Emulator - ROC) modsum compression (4E+3M)", 200, -99, 101);
  hModSumDiff->SetMinimum(1.e-1);
  hModSumDiff->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ROC");
  hModSumDiff->SetLineColor(kMagenta);
  hModSumDiff->SetDirectory(dir_diff);
  TH1F *hModSumDiffMissed = new TH1F("hModSumDiffMissed","Difference in (Emulator - ROC) modsum compression (missed,4E+3M)", 200, -99, 101);
  hModSumDiffMissed->SetMinimum(1.e-1);
  hModSumDiffMissed->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ROC");
  hModSumDiffMissed->SetLineColor(kAzure);
  hModSumDiffMissed->SetDirectory(dir_diff);
  TH1F *hModSumDiffADC = new TH1F("hModSumDiffADC","Difference in (Emulator - ROC) modsum compression with totflag==0 (4E+3M)", 200, -99, 101);
  hModSumDiffADC->SetMinimum(1.e-1);
  hModSumDiffADC->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ROC");
  hModSumDiffADC->SetLineColor(kRed);
  hModSumDiffADC->SetDirectory(dir_diff);
  TH1F *hModSumDiffTOT = new TH1F("hModSumDiffTOT","Difference in (Emulator - ROC) modsum compression with totflag==3 (4E+3M)", 200, -99, 101);
  hModSumDiffTOT->SetMinimum(1.e-1);
  hModSumDiffTOT->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ROC");
  hModSumDiffTOT->SetLineColor(kBlue);
  hModSumDiffTOT->SetDirectory(dir_diff);
  TH1F *hModSumDiffTOTUP = new TH1F("hModSumDiffTOTUP","Difference in (Emulator - ROC) modsum compression with totflag==3 (4E+3M)", 200, -99, 101);
  hModSumDiffTOTUP->SetMinimum(1.e-1);
  hModSumDiffTOTUP->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ROC");
  hModSumDiffTOTUP->SetLineColor(kGreen+2);
  hModSumDiffTOTUP->SetDirectory(dir_diff);
  TH1F *hModSumDiff5E3M = new TH1F("hModSumDiff5E3M","Difference in (Emulator - ROC) modsum compression (5E+3M)", 200, -99, 101);
  hModSumDiff5E3M->SetMinimum(1.e-1);
  hModSumDiff5E3M->GetXaxis()->SetTitle("Difference in modsum compressed value : Emulator - ROC");
  hModSumDiff5E3M->SetLineColor(kMagenta);
  hModSumDiff5E3M->SetDirectory(dir_diff);  
  
  TDirectory *dir_adctot = fout->mkdir("adctot_plots");
  dir_adctot->cd();
  int choffset = get<0>(tctorocch[8]) ; //==36, since there are 16 TC per chip
  TH1I *hADC_ch_flag0[3][2][32], *hADC_ch_flag3[3][2][32];
  TH1I *hTOT_ch_flag3[3][2][32];
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int chindex = 0 ;
      for(int ich=0;ich<37;ich++){
	if(ich>=37 or ich==18) continue;
	int ch = (ihalf==1)?(ich+choffset):ich;
    	if(ich>18) ch -= 1;
	int rocch = ch%36;
	ch += 72*ichip;
	for(int itc=0;itc<48;itc++){
	  if(ch==get<0>(tctorocch[itc]) or ch==get<1>(tctorocch[itc]) or ch==get<2>(tctorocch[itc]) or ch==get<3>(tctorocch[itc])){
	    //printf("hADC_ch_flag0_%d_hf-%d_ich-%d_rocch-%d_chindex_%d\n",ichip,ihalf,ich,rocch,chindex);
	    hADC_ch_flag0[ichip][ihalf][chindex] = new TH1I(Form("hADC_ch_flag0_%d_%d_%d",ichip,ihalf,rocch),Form("ADC for chip=%d,half=%d,ch=%d & (Flag=0b00)",ichip,ihalf,rocch),1044,-10,1034);
	    hADC_ch_flag3[ichip][ihalf][chindex] = new TH1I(Form("hADC_ch_flag3_%d_%d_%d",ichip,ihalf,rocch),Form("ADC for chip=%d,half=%d,ch=%d & (Flag=0b11)",ichip,ihalf,rocch),1044,-10,1034);
	    hTOT_ch_flag3[ichip][ihalf][chindex] = new TH1I(Form("hTOT_ch_flag3_%d_%d_%d",ichip,ihalf,rocch),Form("TOT for chip=%d,half=%d,ch=%d & (Flag=0b11)",ichip,ihalf,rocch),1044,-10,1034);
	    ////////////////////////////////////////////////////////////
	    hADC_ch_flag0[ichip][ihalf][chindex]->GetXaxis()->SetTitle("ADC");
	    hADC_ch_flag0[ichip][ihalf][chindex]->GetYaxis()->SetTitle("Entries");
	    hADC_ch_flag3[ichip][ihalf][chindex]->GetXaxis()->SetTitle("ADC");
	    hADC_ch_flag3[ichip][ihalf][chindex]->GetYaxis()->SetTitle("Entries");
	    hTOT_ch_flag3[ichip][ihalf][chindex]->GetXaxis()->SetTitle("TOT");
	    hTOT_ch_flag3[ichip][ihalf][chindex]->GetYaxis()->SetTitle("Entries");
	    hADC_ch_flag0[ichip][ihalf][chindex]->SetDirectory(dir_adctot);
	    hADC_ch_flag3[ichip][ihalf][chindex]->SetDirectory(dir_adctot);
	    hTOT_ch_flag3[ichip][ihalf][chindex]->SetDirectory(dir_adctot);  
	    chindex++;
	  }
	}
      }
    }
  }
  //===============================================================================================================================
  
  
  //===============================================================================================================================
  //Scan the full statistics in multiple loops (adjust the number of events to be processed in each loop according to the available memory)
  //===============================================================================================================================
  uint64_t nofTrigEvents = 0;
  uint64_t nofDAQEvents = 0;
  uint64_t nofMatchedDAQEvents = 0;
  long double nloopEvent = 2e2 ;
  int nloop = TMath::CeilNint(maxEvent/nloopEvent) ;
  if(scanMode) nloop = 1;
  cout <<"nloop : " << nloop << endl;
  uint64_t minEventTrig = 0, maxEventTrig = 0, minEventDAQ = 0, maxEventDAQ = 0 ;
  
  for(int ieloop=0;ieloop<nloop;ieloop++){
  //for(int ieloop=0;ieloop<3;ieloop++){
    
    //===============================================================================================================================
    //Set loop boundaries
    //===============================================================================================================================
    minEventTrig = ieloop*nloopEvent ;
    maxEventTrig = (ieloop+1)*nloopEvent;
    minEventDAQ = (ieloop==0)?minEventTrig:minEventTrig-nloopEvent/10;
    maxEventDAQ = maxEventTrig+nloopEvent/10 ;
    if(scanMode){
      minEventTrig = scanEvent - 1 ;
      maxEventTrig = scanEvent + 1 ;
      minEventDAQ  = scanEvent - 10 ;
      maxEventDAQ =  scanEvent + 10 ;

    }

    printf("iloop : %d, minEventTrig = %lu, maxEventTrig = %lu, minEventDAQ = %lu, maxEventDAQ = %lu\n",ieloop,minEventTrig, maxEventTrig, minEventDAQ, maxEventDAQ);
    
    //===============================================================================================================================
    //Read Link0, Link1/Link2 files
    //===============================================================================================================================
    // map<uint64_t,bcdata> econtarray;
    // read_econt_data_bc(econtarray,relayNumber,runNumber,minEventTrig, maxEventTrig);
    map<uint64_t,stc4data> econtarray;
    read_econt_data_stc4(econtarray,relayNumber,runNumber,minEventTrig, maxEventTrig);
    cout<<"Link0 size : " << econtarray.size() <<endl;
    nofTrigEvents += econtarray.size();
    //for (auto&& p : econtarray) { delete p.second; }
    
    map<uint64_t,vector<daqdata>> rocarray; 
    read_roc_data(rocarray,relayNumber,runNumber,linkNumber,minEventDAQ, maxEventDAQ);
    cout<<"Link"<<linkNumber<<" size : " << rocarray.size() <<endl;
    nofDAQEvents += rocarray.size();    
    //===============================================================================================================================
    
    //===============================================================================================================================
    // Event counting for ECON-T and ECON-D
    //===============================================================================================================================
    uint64_t nofECONTEvents = 0, nofECONDEvents = 0, sizeofROCarray = 0;
    for(const auto& [eventId, econt] : econtarray){    

      if(eventId%2000==0)  cout << "Processing event : "<<eventId<<"  .........."<< endl;
      // //===============================================================================================================================
      // // Fill ADC/TOT histograms
      // //===============================================================================================================================
      // int chindex = 0 ;
      // int prevhalf = 2;
      // for(const auto& roc : rocarray[eventId]){
      // 	if(roc.channel.to_ulong()>=37 or roc.channel.to_ulong()==18) continue;
      // 	//if(roc.totflag.to_ulong()==2) continue;
      // 	int ichip = int(roc.chip.to_ulong());
      // 	int ihalf = int(roc.half.to_ulong());
      // 	if(ihalf!=prevhalf) chindex = 0 ;
      // 	int ich = int(roc.channel.to_ulong());
      // 	int roc_totflag = int(roc.totflag.to_ulong());
      // 	int ch = (ihalf==1)?(ich+choffset):ich;
      // 	if(ich>18) ch -= 1;
      // 	int rocch = ch%36;
      // 	ch += 72*ichip;
      // 	for(int itc=0;itc<48;itc++){
      // 	  if(ch==get<0>(tctorocch[itc]) or ch==get<1>(tctorocch[itc]) or ch==get<2>(tctorocch[itc]) or ch==get<3>(tctorocch[itc])){
      // 	    //printf("hADC_ch_flag0_%d_hf-%d_ich-%d_rocch-%d_chindex_%d\n",ichip,ihalf,ich,rocch,chindex);
      // 	    if(roc_totflag==0) hADC_ch_flag0[ichip][ihalf][chindex]->Fill(int(roc.adc.to_ulong()));
      // 	    if(roc_totflag==3){
      // 	      hADC_ch_flag3[ichip][ihalf][chindex]->Fill(int(roc.adc.to_ulong()));
      // 	      hTOT_ch_flag3[ichip][ihalf][chindex]->Fill(int(roc.tot.to_ulong()));
      // 	    }
      // 	    chindex++;
      // 	  }
      // 	}
      // 	prevhalf = ihalf ;
      // }
      // //===============================================================================================================================
      nofECONTEvents++;
      unsigned rocArraySize = rocarray[eventId].size();
      if(rocArraySize==222){
	nofECONDEvents++; //222 = 37 channel * 3 chips * 2 halves
      }
      sizeofROCarray += rocArraySize ;
    }
    printf("iloop : %d, nofECONTEvents = %lu, nofECONDEvents = %lu, sizeofROCarray = %lu\n", ieloop, nofECONTEvents, nofECONDEvents, sizeofROCarray);
    nofMatchedDAQEvents += nofECONDEvents;
    //===============================================================================================================================
    
    CompareSTC4Energies(econtarray, rocarray, isMSB, tctorocch, pedestal_adc, threshold_adc, dir_diff);
    
    //===============================================================================================================================
    //Clear Link0, Link1/Link2 files
    //===============================================================================================================================
    for (auto& [eventId, roc] : rocarray) { roc.clear(); }
    rocarray.clear();
    econtarray.clear();
    //===============================================================================================================================
    
  }//event loop
  cout<<"Total Link0 size : " << nofTrigEvents <<endl;
  cout<<"Total Link"<<linkNumber<<" size (nofDAQEvents) : " << nofDAQEvents <<endl;
  cout<<"Total Link"<<linkNumber<<" size (nofMatchedDAQEvents) : " << nofMatchedDAQEvents <<endl;

  //===============================================================================================================================
  //Create canvases
  //===============================================================================================================================
  //===============================================================================================================================
  //Diff canvases
  //===============================================================================================================================

  TCanvas *c1_Diff_ADC = new TCanvas(Form("c1_Diff_Comp_ADC"),Form("c1_Diff_Comp_ADC"));
  c1_Diff_ADC->Divide(4,3);
  for(int istc=0;istc<12;istc++){
    c1_Diff_ADC->cd(istc+1)->SetLogy();
    hCompressDiffSTCADC[istc]->Draw();
  }
  TCanvas *c1_Diff_TOT = new TCanvas(Form("c1_Diff_Comp_TOT"),Form("c1_Diff_Comp_TOT"));
  c1_Diff_TOT->Divide(4,3);
  for(int istc=0;istc<12;istc++){
    c1_Diff_TOT->cd(istc+1)->SetLogy();
    hCompressDiffSTCTOT[istc]->Draw();
  }
  TCanvas *c1_Diff_TOTUP = new TCanvas(Form("c1_Diff_Comp_TOTUP"),Form("c1_Diff_Comp_TOTUP"));
  c1_Diff_TOTUP->Divide(4,3);
  for(int istc=0;istc<12;istc++){
    c1_Diff_TOTUP->cd(istc+1)->SetLogy();
    hCompressDiffSTCTOTUP[istc]->Draw();
  }

  //===============================================================================================================================
  //ADC/TOT canvases
  //===============================================================================================================================

  TCanvas *c1_ADC_flag0[3][2],*c1_ADC_flag3[3][2];
  TCanvas *c1_TOT_flag3[3][2];
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_ADC_flag0[ichip][ihalf] = new TCanvas(Form("canvas_ADC_flag0_%d_%d",ichip,ihalf));
      c1_ADC_flag0[ichip][ihalf]->Divide(8,4);
      for(int ich=0;ich<32;ich++){
  	c1_ADC_flag0[ichip][ihalf]->cd(ich+1)->SetLogy();
  	hADC_ch_flag0[ichip][ihalf][ich]->Draw();
      }
    }
  }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_ADC_flag3[ichip][ihalf] = new TCanvas(Form("canvas_ADC_flag3_%d_%d",ichip,ihalf));
      c1_ADC_flag3[ichip][ihalf]->Divide(8,4);
      for(int ich=0;ich<32;ich++){
  	c1_ADC_flag3[ichip][ihalf]->cd(ich+1)->SetLogy();
  	hADC_ch_flag3[ichip][ihalf][ich]->Draw();
      }
    }
  }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_TOT_flag3[ichip][ihalf] = new TCanvas(Form("canvas_TOT_flag3_%d_%d",ichip,ihalf));
      c1_TOT_flag3[ichip][ihalf]->Divide(8,4);
      for(int ich=0;ich<32;ich++){
  	c1_TOT_flag3[ichip][ihalf]->cd(ich+1)->SetLogy();
  	hTOT_ch_flag3[ichip][ihalf][ich]->Draw();
      }
    }
  }

  //===============================================================================================================================

  //===============================================================================================================================
  //Save histograms
  //===============================================================================================================================
  fout->cd();
  c1_Diff_ADC->Write();
  c1_Diff_TOT->Write();
  c1_Diff_TOTUP->Write();

  dir_diff->Write();
  dir_adctot->Write();
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_ADC_flag0[ichip][ihalf]->Write();
      c1_ADC_flag3[ichip][ihalf]->Write();
      c1_TOT_flag3[ichip][ihalf]->Write();
    }
  }
  fout->Close();
  delete fout;
  //===============================================================================================================================
  
  // //===============================================================================================================================
  // //Pedestal and threshold histograms
  // //===============================================================================================================================
  // TH1F *hPedADC_LSB[3][2], *hThreshADC_LSB[3][2];
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     hPedADC_LSB[ichip][ihalf] = new TH1F(Form("hPedADC_LSB_%d_%d",ichip,ihalf),Form("hPedADC_LSB_%d_%d",ichip,ihalf),40,-1,39);
  //     hThreshADC_LSB[ichip][ihalf] = new TH1F(Form("hThreshADC_LSB_%d_%d",ichip,ihalf),Form("hThreshADC_LSB_%d_%d",ichip,ihalf),40,-1,39);
  //     for(int ichannel=0;ichannel<36;ichannel++) {
  // 	hPedADC_LSB[ichip][ihalf]->SetBinContent(hPedADC_LSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), float(pedestal_adc[0][ichip][ihalf][ichannel]));
  // 	hThreshADC_LSB[ichip][ihalf]->SetBinContent(hThreshADC_LSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), float(threshold_adc[0][ichip][ihalf][ichannel]));
  //     }
  //     hPedADC_LSB[ichip][ihalf]->GetXaxis()->SetTitle("Channel");
  //     hPedADC_LSB[ichip][ihalf]->GetYaxis()->SetTitle("Pedestal");
  //     hThreshADC_LSB[ichip][ihalf]->GetXaxis()->SetTitle("Channel");
  //     hThreshADC_LSB[ichip][ihalf]->GetYaxis()->SetTitle("Threshold");
  //   }
  // }
  
  // TH1F *hPedADC_MSB[3][2], *hThreshADC_MSB[3][2];
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     hPedADC_MSB[ichip][ihalf] = new TH1F(Form("hPedADC_MSB_%d_%d",ichip,ihalf),Form("hPedADC_MSB_%d_%d",ichip,ihalf),40,-1,39);
  //     hThreshADC_MSB[ichip][ihalf] = new TH1F(Form("hThreshADC_MSB_%d_%d",ichip,ihalf),Form("hThreshADC_MSB_%d_%d",ichip,ihalf),40,-1,39);
  //     for(int ichannel=0;ichannel<36;ichannel++) {
  // 	hPedADC_MSB[ichip][ihalf]->SetBinContent(hPedADC_MSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), float(pedestal_adc[1][ichip][ihalf][ichannel]));
  // 	hThreshADC_MSB[ichip][ihalf]->SetBinContent(hThreshADC_MSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), float(threshold_adc[1][ichip][ihalf][ichannel]));
  //     }
  //     hPedADC_MSB[ichip][ihalf]->GetXaxis()->SetTitle("Channel");
  //     hPedADC_MSB[ichip][ihalf]->GetYaxis()->SetTitle("Pedestal");
  //     hThreshADC_MSB[ichip][ihalf]->GetXaxis()->SetTitle("Channel");
  //     hThreshADC_MSB[ichip][ihalf]->GetYaxis()->SetTitle("Threshold");
  //   }
  // }
  // ////////////////////////////////////////////////////////////////////////////////////////////////
  // TCanvas *c1_ped_adc_lsb = new TCanvas("c1_ped_adc_lsb","c1_ped_adc_lsb",1200,800);
  // c1_ped_adc_lsb->Divide(3,2);
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     int icanvas = 2*ichip + ihalf + 1;
  //     c1_ped_adc_lsb->cd(icanvas);
  //     hPedADC_LSB[ichip][ihalf]->Draw();
  //   }
  // }
  
  // TCanvas *c2_thresh_adc_lsb = new TCanvas("c2_thresh_adc_lsb","c2_thresh_adc_lsb",1200,800);
  // c2_thresh_adc_lsb->Divide(3,2);
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     int icanvas = 2*ichip + ihalf + 1;
  //     c2_thresh_adc_lsb->cd(icanvas);
  //     hThreshADC_LSB[ichip][ihalf]->Draw();
  //   }
  // }
  // ////////////////////////////////////////////////////////////////////////////////////////////////
  // TCanvas *c1_ped_adc_msb = new TCanvas("c1_ped_adc_msb","c1_ped_adc_msb",1200,800);
  // c1_ped_adc_msb->Divide(3,2);
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     int icanvas = 2*ichip + ihalf + 1;
  //     c1_ped_adc_msb->cd(icanvas);
  //     hPedADC_MSB[ichip][ihalf]->Draw();
  //   }
  // }
  
  // TCanvas *c2_thresh_adc_msb = new TCanvas("c2_thresh_adc_msb","c2_thresh_adc_msb",1200,800);
  // c2_thresh_adc_msb->Divide(3,2);
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     int icanvas = 2*ichip + ihalf + 1;
  //     c2_thresh_adc_msb->cd(icanvas);
  //     hThreshADC_MSB[ichip][ihalf]->Draw();
  //   }
  // }


  // TFile *fcanva_out = new TFile(Form("log/Pedestal_bc_link%d.root",linkNumber),"recreate");
  // c1_ped_adc_lsb->Write();
  // c2_thresh_adc_lsb->Write();
  // c1_ped_adc_msb->Write();
  // c2_thresh_adc_msb->Write();
  // fcanva_out->Close();
  // delete fcanva_out;

  // //===============================================================================================================================
  
  return true;
}
