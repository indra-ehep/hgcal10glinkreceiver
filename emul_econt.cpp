#include <iostream>
#include <bitset>

#include "TH1D.h"
#include "TH2D.h"
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

const uint64_t maxEvent = 1e6; //6e5

typedef struct{
  bitset<2> chip;
  bitset<1> half;
  bitset<6> channel;
  bitset<2> totflag;
  bitset<10> adc;
  //bitset<10> adcm;
  bitset<12> tot;
} daqdata;

typedef struct{
  uint8_t modsum[2];
  uint8_t energy_raw[2][9];                    
  uint8_t loc_raw[2][9]; 
} bcdata;



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

int read_roc_data(map<uint64_t,vector<daqdata>>& rocarray, unsigned relayNumber, unsigned runNumber, unsigned linkNumber)
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

      if (nEvents < 4){ 
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
      //Increment event counter
      nEvents++;
      
      int isMSB[6];
      int ffsep_word_loc[6];
      bool is_ff_sep_notfound = false;
      for(int iloc=0;iloc<6;iloc++){
      	isMSB[iloc] = -1;
      	ffsep_word_loc[iloc] = find_ffsep_word(rEvent, isMSB[iloc], iloc+1);
      	if(isMSB[iloc] == -1) is_ff_sep_notfound = true;
	if (nEvents < 5){ 
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
      if (nEvents < 5)
	cout << "Event : " << nEvents << ": 0x0000 0x0000 closing separator word at location : " << expctd_empty_word_loc << " hasfound_emptry_word : "<< hasfound_emptry_word << ",  and isMSB : " << empty_isMSB << endl;
      
      if(!hasfound_emptry_word) continue;
      if(is_ff_sep_notfound) continue;
      if(nEvents>maxEvent) continue;
      
      int ichip = 0;
      const uint64_t *p64(((const uint64_t*)rEvent)+1);
      
      uint32_t wordH = ((p64[ffsep_word_loc[0]-1] >> 32) & 0xFFFFFFFF) ;
      int daq_header_payload_length = int(((wordH>>14) & 0x1FF));
      daq_header_payload_length -= 1 ; //1 for DAQ header
      if (nEvents < 5){
	std::cout << std::hex << std::setfill('0');
	cout << "Event : " << nEvents << ", WordH : 0x" << wordH  ;
	std::cout << std::dec << std::setfill(' ');
	cout<<", Size in ECON-D header : " << daq_header_payload_length << endl;
      }
      
      int record_header_sizeinfo = int(rEvent->payloadLength()); //in 64-bit format
      int reduced_record_header_sizeinfo = record_header_sizeinfo - (4+2+1) ; //exclude 4 64-bit words for slink header/trailer + 2 DAQ header + 1 for DAQ trailer
      int record_header_sizeinfo_32bit = 2*reduced_record_header_sizeinfo ; //change to 32-bit format

      if(record_header_sizeinfo_32bit!=daq_header_payload_length) continue;
      
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

	  if (nEvents < 4){ 
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
  }//file reader

  _fileReader.close();
  return true;
}


int read_econt_data_bc(map<uint64_t,bcdata>& econtarray, unsigned relayNumber, unsigned runNumber)
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
  
  bcdata edata;
  uint64_t nEvents = 0;
  uint64_t prevEvent = 0;
  uint32_t prevSequence = 0;
  uint32_t packet[4];
  uint32_t packet_counter;
  bool packet_tcs[48];
  uint64_t packet_energies[9];

  uint16_t daq_data[5];                              
  uint16_t daq_nbx[5];                               
  uint16_t size_in_cafe[5];                          
  uint32_t bx_raw[2][15];                        
  int refbxindex = 10;
  
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

      daq_data[2] = p64[2]>>14 & 0xF;
      daq_nbx[2] = p64[2]>>18 & 0x7;
      uint64_t daq2_event_size = (2*daq_nbx[2] + 1)*daq_data[2];
      int third_cafe_word_loc = find_cafe_word(rEvent, 3);
      size_in_cafe[2] = p64[third_cafe_word_loc] & 0xFF;

      int sixth_cafe_word_loc = find_cafe_word(rEvent, 6);
      
      if(sixth_cafe_word_loc!=0) continue;
      if(first_cafe_word_loc != 3) continue;
      if(daq0_event_size != size_in_cafe[0]) continue;	
      if(daq1_event_size != size_in_cafe[1]) continue;
      if(daq2_event_size != size_in_cafe[2]) continue;	

      if (nEvents < 2) {
	event_dump(rEvent);
	rEvent->RecordHeader::print();
      	boe->print();
      	eoe->print();
      }
      if(nEvents>maxEvent) continue;
      
      int bx_index = -1.0*int(daq_nbx[0]);
      const int maxnbx = (2*daq_nbx[0] + 1);
      uint64_t energy_raw[2][maxnbx][9];
      for(int iect=0;iect<2;iect++)
  	for(int ibx=0;ibx<maxnbx;ibx++){
	  bx_raw[iect][ibx] = 0;
  	  for(int istc=0;istc<9;istc++){
  	    energy_raw[iect][ibx][istc] = 0;
  	    edata.energy_raw[iect][istc] = 0;
  	    edata.loc_raw[iect][istc] = 0;
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
		   <<", edata.bxId : "<<eoe->bxId() <<", modulo8 : "<< (eoe->bxId()%8)
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

	bx_raw[0][(bx_index+int(daq_nbx[0]))] = bx_counter;
	for(int itc=0;itc<9;itc++){	  
	  energy_raw[0][(bx_index+int(daq_nbx[0]))][itc] = packet_energies[itc];
	}
	int itrigcell = 0;
	if((bx_index+int(daq_nbx[0]))==refbxindex){
	  edata.modsum[0] = modsum;
	  for(int itc=0;itc<9;itc++)
	    edata.energy_raw[0][itc] = packet_energies[itc];
	  for(int itc=0;itc<48;itc++)
	    if(packet_tcs[itc]) edata.loc_raw[0][itrigcell++] = itc;
	}
	
	
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
		   <<", edata.bxId : "<<eoe->bxId() <<", modulo8 : "<< (eoe->bxId()%8)
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

	bx_raw[1][(bx_index+int(daq_nbx[0]))] = bx_counter1;
	for(int itc=0;itc<9;itc++){	  
	  energy_raw[1][(bx_index+int(daq_nbx[0]))][itc] = packet_energies[itc];
	}
	itrigcell = 0;
	if((bx_index+int(daq_nbx[0]))==refbxindex){
	  edata.modsum[1] = modsum1;
	  for(int itc=0;itc<9;itc++)
	    edata.energy_raw[1][itc] = packet_energies[itc];
	  for(int itc=0;itc<48;itc++)
	    if(packet_tcs[itc]) edata.loc_raw[1][itrigcell++] = itc;
	}
	
      	bx_index++;
      }
      
      //econtarray.push_back(edata);
      econtarray[boe->eventId()] = edata;
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
  ifstream inwafermap("/home/indra/Downloads/WaferCellMapTraces.txt");
  //ifstream inwafermap("/home/hep/idas/codes/WaferCellMapTraces.txt");
  //ifstream inwafermap("/afs/cern.ch/user/i/idas/Downloads/WaferCellMapTraces.txt");
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


int main(int argc, char** argv){
  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return 1;
  }
  
  //Assign relay and run numbers
  unsigned linkNumber(0);
  bool skipMSB(true);
  
  // ./emul_econt.exe $Relay $rname
  unsigned relayNumber(0);
  unsigned runNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  
  //===============================================================================================================================
  //Read Link0, Link1 and Link2 files
  //===============================================================================================================================
  // vector<econtdata> econtarray; econtarray.clear();
  // //read_econt_data_stc4(econtarray,relayNumber,runNumber);
  // read_econt_data_bc(econtarray,relayNumber,runNumber);
  // cout<<"Link0 size : " << econtarray.size() <<endl;
  
  // vector<rocdata> rocarray1; rocarray1.clear();
  // read_roc_data(rocarray1,relayNumber,runNumber,1);
  // cout<<"Link1 size : " << rocarray1.size() <<endl;
  
  // vector<rocdata> rocarray2; rocarray2.clear();
  // read_roc_data(rocarray2,relayNumber,runNumber,2);
  // cout<<"Link2 size : " << rocarray2.size() <<endl;

  map<uint64_t,bcdata> econtarray; econtarray.clear();
  read_econt_data_bc(econtarray,relayNumber,runNumber);
  cout<<"Link0 size : " << econtarray.size() <<endl;

  // map<uint64_t,vector<daqdata>> rocarray1; rocarray1.clear();
  // read_roc_data(rocarray1,relayNumber,runNumber,1);
  // cout<<"Link1 size : " << rocarray1.size() <<endl;
  
  map<uint64_t,vector<daqdata>> rocarray2; rocarray2.clear();
  read_roc_data(rocarray2,relayNumber,runNumber,2);
  cout<<"Link2 size : " << rocarray2.size() <<endl;

  //===============================================================================================================================
  //Reading complete for Link0, Link1 and Link2 files
  //===============================================================================================================================
  
  //===============================================================================================================================
  //Read channel mapping
  //===============================================================================================================================
  //map<int,tuple<chid,chid,chid,chid>> tctorocch; tctorocch.clear();
  map<int,tuple<int,int,int,int>> tctorocch; tctorocch.clear();
  ReadChannelMapping(tctorocch);

  //===============================================================================================================================
  //Read adc pedestal and threshold from yaml module file
  //===============================================================================================================================
  unsigned pedestal_adc[2][3][2][36], threshold_adc[2][3][2][36]; //2links,3chips,2halves,38sequences
  read_Ped_from_yaml(relayNumber, runNumber, 1, pedestal_adc, threshold_adc);
  read_Ped_from_yaml(relayNumber, runNumber, 2, pedestal_adc, threshold_adc);
  
  // //===============================================================================================================================
  // //Print the information as read from Link0, Link1 and Link2.
  // //===============================================================================================================================
  // TH1I *hTOTFlag_0 = new TH1I("hTOTFlag_0","TOTFlag==0b00",40,-1,39);
  // TH1I *hTOTFlag_1 = new TH1I("hTOTFlag_1","TOTFlag==0b01",40,-1,39);
  // TH1I *hTOTFlag_2 = new TH1I("hTOTFlag_2","TOTFlag==0b10",40,-1,39);
  // TH1I *hTOTFlag_3 = new TH1I("hTOTFlag_3","TOTFlag==0b11",40,-1,39);
  // TH1I *hADC_ch0_flag0 = new TH1I("hADC_ch0_flag0","ADC for chip==0 and half==0 and ch==0 and (Flag==0b00)",1044,-10,1034);
  // TH1I *hADC_ch0_flag1 = new TH1I("hADC_ch0_flag1","ADC for chip==0 and half==0 and ch==0 and (Flag==0b01)",1044,-10,1034);
  // TH1I *hADC_ch0_flag2 = new TH1I("hADC_ch0_flag2","ADC for chip==0 and half==0 and ch==0 and (Flag==0b10)",1044,-10,1034);
  // TH1I *hADC_ch0_flag3 = new TH1I("hADC_ch0_flag3","ADC for chip==0 and half==0 and ch==0 and (Flag==0b11)",1044,-10,1034);
  // TH1I *hADCM_ch0_flag0 = new TH1I("hADCM_ch0_flag0","ADCM for chip==0 and half==0 and ch==0 and (Flag==0b00)",1044,-10,1034);
  // TH1I *hADCM_ch0_flag1 = new TH1I("hADCM_ch0_flag1","ADCM for chip==0 and half==0 and ch==0 and (Flag==0b01)",1044,-10,1034);
  // TH1I *hADCM_ch0_flag2 = new TH1I("hADCM_ch0_flag2","ADCM for chip==0 and half==0 and ch==0 and (Flag==0b10)",1044,-10,1034);
  // TH1I *hADCM_ch0_flag3 = new TH1I("hADCM_ch0_flag3","ADCM for chip==0 and half==0 and ch==0 and (Flag==0b11)",1044,-10,1034);
  // TH1I *hTOA_ch0_flag0 = new TH1I("hTOA_ch0_flag0","TOA for chip==0 and half==0 and ch==0 and (Flag==0b00)",1044,-10,1034);
  // TH1I *hTOA_ch0_flag1 = new TH1I("hTOA_ch0_flag1","TOA for chip==0 and half==0 and ch==0 and (Flag==0b01)",1044,-10,1034);
  // TH1I *hTOA_ch0_flag2 = new TH1I("hTOA_ch0_flag2","TOA for chip==0 and half==0 and ch==0 and (Flag==0b10)",1044,-10,1034);
  // TH1I *hTOA_ch0_flag3 = new TH1I("hTOA_ch0_flag3","TOA for chip==0 and half==0 and ch==0 and (Flag==0b11)",1044,-10,1034);
  // TH1I *hTOT_ch0_flag0 = new TH1I("hTOT_ch0_flag0","TOT for chip==0 and half==0 and ch==0 and (Flag==0b00)",1044,-10,1034);
  // TH1I *hTOT_ch0_flag1 = new TH1I("hTOT_ch0_flag1","TOT for chip==0 and half==0 and ch==0 and (Flag==0b01)",1044,-10,1034);
  // TH1I *hTOT_ch0_flag2 = new TH1I("hTOT_ch0_flag2","TOT for chip==0 and half==0 and ch==0 and (Flag==0b10)",1044,-10,1034);
  // TH1I *hTOT_ch0_flag3 = new TH1I("hTOT_ch0_flag3","TOT for chip==0 and half==0 and ch==0 and (Flag==0b11)",1044,-10,1034);

  // TH1I *hADC_ch1_flag0 = new TH1I("hADC_ch1_flag0","ADC for chip==0 and half==0 and ch==1 and (Flag==0b00)",1044,-10,1034);
  // TH1I *hADC_ch2_flag0 = new TH1I("hADC_ch2_flag0","ADC for chip==0 and half==0 and ch==2 and (Flag==0b00)",1044,-10,1034);
  // TH1I *hADC_ch3_flag0 = new TH1I("hADC_ch3_flag0","ADC for chip==0 and half==0 and ch==3 and (Flag==0b00)",1044,-10,1034);
  
  // for(const auto& roc : rocarray1){
  //   if(roc.eventId<=2)
  //     printf("\troc Event : : %05d, Chip : %02d, Half : %02d, Channel : %02d, ADC : %5d, TOA : %5d, TOT : %5d, TOTflag : %2d, BXCounter : %d, EventCounter : %d, OrbitCounter : %d\n",
  // 	     roc.event, roc.chip, roc.half, roc.channel, roc.adc, roc.toa, roc.tot, roc.totflag, roc.bxcounter, roc.eventcounter, roc.orbitcounter);
  //   if(roc.totflag==0) hTOTFlag_0->Fill(roc.channel);
  //   if(roc.totflag==1) hTOTFlag_1->Fill(roc.channel);
  //   if(roc.totflag==2) hTOTFlag_2->Fill(roc.channel);
  //   if(roc.totflag==3) hTOTFlag_3->Fill(roc.channel);
  // }
  TH1I *hADC_ch_flag0[3][2][36], *hADC_ch_flag3[3][2][36];
  // TH1I *hADCM_ch_flag0[3][2][36], *hADCM_ch_flag3[3][2][36];
  TH1I *hTOT_ch_flag3[3][2][36];
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      for(int ich=0;ich<36;ich++){	
  	hADC_ch_flag0[ichip][ihalf][ich] = new TH1I(Form("hADC_ch_flag0_%d_%d_%d",ichip,ihalf,ich),Form("ADC for chip=%d,half==%d,ch==%d & (Flag==0b00)",ichip,ihalf,ich),1044,-10,1034);
  	hADC_ch_flag3[ichip][ihalf][ich] = new TH1I(Form("hADC_ch_flag3_%d_%d_%d",ichip,ihalf,ich),Form("ADC for chip=%d,half==%d,ch==%d & (Flag==0b11)",ichip,ihalf,ich),1044,-10,1034);
  	// hADCM_ch_flag0[ichip][ihalf][ich] = new TH1I(Form("hADCM_ch_flag0_%d_%d_%d",ichip,ihalf,ich),Form("ADCM for chip=%d,half==%d,ch==%d & (Flag==0b00)",ichip,ihalf,ich),1044,-10,1034);
  	// hADCM_ch_flag3[ichip][ihalf][ich] = new TH1I(Form("hADCM_ch_flag3_%d_%d_%d",ichip,ihalf,ich),Form("ADCM for chip=%d,half==%d,ch==%d & (Flag==0b11)",ichip,ihalf,ich),1044,-10,1034);
	hTOT_ch_flag3[ichip][ihalf][ich] = new TH1I(Form("hTOT_ch_flag3_%d_%d_%d",ichip,ihalf,ich),Form("TOT for chip=%d,half==%d,ch==%d & (Flag==0b11)",ichip,ihalf,ich),1044,-10,1034);
  	////////////////////////////////////////////////////////////
  	hADC_ch_flag0[ichip][ihalf][ich]->GetXaxis()->SetTitle("ADC");
  	hADC_ch_flag0[ichip][ihalf][ich]->GetYaxis()->SetTitle("Entries");
  	hADC_ch_flag3[ichip][ihalf][ich]->GetXaxis()->SetTitle("ADC");
  	hADC_ch_flag3[ichip][ihalf][ich]->GetYaxis()->SetTitle("Entries");
  	// hADCM_ch_flag0[ichip][ihalf][ich]->GetXaxis()->SetTitle("ADCM");
  	// hADCM_ch_flag0[ichip][ihalf][ich]->GetYaxis()->SetTitle("Entries");
  	// hADCM_ch_flag3[ichip][ihalf][ich]->GetXaxis()->SetTitle("ADCM");
  	// hADCM_ch_flag3[ichip][ihalf][ich]->GetYaxis()->SetTitle("Entries");
  	hTOT_ch_flag3[ichip][ihalf][ich]->GetXaxis()->SetTitle("TOT");
  	hTOT_ch_flag3[ichip][ihalf][ich]->GetYaxis()->SetTitle("Entries");
      }
    }
  }
  
  for(const auto& [eventId, rocarray] : rocarray2){
    //if(eventId<10){
    if(eventId<maxEvent){
      for(const auto& roc : rocarray){
  	//
  	if(roc.channel.to_ulong()>=37 or roc.channel.to_ulong()==18) continue;
  	int roc_chip = int(roc.chip.to_ulong());
    	int roc_half = int(roc.half.to_ulong());
    	int roc_ch = int(roc.channel.to_ulong());
  	int roc_adc = int(roc.adc.to_ulong());
  	//int roc_adcm = int(roc.adcm.to_ulong());
	int roc_tot = int(roc.tot.to_ulong());
    	int roc_totflag = int(roc.totflag.to_ulong());
  	if(roc_ch>18) roc_ch -= 1;
  	//printf("\troc Event : : %05d, Chip : %02d, Half : %02d, Channel : %02d, ADC : %5d, ADCM : %5d, TOTflag : %2d\n", eventId, roc_chip, roc_half, roc_ch, roc_adc, roc_adcm, roc_totflag);
  	if(roc_totflag==0){
  	  hADC_ch_flag0[roc_chip][roc_half][roc_ch]->Fill(roc_adc);
  	  //hADCM_ch_flag0[roc_chip][roc_half][roc_ch]->Fill(roc_adcm);
  	}else if(roc_totflag==3){
  	  hADC_ch_flag3[roc_chip][roc_half][roc_ch]->Fill(roc_adc);
  	  //hADCM_ch_flag3[roc_chip][roc_half][roc_ch]->Fill(roc_adcm);
	  hTOT_ch_flag3[roc_chip][roc_half][roc_ch]->Fill(roc_tot);
  	}
      }
    }
  }
  
  TCanvas *c1_ADC_flag0[3][2],*c1_ADC_flag3[3][2];
  //TCanvas *c1_ADCM_flag0[3][2],*c1_ADCM_flag3[3][2];
  TCanvas *c1_TOT_flag3[3][2];
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_ADC_flag0[ichip][ihalf] = new TCanvas(Form("canvas_ADC_flag0_%d_%d",ichip,ihalf));
      c1_ADC_flag0[ichip][ihalf]->Divide(9,4);
      for(int ich=0;ich<36;ich++){
  	c1_ADC_flag0[ichip][ihalf]->cd(ich+1)->SetLogy();
  	hADC_ch_flag0[ichip][ihalf][ich]->Draw();
      }
    }
  }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_ADC_flag3[ichip][ihalf] = new TCanvas(Form("canvas_ADC_flag3_%d_%d",ichip,ihalf));
      c1_ADC_flag3[ichip][ihalf]->Divide(9,4);
      for(int ich=0;ich<36;ich++){
  	c1_ADC_flag3[ichip][ihalf]->cd(ich+1)->SetLogy();
  	hADC_ch_flag3[ichip][ihalf][ich]->Draw();
      }
    }
  }
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     c1_ADCM_flag0[ichip][ihalf] = new TCanvas(Form("canvas_ADCM_flag0_%d_%d",ichip,ihalf));
  //     c1_ADCM_flag0[ichip][ihalf]->Divide(9,4);
  //     for(int ich=0;ich<36;ich++){
  // 	c1_ADCM_flag0[ichip][ihalf]->cd(ich+1)->SetLogy();
  // 	hADCM_ch_flag0[ichip][ihalf][ich]->Draw();
  //     }
  //   }
  // }
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     c1_ADCM_flag3[ichip][ihalf] = new TCanvas(Form("canvas_ADCM_flag3_%d_%d",ichip,ihalf));
  //     c1_ADCM_flag3[ichip][ihalf]->Divide(9,4);
  //     for(int ich=0;ich<36;ich++){
  // 	c1_ADCM_flag3[ichip][ihalf]->cd(ich+1)->SetLogy();
  // 	hADCM_ch_flag3[ichip][ihalf][ich]->Draw();
  //     }
  //   }
  // }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_TOT_flag3[ichip][ihalf] = new TCanvas(Form("canvas_TOT_flag3_%d_%d",ichip,ihalf));
      c1_TOT_flag3[ichip][ihalf]->Divide(9,4);
      for(int ich=0;ich<36;ich++){
  	c1_TOT_flag3[ichip][ihalf]->cd(ich+1)->SetLogy();
  	hTOT_ch_flag3[ichip][ihalf][ich]->Draw();
      }
    }
  }
  TFile *fChDist = new TFile("log/ADCDist.root","recreate");
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_ADC_flag0[ichip][ihalf]->Write();
    }
  }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_ADC_flag3[ichip][ihalf]->Write();
    }
  }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      c1_TOT_flag3[ichip][ihalf]->Write();
    }
  }
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     c1_ADCM_flag0[ichip][ihalf]->Write();
  //     c1_ADCM_flag3[ichip][ihalf]->Write();
  //   }
  // }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      for(int ich=0;ich<36;ich++){	
  	hADC_ch_flag0[ichip][ihalf][ich]->Write();
      }
    }
  }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      for(int ich=0;ich<36;ich++){	
  	hTOT_ch_flag3[ichip][ihalf][ich]->Write();
      }
    }
  }
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      for(int ich=0;ich<36;ich++){	
  	hADC_ch_flag3[ichip][ihalf][ich]->Write();
      }
    }
  }
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     for(int ich=0;ich<36;ich++){	
  // 	hADCM_ch_flag0[ichip][ihalf][ich]->Write();
  //     }
  //   }
  // }
  // for(int ichip=0;ichip<3;ichip++){
  //   for(int ihalf=0;ihalf<2;ihalf++){
  //     for(int ich=0;ich<36;ich++){	
  // 	hADCM_ch_flag3[ichip][ihalf][ich]->Write();
  //     }
  //   }
  // }
  fChDist->Close();
  delete fChDist;
  
  // return true;
  
  // for(const auto& econt : econtarray){
  //   if(econt.event<2){
  //     printf("\tecont Event : : %05d, bxId : %u, bxId_8_modulo : %u, Energy_STC[0][7][0] : %07d, Energy_STC[1][7][0] : %07d, Loc[0][7][0] : %2d\n",
  // 	     econt.event, econt.bxId, econt.bxId%8, econt.energy_raw[0][7][0], econt.energy_raw[1][7][0], econt.loc_raw[0][7][0]);
  //   }
  // }
  
  // for(const auto& roc : rocarray1){
  //   if(roc.eventId<0)
  //     printf("\troc Event : : %05d, Chip : %02d, Half : %02d, Channel : %02d, ADCM : %5d, ADC : %5d, TOA : %5d, TOT : %5d, TOTflag : %2d, BXCounter : %d, EventCounter : %d, OrbitCounter : %d\n",
  // 	     roc.event, roc.chip, roc.half, roc.channel, roc.adcm, roc.adc, roc.toa, roc.tot, roc.totflag, roc.bxcounter, roc.eventcounter, roc.orbitcounter);
  //   if(roc.totflag==0) hTOTFlag_0->Fill(roc.channel);
  //   if(roc.totflag==1) hTOTFlag_1->Fill(roc.channel);
  //   if(roc.totflag==2) hTOTFlag_2->Fill(roc.channel);
  //   if(roc.totflag==3) hTOTFlag_3->Fill(roc.channel);
  //   if(roc.chip==0 and roc.half==0 and roc.channel==0 and roc.totflag==0){
  //     hADC_ch0_flag0->Fill(roc.adc);
  //     hADCM_ch0_flag0->Fill(roc.adcm);
  //     hTOA_ch0_flag0->Fill(roc.toa);
  //     hTOT_ch0_flag0->Fill(roc.tot);
  //   }
  //   if(roc.chip==0 and roc.half==0 and roc.channel==1 and roc.totflag==0) hADC_ch1_flag0->Fill(roc.adc);
  //   if(roc.chip==0 and roc.half==0 and roc.channel==2 and roc.totflag==0) hADC_ch2_flag0->Fill(roc.adc);
  //   if(roc.chip==0 and roc.half==0 and roc.channel==3 and roc.totflag==0) hADC_ch3_flag0->Fill(roc.adc);

  //   if(roc.chip==0 and roc.half==0 and roc.channel==0 and roc.totflag==1){
  //     hADC_ch0_flag1->Fill(roc.adc);
  //     hADCM_ch0_flag1->Fill(roc.adcm);
  //     hTOA_ch0_flag1->Fill(roc.toa);
  //     hTOT_ch0_flag1->Fill(roc.tot);
  //   }
  //   if(roc.chip==0 and roc.half==0 and roc.channel==0 and roc.totflag==2){
  //     hADC_ch0_flag2->Fill(roc.adc);
  //     hADCM_ch0_flag2->Fill(roc.adcm);
  //     hTOA_ch0_flag2->Fill(roc.toa);
  //     hTOT_ch0_flag2->Fill(roc.tot);
  //   }
  //   if(roc.chip==0 and roc.half==0 and roc.channel==0 and roc.totflag==3){
  //     hADC_ch0_flag3->Fill(roc.adc);
  //     hADCM_ch0_flag3->Fill(roc.adcm);
  //     hTOA_ch0_flag3->Fill(roc.toa);
  //     hTOT_ch0_flag3->Fill(roc.tot);
  //   }
  // }
  // for(const auto& [eventId, daqarray] : rocarray1){
  //   if(eventId<3){
  //     for(const auto& roc : daqarray)
  // 	printf("\troc Event : : %05d, Chip : %02d, Half : %02d, Channel : %02d, ADC : %5d, TOT : %5d, TOTflag : %2d\n", eventId, roc.chip, roc.half, roc.channel, roc.adc, roc.tot, roc.totflag);
  //   }
  // }
  for(const auto& [eventId, econt] : econtarray){
    if(eventId<10){
      printf("\tecont Event : : %05d, Energy[0][0] : %07d, Energy[1][0] : %07d, Loc[0][0] : %2d\n",eventId, econt.energy_raw[0][0], econt.energy_raw[1][0], econt.loc_raw[0][0]);
    }
  }

  // for(const auto& econt : econtarray){
  //   if(econt.event<2){
  //     printf("\tecont Event : : %05d, bxId : %u, bxId_8_modulo : %u, Energy_STC[0][7][0] : %07d, Energy_STC[1][7][0] : %07d, Loc[0][7][0] : %2d\n",
  // 	     econt.event, econt.bxId, econt.bxId%8, econt.energy_raw[0][7][0], econt.energy_raw[1][7][0], econt.loc_raw[0][7][0]);
  //   }
  // }

  // //===============================================================================================================================
  // //Print the information as read from Link0, Link1 and Link2.
  // //===============================================================================================================================

  // //===============================================================================================================================
  // //Read pedestal file processing Run 1695716961
  // //===============================================================================================================================
  // stringstream ss;
  // string s;
  // int chip, half, channel ;
  // unsigned ped_adc_i, noise_adc_i, ped_tot_i, noise_tot_i;

  // unsigned ped_adc[2][3][2][38], noise_adc[2][3][2][38];
  // unsigned ped_tot[2][3][2][38], noise_tot[2][3][2][38];
  // ifstream fin_lsb("log/ped_link1.txt");
  // while(getline(fin_lsb,s)){
  //   ss << s.data() << endl;
  //   ss >> chip >> half >> channel >> ped_adc_i >> noise_adc_i >> ped_tot_i >> noise_tot_i ;
  //   ped_adc[0][chip][half][channel] = ped_adc_i;
  //   noise_adc[0][chip][half][channel] = noise_adc_i;
  //   ped_tot[0][chip][half][channel] = ped_tot_i;
  //   noise_tot[0][chip][half][channel] = noise_tot_i;
  // }
  // fin_lsb.close();
  
  // ifstream fin_msb("log/ped_link2.txt");
  // while(getline(fin_msb,s)){
  //   ss << s.data() << endl;
  //   ss >> chip >> half >> channel >> ped_adc_i >> noise_adc_i >> ped_tot_i >> noise_tot_i ;
  //   ped_adc[1][chip][half][channel] = ped_adc_i;
  //   noise_adc[1][chip][half][channel] = noise_adc_i;
  //   ped_tot[1][chip][half][channel] = ped_tot_i;
  //   noise_tot[1][chip][half][channel] = noise_tot_i;
  // }
  // fin_msb.close();
  // //===============================================================================================================================
  // //Reading complete for pedestal input
  // //===============================================================================================================================

  // ///// ./emul_econt.exe 1695733045 1695733046
  // //===============================================================================================================================
  // //STC4
  // //===============================================================================================================================
  // TH1F *hCompressDiff = new TH1F("hCompressDiff","Difference in (Emulator - ECONT) compression in percent", 200, -99, 101);
  // TH1F *hCompressDiffECONT = new TH1F("hCompressDiffECONT","Difference in (Emulator - ECONT) compression", 200, -99, 101);
  // int choffset = get<0>(tctorocch[8]) ; //since there are 16 TC per chip
  // int isMSB = 1; //0/1:LSB/MSB corresponds to link1/Link2
  // for(const auto& econt : econtarray){
    
  //   //if(econt.eventId>=10) continue;
    
  //   //First loop to get the bx with maximum modulesum
  //   uint16_t modsum[15];
  //   for(unsigned ibx(0);ibx<15;ibx++) modsum[ibx] = econt.modsum[isMSB][ibx];
  //   const int N = sizeof(modsum) / sizeof(uint16_t);
  //   int bx_max = distance(modsum, max_element(modsum, modsum + N));
    
  //   //The bx with mod sum is not the 8 modulo bx, skip it
  //   bool condn = (econt.bxId==3564) ? (econt.bx_raw[isMSB][bx_max]==15) : (econt.bxId%8 == econt.bx_raw[isMSB][bx_max]) ;
  //   if(!condn) continue;

  //   if(econt.eventId<=5){
  //     cout << "Event : "<<econt.eventId<<", bx_index_with_maximum_modsum : "<< bx_max << ", modsum : "<<econt.modsum[isMSB][bx_max]<< endl;
  //     cout<<"E : \t"; for(int istc=0;istc<12;istc++) cout<<econt.energy_raw[isMSB][bx_max][istc]<<" "; cout<<endl;
  //     cout<<"L : \t"; for(int istc=0;istc<12;istc++) cout<<setw(2)<<std::setfill('0')<<econt.loc_raw[isMSB][bx_max][istc]<<" "; cout<<endl;  
  //   }
    
  //   for(unsigned istc = 0 ; istc < 12 ; istc++){
  //     //if(istc>2) continue;
  //     int noftctot = 0;
  //     uint32_t stcsum = 0;
  //     bool isselchiphalf = false;
  //     for(unsigned itc = 4*istc ; itc < 4*istc + 4 ; itc++ ){
  // 	//if(itc>3) continue;
  // 	//if(econt.eventId<=5) cout << "Event : "<<econt.eventId<<", istc : "<< istc << ", itc : "<<itc<< endl;
  // 	uint32_t compressed = 0;
  // 	uint32_t compressedup = 0;
  // 	uint32_t decompressed = 0;
	
  // 	uint32_t totadc = 0;
  // 	uint32_t totadcup = 0;
  // 	int noftot3 = 0;
  // 	bool istot1 = false;
  // 	bool istot2 = false;
  // 	bool issattot = false;
  // 	for(const auto& roc : rocarray2){
  // 	  if(roc.eventId!=econt.eventId or roc.bxcounter!=econt.bxId) continue;
  // 	  if(roc.channel>=37 or roc.channel==18) continue;
  // 	  int ch = (roc.half==1)?(roc.channel+choffset):roc.channel;
  // 	  if(roc.channel>18) ch -= 1;
  // 	  ch += 72*roc.chip ; 
  // 	  uint32_t adc = uint32_t(roc.adc) & 0x3FF;
  // 	  adc = (adc>(ped_adc[isMSB][roc.chip][roc.half][roc.channel]+noise_adc[isMSB][roc.chip][roc.half][roc.channel])) ? adc : 0 ;
  // 	  uint32_t tot = uint32_t(roc.tot) ; 
  // 	  tot = (tot>(ped_tot[isMSB][roc.chip][roc.half][roc.channel]+noise_tot[isMSB][roc.chip][roc.half][roc.channel])) ? tot : 0 ;
  // 	  uint32_t totup = tot + 7;
  // 	  uint32_t totlin = tot*25;
  // 	  uint32_t totlinup = totup*25;
  // 	  if(ch==get<0>(tctorocch[itc]) or ch==get<1>(tctorocch[itc]) or ch==get<2>(tctorocch[itc]) or ch==get<3>(tctorocch[itc])){
  // 	    totadc += (roc.totflag==3) ? totlin : adc ;
  // 	    totadcup += (roc.totflag==3) ? totlinup : adc ;
  // 	    if(roc.totflag==3) noftot3++;
  // 	    if(roc.totflag==3) noftctot++;
  // 	    if(roc.totflag==1) istot1 = true;
  // 	    if(roc.totflag==2) istot2 = true;
  // 	    if(tot>=512) issattot = true;
  // 	    if(roc.chip==0 and roc.half==1) isselchiphalf = true;
  // 	    if(econt.eventId<=5) cout<<"\t\tievent : " << roc.eventId <<", chip : " << roc.chip << ", half : "<<roc.half<< ", channel : " << roc.channel<<", ch : "<<ch<<", roc.adc : "<<roc.adc<<", adc : "<<adc<<", totflag : "<<roc.totflag <<", roc.tot : "<<roc.tot<<", tot : "<<tot<<", ped : "<<ped_adc[isMSB][roc.chip][roc.half][roc.channel] << ", noise : " << noise_adc[isMSB][roc.chip][roc.half][roc.channel] << ", ped_tot : " << ped_tot[isMSB][roc.chip][roc.half][roc.channel] << ", noise_tot : " << noise_tot[isMSB][roc.chip][roc.half][roc.channel]<<", totadc : "<<totadc<<endl;
  // 	  }//if matching channel
  // 	}//roc for loop
    
  // 	if(!istot1 and !istot2) {
  // 	  compressed = compress_roc(totadc, 1);
  // 	  compressedup = compress_roc(totadcup, 1);
  // 	  decompressed = decompress_econt(compressed, 1);
  // 	  uint32_t calib = 0x800;
  // 	  decompressed = (calib*decompressed)>>11;
  // 	}
  // 	if(econt.eventId<=5) cout <<"\t  itc : "<<(itc)<<", totadc : "<<totadc<<", compressed : "<< compressed<< ", decompressed [econt-t input] : "<< decompressed<< endl;
	
  // 	stcsum += decompressed  ;
  //     }//trig for loop
  //     uint32_t compressed_econt = compress_econt(stcsum, 1);
  //     if(econt.eventId<=5)
  // 	cout <<"istc : "<<istc<<", stcsum : "<<stcsum<<", compressed_econt : "<<compressed_econt<<", econt_raw_energy : "<<econt.energy_raw[isMSB][bx_max][istc] << endl;
      
  //     float diff = float(compressed_econt) - float(econt.energy_raw[isMSB][bx_max][istc]);
  //     float reldiff = 100*(float(compressed_econt) - float(econt.energy_raw[isMSB][bx_max][istc]))/float(econt.energy_raw[isMSB][bx_max][istc]);
  //     //if(noftctot<10) {hCompressDiffECONT->Fill(diff); hCompressDiff->Fill(reldiff);}
  //     //if(isselchiphalf) {hCompressDiffECONT->Fill(diff); hCompressDiff->Fill(reldiff);}
  //     hCompressDiffECONT->Fill(diff);
  //     hCompressDiff->Fill(reldiff);

  //   }//stc loop
  // }//econt loop
  // //===============================================================================================================================
  // //STC4
  // //===============================================================================================================================

  // ////./emul_econt.exe 1695829026 1695829027
  // //===============================================================================================================================
  // //BC9 Old
  // //===============================================================================================================================
  // TH1F *hCompressDiff = new TH1F("hCompressDiff","Difference in (Emulator - ROC) compression", 200, -99, 101);
  // TH1F *hCompressDiffECONT = new TH1F("hCompressDiffECONT","Difference in (Emulator - ECONT) compression", 200, -99, 101);
  // TH1F *hCompressDiffTOTECONT = new TH1F("hCompressDiffTOTECONT","Difference in (Emulator - ECONT) compression for noftotflag>0", 200, -99, 101);
  // TH1F *hCompressDiffECONTPD = new TH1F("hCompressDiffECONTPD","Difference in (Emulator - ECONT) compression", 200, -99, 101);
  // TH1F *hECONTDiff = new TH1F("hECONTDiff","Difference in two methods", 200, -99, 101);
  // TH2F *h2DTCvsCh0 = new TH2F("h2DTCvsCh0","h2DTCvsCh0",300,0,300,200,0,200);
  // TH2F *h2DTCvsCh1 = new TH2F("h2DTCvsCh1","h2DTCvsCh1",300,0,300,200,0,200);
  // TH2F *h2DTCvsCh2 = new TH2F("h2DTCvsCh2","h2DTCvsCh2",300,0,300,200,0,200);
  // TH2F *h2DTCvsCh3 = new TH2F("h2DTCvsCh3","h2DTCvsCh3",300,0,300,200,0,200);
  // TProfile *hProfTCvsCh0 = new TProfile("hProfTCvsCh0","hProfTCvsCh0",300,0,300,0,100);
  // TH1I *hTC0 = new TH1I("hTC0","decompressed energy for TC0",1044,-10,1034);
  // TH2F *h2DTCvsTOTCh0 = new TH2F("h2DTCvsTOTCh0","h2DTCvsTOTCh0",1200,0,12000,1200,0,12000);
  // TH2F *h2DTCvsTOTCh1 = new TH2F("h2DTCvsTOTCh1","h2DTCvsTOTCh1",1200,0,12000,1200,0,12000);
  // TH2F *h2DTCvsTOTCh2 = new TH2F("h2DTCvsTOTCh2","h2DTCvsTOTCh2",1200,0,12000,1200,0,12000);
  // TH2F *h2DTCvsTOTCh3 = new TH2F("h2DTCvsTOTCh3","h2DTCvsTOTCh3",1200,0,12000,1200,0,12000);
  // TH1I *hTC0TOT = new TH1I("hTC0TOT","decompressed energy for TC0 for TOT",1200,0,12000);
  // int choffset = get<0>(tctorocch[8]) ; //==36, since there are 16 TC per chip
  // cout<<"choffset : "<<choffset<<endl;
  // int isMSB = 0; //0/1:LSB/MSB corresponds to link1/Link2
  // for(const auto& econt : econtarray){
    
  //   //if(econt.eventId>=10) continue;
    
  //   //First loop to get the bx with maximum modulesum
  //   // uint16_t modsum[15];
  //   // for(unsigned ibx(0);ibx<15;ibx++) modsum[ibx] = econt.modsum[isMSB][ibx];
  //   // const int N = sizeof(modsum) / sizeof(uint16_t);
  //   // int bx_max = distance(modsum, max_element(modsum, modsum + N));

  //   // if(bx_max==2) continue;
  //   int bx_max = 10;

  //   //The bx with mod sum is not the 8 modulo bx, skip it
  //   bool condn = (econt.bxId==3564) ? (econt.bx_raw[isMSB][bx_max]==15) : (econt.bxId%8 == econt.bx_raw[isMSB][bx_max]) ;
  //   if(!condn) continue;
    
  //   if(econt.eventId<=10){
  //     cout << "Event : "<<econt.eventId<<", bx_index_with_maximum_modsum : "<< bx_max << endl;
  //     cout<<"E : \t"; for(int itc=0;itc<9;itc++) cout<<econt.energy_raw[isMSB][bx_max][itc]<<" "; cout<<endl;
  //     cout<<"L : \t"; for(int itc=0;itc<9;itc++) cout<<setw(2)<<std::setfill('0')<<econt.loc_raw[isMSB][bx_max][itc]<<" "; cout<<endl;  
  //   }
  //   for(unsigned itc = 0 ; itc < 48 ; itc++ ){
  //   //for(unsigned itc = 0 ; itc < 1 ; itc++ ){
      
  //     bool foundTC = false; int refTC = -1;
  //     for(int jtc=0;jtc<9;jtc++)
  // 	if(itc==econt.loc_raw[isMSB][bx_max][jtc]){
  // 	  foundTC = true;
  // 	  refTC = jtc;
  // 	}
  //     if(!foundTC) continue;
      
  //     uint32_t compressed = 0;
  //     uint32_t compressedup = 0;
  //     uint32_t decompressed = 0;
  //     uint32_t tcval = 0;
      
  //     uint32_t totadc = 0;
  //     uint32_t totadcup = 0;
  //     int noftot3 = 0;
  //     bool istot1 = false;
  //     bool iscp0hf0ch0adc = false;
  //     bool iscp0hf0ch1adc = false;
  //     bool iscp0hf0ch2adc = false;
  //     bool iscp0hf0ch3adc = false;
  //     uint32_t refadcch0 = 0;
  //     uint32_t refadcch1 = 0;	    
  //     uint32_t refadcch2 = 0;
  //     uint32_t refadcch3 = 0;
  //     //uint32_t aped0 = 93, aped1 = 96, aped2 = 92, aped3 = 91, adc_th = 22; //from fitting
  //     //uint32_t aped0 = 93, aped1 = 96, aped2 = 89, aped3 = 89, adc_th = 21;
  //     //uint32_t aped0 = 91, aped1 = 96, aped2 = 88, aped3 = 86, adc_th = 20;
  //     //uint32_t aped0 = 86, aped1 = 96, aped2 = 86, aped3 = 86, adc_th = 19; //iterative checking
  //     //uint32_t aped0 = 90, aped1 = 92, aped2 = 90, aped3 = 92, adc_th = 6; //yaml file link2
  //     uint32_t aped0 = 92, aped1 = 91, aped2 = 92, aped3 = 90, adc_th = 5; //yaml file link1
      
  //     uint32_t multfactor = 15;
  //     bool iscp0hf0ch0tot = false;
  //     bool iscp0hf0ch1tot = false;
  //     bool iscp0hf0ch2tot = false;
  //     bool iscp0hf0ch3tot = false;
  //     uint32_t reftotch0 = 0;
  //     uint32_t reftotch1 = 0;
  //     uint32_t reftotch2 = 0;
  //     uint32_t reftotch3 = 0;
  //     uint32_t nof_offs_by_one = 0;
  //     for(const auto& roc : rocarray1){
  //   	if(roc.eventId!=econt.eventId or roc.bxcounter!=econt.bxId) continue; //check if event and bx ids are matching
  //   	if(roc.channel>=37 or roc.channel==18) continue;                      //Skip the calibration and ch number > 27
  // 	if(roc.totflag==2) continue;                                          //Skip the totflag 2
  //   	int ch = (roc.half==1)?(roc.channel+choffset):roc.channel;
  // 	if(roc.channel>18) ch -= 1;
  // 	int pedch = ch;
  //   	ch += 72*roc.chip ;
  // 	if(roc.totflag==0 or roc.totflag==1){
  // 	  uint32_t adc = uint32_t(roc.adc) & 0x3FF;
  // 	  //adc = (adc>(ped_adc[isMSB][roc.chip][roc.half][roc.channel]+noise_adc[isMSB][roc.chip][roc.half][roc.channel])) ? adc : 0 ;
  // 	  if(ch==get<0>(tctorocch[itc]) or ch==get<1>(tctorocch[itc]) or ch==get<2>(tctorocch[itc]) or ch==get<3>(tctorocch[itc])){
  // 	    unsigned ped = pedestal_adc[isMSB][roc.chip][roc.half][pedch];
  // 	    unsigned thr = threshold_adc[isMSB][roc.chip][roc.half][pedch];
  // 	    // if(roc.chip==0 and roc.half==0 and roc.channel==0) {iscp0hf0ch0adc = true; refadcch0 = adc; if(TMath::Abs(int(adc)-int(aped0+adc_th))==1) nof_offs_by_one++; adc = (adc>=(aped0+adc_th)) ? adc-aped0 : 0 ;}
  // 	    // if(roc.chip==0 and roc.half==0 and roc.channel==1) {iscp0hf0ch1adc = true; refadcch1 = adc; if(TMath::Abs(int(adc)-int(aped1+adc_th))==1) nof_offs_by_one++; adc = (adc>=(aped1+adc_th)) ? adc-aped1 : 0 ;}
  // 	    // if(roc.chip==0 and roc.half==0 and roc.channel==2) {iscp0hf0ch2adc = true; refadcch2 = adc; if(TMath::Abs(int(adc)-int(aped2+adc_th))==1) nof_offs_by_one++; adc = (adc>=(aped2+adc_th)) ? adc-aped2 : 0 ;}
  // 	    // if(roc.chip==0 and roc.half==0 and roc.channel==3) {iscp0hf0ch3adc = true; refadcch3 = adc; if(TMath::Abs(int(adc)-int(aped3+adc_th))==1) nof_offs_by_one++; adc = (adc>=(aped3+adc_th)) ? adc-aped3 : 0 ;}
  // 	    if(roc.chip==0 and roc.half==0 and roc.channel==0) {iscp0hf0ch0adc = true; refadcch0 = adc; if(TMath::Abs(int(adc)-int(ped+thr))==1) nof_offs_by_one++; adc = (adc>=(ped+thr)) ? adc-ped : 0 ;}
  // 	    if(roc.chip==0 and roc.half==0 and roc.channel==1) {iscp0hf0ch1adc = true; refadcch1 = adc; if(TMath::Abs(int(adc)-int(ped+thr))==1) nof_offs_by_one++; adc = (adc>=(ped+thr)) ? adc-ped : 0 ;}
  // 	    if(roc.chip==0 and roc.half==0 and roc.channel==2) {iscp0hf0ch2adc = true; refadcch2 = adc; if(TMath::Abs(int(adc)-int(ped+thr))==1) nof_offs_by_one++; adc = (adc>=(ped+thr)) ? adc-ped : 0 ;}
  // 	    if(roc.chip==0 and roc.half==0 and roc.channel==3) {iscp0hf0ch3adc = true; refadcch3 = adc; if(TMath::Abs(int(adc)-int(ped+thr))==1) nof_offs_by_one++; adc = (adc>=(ped+thr)) ? adc-ped : 0 ;}
  // 	    totadc += adc ;
  // 	    if(roc.totflag==1) istot1 = true;
  // 	    if(econt.eventId<=10 and foundTC)
  // 	      cout<<"\t\tievent : " << roc.eventId <<", chip : " << roc.chip << ", half : "<<roc.half<< ", channel : " << roc.channel<<", ch : "<<ch<<", adc : "<<adc<<", totflag : "<<roc.totflag <<", tot : "<<roc.tot<<", totadc : "<<totadc<<endl;
  // 	  }
  // 	}
  // 	if(roc.totflag==3){
  // 	  uint32_t tot = uint32_t(roc.tot) ;
  // 	  //tot = (tot>(ped_tot[isMSB][roc.chip][roc.half][roc.channel]+noise_tot[isMSB][roc.chip][roc.half][roc.channel])) ? tot : 0 ;
  // 	  uint32_t totup = tot + 7;
  // 	  uint32_t totlin = tot*multfactor;
  // 	  uint32_t totlinup = totup*multfactor;
  // 	  if(ch==get<0>(tctorocch[itc]) or ch==get<1>(tctorocch[itc]) or ch==get<2>(tctorocch[itc]) or ch==get<3>(tctorocch[itc])){
  // 	    if(roc.chip==0 and roc.half==0 and roc.channel==0) {iscp0hf0ch0tot = true; reftotch0 = totlin;}
  // 	    if(roc.chip==0 and roc.half==0 and roc.channel==1) {iscp0hf0ch1tot = true; reftotch1 = totlin;}
  // 	    if(roc.chip==0 and roc.half==0 and roc.channel==2) {iscp0hf0ch2tot = true; reftotch2 = totlin;}
  // 	    if(roc.chip==0 and roc.half==0 and roc.channel==3) {iscp0hf0ch3tot = true; reftotch3 = totlin;}
  // 	    totadc += totlin ;
  // 	    totadcup += totlinup ;
  // 	    if(roc.totflag==3) noftot3++;
  // 	    if(econt.eventId<=10 and foundTC)
  // 	      cout<<"\t\tievent : " << roc.eventId <<", chip : " << roc.chip << ", half : "<<roc.half<< ", channel : " << roc.channel<<", ch : "<<ch<<", adc : "<<roc.adc<<", totflag : "<<roc.totflag <<", tot : "<<tot<<", totadc : "<<totadc<<endl;
  // 	  }
	  
  // 	}
  //     }//roc for loop
    
  //     if(!istot1) {
  //   	compressed = compress_roc(totadc, 1);
  //   	compressedup = compress_roc(totadcup, 1);
  //   	decompressed = decompress_econt(compressed, 1);
	
  // 	uint32_t calib = 0x800;
  // 	decompressed = (calib*decompressed)>>11;
	
  //   	//cout<<"itc : "<<itc<<", decompressed : " << decompressed << endl;
  // 	float diff = float(compressed) - float(econt.energy_raw[isMSB][bx_max][refTC]);
  // 	if(foundTC) hCompressDiff->Fill(diff);
  // 	uint32_t rawsum = 0;
  // 	uint32_t compressed_econt_PD = decompress_compress_econt_PD(compressed, 1, rawsum);
  // 	uint32_t compressed_econt = compress_econt(decompressed, 1);
	
  // 	if(econt.eventId<=10 and foundTC)
  // 	  cout <<"\t  itc : "<<(itc)<<", totadc : "<<totadc
  // 	       <<", compressed : "<< compressed
  // 	       << ", decompressed [econt-t input] : "<< decompressed
  // 	       <<", compressed_econt [econ-t output] : "<< compressed_econt 
  // 	       << endl;
  // 	diff = float(compressed_econt) - float(econt.energy_raw[isMSB][bx_max][refTC]);
  // 	bool isZero = false;
  // 	if(compressed_econt==0 or econt.energy_raw[isMSB][bx_max][refTC]==0) isZero = true;
  // 	if(TMath::Abs(diff)>1 and noftot3==0 and !isZero){
  // 	  cout<<"ADC::Large Diff : "<<diff<< ", Event : "<<econt.eventId<<", ADC : ("<<refadcch0<<", "<<refadcch1<<", "<<refadcch2<<", "<<refadcch3<<"), nof_offs_by_one : "<<nof_offs_by_one<<endl;
  // 	  cout << "Event : "<<econt.eventId<<", bx_index_with_maximum_modsum : "<< bx_max <<", modsum : " << econt.modsum[isMSB][bx_max] << endl;
  // 	  cout<<"E : \t"; for(int itc=0;itc<9;itc++) cout<<econt.energy_raw[isMSB][bx_max][itc]<<" "; cout<<endl;
  // 	  cout<<"L : \t"; for(int itc=0;itc<9;itc++) cout<<setw(2)<<std::setfill('0')<<econt.loc_raw[isMSB][bx_max][itc]<<" "; cout<<endl;  
  // 	  cout <<"\t  itc : "<<(itc)<<", totadc : "<<totadc
  // 	       <<", compressed : "<< compressed
  // 	       << ", decompressed [econt-t input] : "<< decompressed
  // 	       <<", compressed_econt [econ-t output] : "<< compressed_econt 
  // 	       << endl;
  // 	}
  // 	if(TMath::Abs(diff)>1 and noftot3!=0 and !isZero){
  // 	  cout<<"TOT::Large Diff : "<<diff<< ", Event : "<<econt.eventId<<", TOT : ("<<reftotch0<<", "<<reftotch1<<", "<<reftotch2<<", "<<reftotch3<<"), ADC : ("<<refadcch0<<", "<<refadcch1<<", "<<refadcch2<<", "<<refadcch3<<") "<<endl;
  // 	  cout << "Event : "<<econt.eventId<<", bx_index_with_maximum_modsum : "<< bx_max <<", modsum : " << econt.modsum[isMSB][bx_max] << endl;
  // 	  cout<<"E : \t"; for(int itc=0;itc<9;itc++) cout<<econt.energy_raw[isMSB][bx_max][itc]<<" "; cout<<endl;
  // 	  cout<<"L : \t"; for(int itc=0;itc<9;itc++) cout<<setw(2)<<std::setfill('0')<<econt.loc_raw[isMSB][bx_max][itc]<<" "; cout<<endl;  
  // 	  cout <<"\t  itc : "<<(itc)<<", totadc : "<<totadc
  // 	       <<", compressed : "<< compressed
  // 	       << ", decompressed [econt-t input] : "<< decompressed
  // 	       <<", compressed_econt [econ-t output] : "<< compressed_econt 
  // 	       << endl;
  // 	}
  // 	if(foundTC and noftot3==0 and !isZero) hCompressDiffECONT->Fill(diff);
  // 	if(foundTC and noftot3!=0 and !isZero) hCompressDiffTOTECONT->Fill(diff);
  // 	diff = float(compressed_econt_PD) - float(econt.energy_raw[isMSB][bx_max][refTC]);
  // 	if(foundTC) hCompressDiffECONTPD->Fill(diff);
  // 	diff = float(compressed_econt_PD) - float(compressed_econt);
  // 	if(foundTC) hECONTDiff->Fill(diff);
  // 	uint32_t tc_decompressed = decompress_econt(econt.energy_raw[isMSB][bx_max][refTC],1);
  // 	if(iscp0hf0ch0adc and foundTC and itc==0 and noftot3==0) {h2DTCvsCh0->Fill(float(refadcch0),float(tc_decompressed)); hProfTCvsCh0->Fill(float(refadcch0),float(tc_decompressed));}
  // 	if(iscp0hf0ch1adc and foundTC and itc==0 and noftot3==0) h2DTCvsCh1->Fill(float(refadcch1),float(tc_decompressed)); 
  // 	if(iscp0hf0ch2adc and foundTC and itc==0 and noftot3==0) h2DTCvsCh2->Fill(float(refadcch2),float(tc_decompressed));
  // 	if(iscp0hf0ch3adc and foundTC and itc==0 and noftot3==0) h2DTCvsCh3->Fill(float(refadcch3),float(tc_decompressed));
  // 	if(foundTC and itc==0 and noftot3==0) hTC0->Fill(float(tc_decompressed));
  // 	if(iscp0hf0ch0tot and foundTC and itc==0 and noftot3!=0) h2DTCvsTOTCh0->Fill(float(reftotch0),float(tc_decompressed));
  // 	if(iscp0hf0ch1tot and foundTC and itc==0 and noftot3!=0) h2DTCvsTOTCh1->Fill(float(reftotch1),float(tc_decompressed));
  // 	if(iscp0hf0ch2tot and foundTC and itc==0 and noftot3!=0) h2DTCvsTOTCh2->Fill(float(reftotch2),float(tc_decompressed));
  // 	if(iscp0hf0ch3tot and foundTC and itc==0 and noftot3!=0) h2DTCvsTOTCh3->Fill(float(reftotch3),float(tc_decompressed));
  // 	if(foundTC and itc==0 and noftot3!=0) hTC0TOT->Fill(float(tc_decompressed));
  //     }
      
  //   }//trigger cell for loop
    
  // }//econt loop
  // //hCompressDiff->SetMinimum(1.e-1);
  // hCompressDiff->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ROC");
  // hCompressDiff->SetLineColor(kBlue);
  // hCompressDiffECONT->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ECONT");
  // hCompressDiffECONT->SetLineColor(kRed);
  // //===============================================================================================================================
  // //BC9 Old
  // //===============================================================================================================================

  ////./emul_econt.exe 1695829026 1695829027
  //===============================================================================================================================
  //BC9 New
  //===============================================================================================================================
  TH1F *hCompressDiff = new TH1F("hCompressDiff","Difference in (Emulator - ROC) compression", 200, -99, 101);
  TH1F *hCompressDiffECONT = new TH1F("hCompressDiffECONT","Difference in (Emulator - ECONT) compression", 200, -99, 101);
  TH1F *hCompressDiffTOTECONT = new TH1F("hCompressDiffTOTECONT","Difference in (Emulator - ECONT) compression for noftotflag>0", 200, -99, 101);
  TH1F *hCompressDiffECONTPD = new TH1F("hCompressDiffECONTPD","Difference in (Emulator - ECONT) compression", 200, -99, 101);
  TH1F *hTCHits = new TH1F("hTCHits","Hit distribution in TCs", 50, -1, 49);
  TH1F *hTCEnergy[48], *hEmulEnergy[48];
  TH1F *hCompressDiffTCADC[48],*hCompressDiffTCTOT[48];
  for(int itc=0;itc<48;itc++){
    hTCEnergy[itc] = new TH1F(Form("hTCEnergy_%d",itc),Form("Compressed energy distribution of ECONT for TC : %d",itc), 200, -99, 101);
    hEmulEnergy[itc] = new TH1F(Form("hEmulEnergy_%d",itc),Form("Compressed energy distribution using Emulator for TC  : %d",itc), 200, -99, 101);
    hCompressDiffTCADC[itc] = new TH1F(Form("hCompressDiffTCADC_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with totflag==0",itc), 200, -99, 101);
    hCompressDiffTCTOT[itc] = new TH1F(Form("hCompressDiffTCTOT_%d",itc),Form("Difference in (Emulator - ECONT) compression for TC : %d with totflag==3",itc), 200, -99, 101);
    hCompressDiffTCADC[itc]->SetLineColor(kRed);
    hCompressDiffTCADC[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
    hCompressDiffTCTOT[itc]->SetLineColor(kBlue);
    hCompressDiffTCTOT[itc]->GetXaxis()->SetTitle("Difference in (Emulator - ECONT)");
    hTCEnergy[itc]->SetLineColor(kBlack);
    hEmulEnergy[itc]->SetLineColor(kMagenta);
  }
  int choffset = get<0>(tctorocch[8]) ; //==36, since there are 16 TC per chip
  cout<<"choffset : "<<choffset<<endl;  
  int isMSB = 1; //0/1:LSB/MSB corresponds to link1/Link2
  uint64_t roc_counter = 0;
  for(const auto& [eventId, econt] : econtarray){
    
    if(eventId<=10){
      cout << "Event : "<<eventId<<", modsum : "<< uint16_t(econt.modsum[isMSB]) << endl;
      cout<<"E : \t"; for(int itc=0;itc<9;itc++) cout<<uint16_t(econt.energy_raw[isMSB][itc])<<" "; cout<<endl;
      cout<<"L : \t"; for(int itc=0;itc<9;itc++) cout<<setw(2)<<std::setfill('0')<<uint16_t(econt.loc_raw[isMSB][itc])<<" "; cout<<endl;  
    }
    for(unsigned itc = 0 ; itc < 48 ; itc++ ){
    //for(unsigned itc = 0 ; itc < 1 ; itc++ ){
      
      bool foundTC = false; int refTC = -1;
      for(int jtc=0;jtc<9;jtc++)
    	if(itc==econt.loc_raw[isMSB][jtc]){
    	  foundTC = true;
    	  refTC = jtc;
    	}
      if(!foundTC) continue;
      
      uint32_t compressed = 0;
      uint32_t compressedup = 0;
      uint32_t decompressed = 0;
      
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
      uint32_t multfactor = 15;
      
      for(const auto& roc : rocarray2[eventId]){
	roc_counter++;
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
	  if(ped<10) ped = 255; //pedestal value 255 has been used for masked channels
    	  unsigned thr = threshold_adc[isMSB][roc_chip][roc_half][pedch];
    	  uint32_t adc = roc.adc.to_ulong() & 0x3FF;
    	  adc = (adc>(ped+thr)) ? adc-ped : 0 ;
    	  if(ch==get<0>(tctorocch[itc]) or ch==get<1>(tctorocch[itc]) or ch==get<2>(tctorocch[itc]) or ch==get<3>(tctorocch[itc])){
  	    if(ch==get<0>(tctorocch[itc])) {iscp0hf0ch0adc = true; refadcch0 = roc.adc.to_ulong();}
  	    if(ch==get<1>(tctorocch[itc])) {iscp0hf0ch1adc = true; refadcch1 = roc.adc.to_ulong();}
  	    if(ch==get<2>(tctorocch[itc])) {iscp0hf0ch2adc = true; refadcch2 = roc.adc.to_ulong();}
  	    if(ch==get<3>(tctorocch[itc])) {iscp0hf0ch3adc = true; refadcch3 = roc.adc.to_ulong();}
    	    totadc += adc ;
    	    if(roc.totflag==1) istot1 = true;
	    if(roc.totflag==2) istot2 = true;
    	    if(eventId<=10 and foundTC)
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
    	    if(ch!=137) totadc += totlin ;
    	    totadcup += totlinup ;
    	    if(roc_totflag==3) noftot3++;
    	    if(eventId<=10 and foundTC)
    	      cout<<"\t\tievent : " << eventId <<", chip : " << roc_chip << ", half : "<<roc_half<< ", channel : " << roc_channel<<", ch : "<<ch<<", adc : "<<roc.adc.to_ulong()<<", totflag : "<<roc_totflag <<", tot : "<<tot<<", totadc : "<<totadc<<endl;
    	  }
	  
    	}
      }//roc for loop

      if(!istot1 and !istot2) {
	//{
    	compressed = compress_roc(totadc, 1);
    	compressedup = compress_roc(totadcup, 1);
    	decompressed = decompress_econt(compressed, 1);
	
    	uint32_t calib = 0x800;
    	decompressed = (calib*decompressed)>>11;
	
    	//cout<<"itc : "<<itc<<", decompressed : " << decompressed << endl;
    	float diff = float(compressed) - float(econt.energy_raw[isMSB][refTC]);
    	if(foundTC) hCompressDiff->Fill(diff);
    	uint32_t rawsum = 0;
    	uint32_t compressed_econt_PD = decompress_compress_econt_PD(compressed, 1, rawsum);
    	uint32_t compressed_econt = compress_econt(decompressed, 1);
	
    	if(eventId<=10 and foundTC)
    	  cout <<"\t  itc : "<<(itc)<<", totadc : "<<totadc
    	       <<", compressed : "<< compressed
    	       << ", decompressed [econt-t input] : "<< decompressed
    	       <<", compressed_econt [econ-t output] : "<< compressed_econt 
    	       << endl;
    	diff = float(compressed_econt) - float(econt.energy_raw[isMSB][refTC]);
    	bool isZero = false;
    	if(compressed_econt==0 or econt.energy_raw[isMSB][refTC]==0) isZero = true;
    	if(TMath::Abs(diff)>10 and noftot3==0 and !isZero){
    	  cout<<"ADC::Large Diff : "<<diff<< ", Event : "<<eventId<<", ADC : ("<<refadcch0<<", "<<refadcch1<<", "<<refadcch2<<", "<<refadcch3<<")"<<endl;
	  cout << "Event : "<<eventId<<", modsum : "<< uint16_t(econt.modsum[isMSB]) << endl;
	  cout<<"E : \t"; for(int itc=0;itc<9;itc++) cout<<uint16_t(econt.energy_raw[isMSB][itc])<<" "; cout<<endl;
	  cout<<"L : \t"; for(int itc=0;itc<9;itc++) cout<<setw(2)<<std::setfill('0')<<uint16_t(econt.loc_raw[isMSB][itc])<<" "; cout<<endl;  
    	  cout <<"\t  itc : "<<(itc)<<", totadc : "<<totadc
    	       <<", compressed : "<< compressed
    	       << ", decompressed [econt-t input] : "<< decompressed
    	       <<", compressed_econt [econ-t output] : "<< compressed_econt 
    	       << endl;
    	}
    	if(TMath::Abs(diff)>10 and noftot3!=0 and !isZero){
    	  cout<<"TOT::Large Diff : "<<diff<< ", Event : "<<eventId<<", TOT : ("<<reftotch0<<", "<<reftotch1<<", "<<reftotch2<<", "<<reftotch3<<"), ADC : ("<<refadcch0<<", "<<refadcch1<<", "<<refadcch2<<", "<<refadcch3<<") "<<endl;
	  cout << "Event : "<<eventId<<", modsum : "<< uint16_t(econt.modsum[isMSB]) << endl;
	  cout<<"E : \t"; for(int itc=0;itc<9;itc++) cout<<uint16_t(econt.energy_raw[isMSB][itc])<<" "; cout<<endl;
	  cout<<"L : \t"; for(int itc=0;itc<9;itc++) cout<<setw(2)<<std::setfill('0')<<uint16_t(econt.loc_raw[isMSB][itc])<<" "; cout<<endl;  
    	  cout <<"\t  itc : "<<(itc)<<", totadc : "<<totadc
    	       <<", compressed : "<< compressed
    	       << ", decompressed [econt-t input] : "<< decompressed
    	       <<", compressed_econt [econ-t output] : "<< compressed_econt 
    	       << endl;
    	}
	if(foundTC and !isZero){
	  hTCHits->Fill(itc);
	  hTCEnergy[itc]->Fill(float(econt.energy_raw[isMSB][refTC]));
	  hEmulEnergy[itc]->Fill(float(compressed_econt));
	}
    	if(foundTC and noftot3==0 and !isZero){
	  hCompressDiffECONT->Fill(diff);
	  hCompressDiffTCADC[itc]->Fill(diff);
	}
    	if(foundTC and noftot3!=0 and !isZero){
	  hCompressDiffTOTECONT->Fill(diff);
	  hCompressDiffTCTOT[itc]->Fill(diff);
	}
    	diff = float(compressed_econt_PD) - float(econt.energy_raw[isMSB][refTC]);
    	if(foundTC and noftot3==0 and !isZero) hCompressDiffECONTPD->Fill(diff);
    	// diff = float(compressed_econt_PD) - float(compressed_econt);
    	// if(foundTC) hECONTDiff->Fill(diff);
    	// uint32_t tc_decompressed = decompress_econt(econt.energy_raw[isMSB][bx_max][refTC],1);
    	// if(foundTC and itc==0 and noftot3==0) hTC0->Fill(float(tc_decompressed));
    	// if(foundTC and itc==0 and noftot3!=0) hTC0TOT->Fill(float(tc_decompressed));
      }
      
    }//trigger cell for loop
    
  }//econt loop
  hCompressDiff->SetMinimum(1.e-1);
  hCompressDiff->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ROC");
  hCompressDiff->SetLineColor(kBlue);
  hCompressDiffECONT->GetXaxis()->SetTitle("Difference in compressed value : Emulator - ECONT");
  hCompressDiffECONT->SetLineColor(kRed);
  //===============================================================================================================================
  //BC9 New
  //===============================================================================================================================

  TCanvas *c1_Diff_ADC[3], *c1_Diff_TOT[3];
  for(int ichip=0;ichip<3;ichip++){
    c1_Diff_ADC[ichip] = new TCanvas(Form("c1_Diff_Comp_ADC_chip_%d",ichip),Form("c1_Diff_Comp_ADC_chip_%d",ichip));
    c1_Diff_ADC[ichip]->Divide(4,4);
    for(int ipad=0;ipad<16;ipad++){
      c1_Diff_ADC[ichip]->cd(ipad+1)->SetLogy();
      int ihist = 16*ichip + ipad;
      hCompressDiffTCADC[ihist]->Draw();
    }
  }
  for(int ichip=0;ichip<3;ichip++){
    c1_Diff_TOT[ichip] = new TCanvas(Form("c1_Diff_Comp_TOT_chip_%d",ichip),Form("c1_Diff_Comp_TOT_chip_%d",ichip));
    c1_Diff_TOT[ichip]->Divide(4,4);
    for(int ipad=0;ipad<16;ipad++){
      c1_Diff_TOT[ichip]->cd(ipad+1)->SetLogy();
      int ihist = 16*ichip + ipad;
      hCompressDiffTCTOT[ihist]->Draw();
    }
  }
  // //===============================================================================================================================
  // //ADC/ADCM/TOA/TOT histograms
  // //===============================================================================================================================
  // TCanvas *c1 = new TCanvas("c1","ADC");
  // c1->Divide(2,2);
  // c1->cd(1)->SetLogy();
  // hADC_ch0_flag0->Draw();
  // c1->cd(2)->SetLogy();
  // hADC_ch0_flag1->Draw();
  // c1->cd(3)->SetLogy();
  // hADC_ch0_flag2->Draw();
  // c1->cd(4)->SetLogy();
  // hADC_ch0_flag3->Draw();

  // TCanvas *c2 = new TCanvas("c2","ADCM");
  // c2->Divide(2,2);
  // c2->cd(1)->SetLogy();
  // hADCM_ch0_flag0->Draw();
  // c2->cd(2)->SetLogy();
  // hADCM_ch0_flag1->Draw();
  // c2->cd(3)->SetLogy();
  // hADCM_ch0_flag2->Draw();
  // c2->cd(4)->SetLogy();
  // hADCM_ch0_flag3->Draw();

  // TCanvas *c3 = new TCanvas("c3","TOA");
  // c3->Divide(2,2);
  // c3->cd(1)->SetLogy();
  // hTOA_ch0_flag0->Draw();
  // c3->cd(2)->SetLogy();
  // hTOA_ch0_flag1->Draw();
  // c3->cd(3)->SetLogy();
  // hTOA_ch0_flag2->Draw();
  // c3->cd(4)->SetLogy();
  // hTOA_ch0_flag3->Draw();

  // TCanvas *c4 = new TCanvas("c4","TOT");
  // c4->Divide(2,2);
  // c4->cd(1)->SetLogy();
  // hTOT_ch0_flag0->Draw();
  // c4->cd(2)->SetLogy();
  // hTOT_ch0_flag1->Draw();
  // c4->cd(3)->SetLogy();
  // hTOT_ch0_flag2->Draw();
  // c4->cd(4)->SetLogy();
  // hTOT_ch0_flag3->Draw();

  // TCanvas *c5 = new TCanvas("c5","ADC");
  // c5->Divide(2,2);
  // c5->cd(1)->SetLogy();
  // hADC_ch0_flag0->Draw();
  // c5->cd(2)->SetLogy();
  // hADC_ch1_flag0->Draw();
  // c5->cd(3)->SetLogy();
  // hADC_ch2_flag0->Draw();
  // c5->cd(4)->SetLogy();
  // hADC_ch3_flag0->Draw();
  // //===============================================================================================================================
  // //ADC/ADCM/TOA/TOT histograms
  // //===============================================================================================================================


  //===============================================================================================================================
  //Pedestal and threshold histograms
  //===============================================================================================================================
  TH1F *hPedADC_LSB[3][2], *hThreshADC_LSB[3][2];
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      hPedADC_LSB[ichip][ihalf] = new TH1F(Form("hPedADC_LSB_%d_%d",ichip,ihalf),Form("hPedADC_LSB_%d_%d",ichip,ihalf),40,-1,39);
      hThreshADC_LSB[ichip][ihalf] = new TH1F(Form("hThreshADC_LSB_%d_%d",ichip,ihalf),Form("hThreshADC_LSB_%d_%d",ichip,ihalf),40,-1,39);
      for(int ichannel=0;ichannel<36;ichannel++) {
  	hPedADC_LSB[ichip][ihalf]->SetBinContent(hPedADC_LSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), float(pedestal_adc[0][ichip][ihalf][ichannel]));
  	hThreshADC_LSB[ichip][ihalf]->SetBinContent(hThreshADC_LSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), float(threshold_adc[0][ichip][ihalf][ichannel]));
      }
    }
  }
  
  TH1F *hPedADC_MSB[3][2], *hThreshADC_MSB[3][2];
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      hPedADC_MSB[ichip][ihalf] = new TH1F(Form("hPedADC_MSB_%d_%d",ichip,ihalf),Form("hPedADC_MSB_%d_%d",ichip,ihalf),40,-1,39);
      hThreshADC_MSB[ichip][ihalf] = new TH1F(Form("hThreshADC_MSB_%d_%d",ichip,ihalf),Form("hThreshADC_MSB_%d_%d",ichip,ihalf),40,-1,39);
      for(int ichannel=0;ichannel<36;ichannel++) {
  	hPedADC_MSB[ichip][ihalf]->SetBinContent(hPedADC_MSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), float(pedestal_adc[1][ichip][ihalf][ichannel]));
  	hThreshADC_MSB[ichip][ihalf]->SetBinContent(hThreshADC_MSB[ichip][ihalf]->GetXaxis()->FindBin(ichannel), float(threshold_adc[1][ichip][ihalf][ichannel]));
      }
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////
  TCanvas *c1_ped_adc_lsb = new TCanvas("c1_ped_adc_lsb","c1_ped_adc_lsb",1200,800);
  c1_ped_adc_lsb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c1_ped_adc_lsb->cd(icanvas);
      hPedADC_LSB[ichip][ihalf]->Draw();
    }
  }
  
  TCanvas *c2_thresh_adc_lsb = new TCanvas("c2_thresh_adc_lsb","c2_thresh_adc_lsb",1200,800);
  c2_thresh_adc_lsb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c2_thresh_adc_lsb->cd(icanvas);
      hThreshADC_LSB[ichip][ihalf]->Draw();
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
  
  TCanvas *c2_thresh_adc_msb = new TCanvas("c2_thresh_adc_msb","c2_thresh_adc_msb",1200,800);
  c2_thresh_adc_msb->Divide(3,2);
  for(int ichip=0;ichip<3;ichip++){
    for(int ihalf=0;ihalf<2;ihalf++){
      int icanvas = 2*ichip + ihalf + 1;
      c2_thresh_adc_msb->cd(icanvas);
      hThreshADC_MSB[ichip][ihalf]->Draw();
    }
  }
  
  TFile *fcanva_out = new TFile("log/Pedestal.root","recreate");
  c1_ped_adc_lsb->Write();
  c2_thresh_adc_lsb->Write();
  c1_ped_adc_msb->Write();
  c2_thresh_adc_msb->Write();
  fcanva_out->Close();
  delete fcanva_out;
  // //===============================================================================================================================
  // //Pedestal and threshold histograms
  // //===============================================================================================================================

  
  

  TFile *fout = new TFile("log/out.root","recreate");
  for(int ichip=0;ichip<3;ichip++){
    c1_Diff_ADC[ichip]->Write();
    c1_Diff_TOT[ichip]->Write();
  }
  // hTOTFlag_0->Write();
  // hTOTFlag_1->Write();
  // hTOTFlag_2->Write();
  // hTOTFlag_3->Write();
  // c1->Write();
  // c2->Write();
  // c3->Write();
  // c4->Write();
  // c5->Write();
  hCompressDiff->Write();
  hCompressDiffECONT->Write();
  hCompressDiffECONTPD->Write();
  hCompressDiffTOTECONT->Write();
  // hECONTDiff->Write();
  // h2DTCvsCh0->Write();
  // h2DTCvsCh1->Write();
  // h2DTCvsCh2->Write();
  // h2DTCvsCh3->Write();
  // hProfTCvsCh0->Write();
  // hTC0->Write();
  // h2DTCvsTOTCh0->Write();
  // h2DTCvsTOTCh1->Write();
  // h2DTCvsTOTCh2->Write();
  // h2DTCvsTOTCh3->Write();
  // hTC0TOT->Write();
  hTCHits->Write();
  for(int itc=0;itc<48;itc++) hTCEnergy[itc]->Write();
  for(int itc=0;itc<48;itc++) hEmulEnergy[itc]->Write();
  for(int itc=0;itc<48;itc++) hCompressDiffTCADC[itc]->Write();
  for(int itc=0;itc<48;itc++) hCompressDiffTCTOT[itc]->Write();
  fout->Close();
  delete fout;
  
  tctorocch.clear();
  econtarray.clear();
  //rocarray1.clear();
  rocarray2.clear();

  cout<<"roc_counter : "<<roc_counter<<endl;
  
  return true;
}
