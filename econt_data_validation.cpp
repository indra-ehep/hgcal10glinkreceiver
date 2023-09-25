#include <iostream>

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

#include "TFileHandler.h"
#include "FileReader.h"

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
  uint16_t l1aType;
  uint8_t ECONT_packet_status[2][12];                //2 : LSB/MSB, 12 : STC
  bool ECONT_packet_validity[2][12];                 //2 : LSB/MSB, 12 : STC
  uint8_t OC[2],EC[2];                               //2 : LSB/MSB
  uint16_t BC[2];                                    //2 : LSB/MSB
  
  uint16_t daq_data[4];                              //4 : data blocks separated by 0xfecafe
  uint16_t daq_nbx[4];                               //4 : data blocks separated by 0xfecafe
  uint16_t size_in_cafe[4];                          //4 : data blocks separated by 0xfecafe
  
  uint64_t energy_raw[2][15][12];                    //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint64_t loc_raw[2][15][12];                       //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint64_t energy_unpkd[2][15][12];                  //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  uint64_t loc_unpkd[2][15][12];                     //2 : LSB/MSB, 15 : for max 7 bxs, 12 : STC
  
} econt_event;

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
  return ((word >> 8) & 0xFFFFFF) == 16698110; //0xFECAFE                                                      // 0xFF FFFF masking to be there to compare with 16698110 (could have used the hex of this)
}

// returns location in this event of the n'th 0xfecafe... line
int find_cafe_word(const Hgcal10gLinkReceiver::RecordRunning *rEvent, int n, int cafe_word_loc = -1) {
  const uint64_t *p64(((const uint64_t*)rEvent)+1);

  if (cafe_word_loc > 0) {
    if (is_cafe_word(p64[cafe_word_loc])) {
      return cafe_word_loc;
    }
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
    std::cerr << "Could not find cafe word" << std::endl;
    exit(0);
  }
  else {
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
  uint64_t word1 = (packet64[0] << 28+32) + (packet64[1] << 28) + (packet64[2] >> 4);
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
	 << "OC[LSB] : " << ev.OC[0] << ", OC[MSB] : " << ev.OC[1]
      	 << "BC[LSB] : " << ev.BC[0] << ", BC[MSB] : " << ev.BC[1]
	 <<endl;
    econt_events.pop_back();
  }
  cout<<endl;

  
}
int main(int argc, char** argv){
  
  if(argc < 3){
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return 1;
  }

  //Command line arg assignment
  //Assign relay and run numbers
  unsigned relayNumber(0);
  unsigned runNumber(0);
  unsigned linkNumber(0);
  bool skipMSB(true);
  
  // ./econt_data_validation.exe $Relay $rname
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;
  std::istringstream issRun(argv[2]);
  issRun >> runNumber;
  
  //Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;

  //Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);

  //Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);
  _fileReader.setDirectory(std::string("dat/Relay")+argv[1]);
  _fileReader.openRun(runNumber,linkNumber);

  uint64_t nEvents = 0;
  uint32_t packet[4];
  uint32_t packet_counter;
  uint32_t packet_locations[12];
  uint64_t packet_energies[12];
  
  // make histograms 10 trigger types, two ECONTs, and 12 STC 
  TH1I**** hloc = 0x0; // locations of most energetic TC in STC
  hloc = new TH1I***[10];                       // 10 trigger types
  int nTrigType = 0;
  for (int itrig=0; itrig<10; itrig++) {
    hloc[itrig] = new TH1I**[2];                // 2 ECONTs
    for (int ietc=0; ietc<2; ietc++) {
      hloc[itrig][ietc] = new TH1I*[12];        // 12 STCs
      for (int istc=0;istc<12;istc++) {
	hloc[itrig][ietc][istc] = new TH1I(Form("hloc_%d_%d_%d",itrig,ietc,istc),Form("Trig %d, iECONT %d, STC : %d",itrig,ietc,istc),4,0,4);
	hloc[itrig][ietc][istc]->SetMinimum(0.0);
	hloc[itrig][ietc][istc]->GetXaxis()->SetTitle("TC");
	hloc[itrig][ietc][istc]->GetYaxis()->SetTitle("Entries");
      }//istc  
    }//ietc
  }//itrig
  
    
  // to cache where the cafe separators are
  int scintillator_cafe_word_loc;
  int econt_cafe_word_loc;
  int total_phys_events = 0;
  int total_coinc_events = 0;
  
  //Keep a record of status of last 100 events
  int max_event_entries = 100;
  deque<econt_event> econt_events;
  
  bool isGood = true;
  //Use the fileReader to read the records
  
  while(_fileReader.read(r)) {
    //Check the state of the record and print the record accordingly
    if( r->state()==Hgcal10gLinkReceiver::FsmState::Starting){
      if(!(rStart->valid())){
  	std::cerr << " FsmState::Starting validadity fails : rStart->valid() " << rStart->valid() << std::endl;
  	isGood = false;
  	rStart->print();
  	std::cout << std::endl;
      }
    }
    
    else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping){
      if(!(rStop->valid())){
  	std::cerr << " FsmState::Stopping validadity fails : rStop->valid() " << rStop->valid() << std::endl;
  	isGood = false;
  	rStop->print();
  	std::cout << std::endl;
      }
    }
    //Else we have an event record 
    else{
      //Increment event counter and reset error state
      nEvents++;

      //if(nEvents>=2) continue;
      
      const Hgcal10gLinkReceiver::SlinkBoe *boe = rEvent->slinkBoe();      
      if (nEvents < 2) {
  	//const uint64_t *p64(((const uint64_t*)rEvent)+1);
  	// std::cout << std::hex << std::setfill('0');
  	// std::cout << "Word 0 0x" << std::setw(16) << p64[0] << std::endl;
  	// std::cout << std::dec << std::setfill(' ');
	
  	//cout<<"Event : " <<nEvents<<" and boe::eventID : " <<boe->eventId()<<endl;
  	//cout<<"\n MiniDAQ header for Slink 0 LSB : " << endl;
  	// for(int istc = 0 ; istc < 12 ; istc++){
  	//   int shift = istc*3;
  	//   cout<<istc<< ", packet status : " << (p64[0]>>shift & 0x7) <<endl;
  	// }
	
  	// std::cout << std::hex << std::setfill('0');
  	// cout<<"OC : 0x"<< (p64[0]>>(12*3) & 0x7) << endl;
  	// cout<<"EC : 0x"<< (p64[0]>>39 & 0x3F) << endl;
  	// cout<<"BC : 0x"<< (p64[0]>>45 & 0xFFF) << endl;
  	// std::cout << std::dec << std::setfill(' ');

  	// 	std::cout << std::hex << std::setfill('0');
  	// 	std::cout << "Word 0 0x" << std::setw(16) << p64[1] << std::endl;
  	// 	std::cout << std::dec << std::setfill(' ');
	

  	// 	cout<<"MiniDAQ header for Slink 1 MSB : " << endl;
  	// 	for(int istc = 0 ; istc < 12 ; istc++){
  	// 	  int shift = istc*3;
  	// 	  cout<<istc<< ", packet status : " << (p64[1]>>shift & 0x7) <<endl;
  	// false;
  	// 	}
  	// 	std::cout << std::hex << std::setfill('0');
  	// 	cout<<"OC : 0x"<< (p64[1]>>(12*3) & 0x7) << endl;
  	// 	cout<<"EC : 0x"<< (p64[1]>>39 & 0x3F) << endl;
  	// 	cout<<"BC : 0x"<< (p64[1]>>45 & 0xFFF) << endl;
  	// 	std::cout << std::dec << std::setfill(' ');

  	//boe->print();
  	const Hgcal10gLinkReceiver::BePacketHeader *beheader = rEvent->bePacketHeader();
  	//beheader->print();
  	const Hgcal10gLinkReceiver::SlinkEoe *eoe = rEvent->slinkEoe();
  	//eoe->print();
  	//std::cout << std::endl;
  	//event_dump(rEvent);
  	// event_dump_32(rEvent, false);
  	// event_dump_32(rEvent, true);

      }
      uint16_t l1atype = boe->l1aType();      
      if(l1atype==0x0001)
      	total_phys_events++;
      if(l1atype==0x0003)
      	total_coinc_events++;
      nTrigType = 3; //0 inclusive, 1: physics, 2: coincident
      
      econt_event ev;
      ev.eventId = boe->eventId();
      ev.l1aType = boe->l1aType();
      const uint64_t *p64(((const uint64_t*)rEvent)+1);
      
      for(int istc = 0 ; istc < 12 ; istc++){
  	int shift = istc*3;
  	ev.ECONT_packet_status[0][istc] = (p64[0]>>shift & 0x7);
  	ev.ECONT_packet_status[1][istc] = (p64[1]>>shift & 0x7);
  	ev.ECONT_packet_validity[0][istc] = (ev.ECONT_packet_status[0][istc]==0x0) ? true : false;
  	ev.ECONT_packet_validity[1][istc] = (ev.ECONT_packet_status[1][istc]==0x0) ? true : false;
  	if(ev.ECONT_packet_validity[0][istc] == false){
  	  isGood = false;
  	}
  	if(ev.ECONT_packet_validity[1][istc] == false and !skipMSB){
  	  isGood = false;
  	}
	
      }
      ev.OC[0] = (p64[0]>>(12*3) & 0x7) ;
      ev.OC[1] = (p64[1]>>(12*3) & 0x7) ;
      ev.EC[0] = (p64[0]>>39 & 0x3F) ;
      ev.EC[1] = (p64[1]>>39 & 0x3F) ;
      ev.BC[0] = (p64[0]>>45 & 0xFFF) ;
      ev.BC[1] = (p64[1]>>45 & 0xFFF) ;

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
      
      if(daq0_event_size != ev.size_in_cafe[0]){
  	std::cerr << "Event : "<< ev.eventId << " Event size do not match between trigger RO header and first 0xfecafe word" << std::endl;
  	isGood = false;
      }
      if(daq1_event_size != ev.size_in_cafe[1]){
  	std::cerr << "Event : "<< ev.eventId << " Event size do not match between trigger RO header and second 0xfecafe word" << std::endl;
  	isGood = false;
      }
      if(daq2_event_size != ev.size_in_cafe[2]){
  	std::cerr << "Event : "<< ev.eventId << " Event size do not match between trigger RO header and third 0xfecafe word" << std::endl;
  	isGood = false;
      }
      if(daq3_event_size != ev.size_in_cafe[3]){
  	std::cerr << "Event : "<< ev.eventId << " Event size do not match between trigger RO header and fourth 0xfecafe word" << std::endl;
  	isGood = false;
      }
      if(ev.daq_nbx[0]!=ev.daq_nbx[1]){
  	std::cerr << "Event : "<< ev.eventId << " Bx size do not match between packed and unpacked data" << std::endl;
  	isGood = false;
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
      	// if (nEvents < 2) {
      	//   for(int istc=0;istc<12;istc++){
      	//     std::cout<<packet_energies[istc]<<" ";
      	//   }
      	//   std::cout<<std::endl;
      	// }
      	//if(boe->l1aType()==0x3){
      	//std::cout<<" boe->l1aType() : " << boe->l1aType() << std::endl;
      	//boe->print();
	
      	for(int istc=0;istc<12;istc++){
	  
      	  hloc[0][0][istc]->Fill(packet_locations[istc]);
	  if(l1atype==0x0001)
	    hloc[1][0][istc]->Fill(packet_locations[istc]);
	  if(l1atype==0x0003)
	    hloc[2][0][istc]->Fill(packet_locations[istc]);
	  
      	  energy_raw[0][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_energies[istc];
  	  ev.energy_raw[0][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_energies[istc];
  	  ev.loc_raw[0][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_locations[istc];
      	}
	
  	const uint32_t word1 = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	const uint32_t bx_counter1 = ( word1 >> 28 ) & 0xF;
	
      	packet[0] = ((p64[i] >> 32) & 0xFFFFFFFF) ;
      	packet[1] = ((p64[i+1] >> 32) & 0xFFFFFFFF) ;
      	packet[2] = ((p64[i+2] >> 32) & 0xFFFFFFFF) ;
      	packet[3] = ((p64[i+3] >> 32) & 0xFFFFFFFF) ;
	
      	set_packet_locations(packet_locations, packet);
      	set_packet_energies(packet_energies, packet);
      	// if (nEvents < 2) {
  	//   std::cout<<"\t";
      	//   for(int istc=0;istc<12;istc++){
      	//     std::cout<<packet_energies[istc]<<" ";
      	//   }
      	//   std::cout<<std::endl;
      	// }
	
      	for(int istc=0;istc<12;istc++){
	  
      	  hloc[0][1][istc]->Fill(packet_locations[istc]);
	  if(l1atype==0x0001)
	    hloc[1][1][istc]->Fill(packet_locations[istc]);
	  if(l1atype==0x0003)
	    hloc[2][1][istc]->Fill(packet_locations[istc]);
	  
      	  energy_raw[1][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_energies[istc];
  	  ev.energy_raw[1][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_energies[istc];
  	  ev.loc_raw[1][(bx_index+int(ev.daq_nbx[0]))][istc] = packet_locations[istc];
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
  	  }
      
      int index_stc = 0;
      int index_ibx = 0;
      uint32_t prev_bx_counter = 0xFF;

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
	
      	// std::cout << "Unpackaer output  0x" << std::setw(8) << word << " bx_counter 0x" << std::setw(1) << bx_counter
      	//   // << " bx_counter1 0x" << std::setw(1) << bx_counter1 
      	// 	  << std::dec << std::setfill(' ') 
      	// 	  << " energy1 loc : " << std::setw(2) << energy1_loc << " energy1 :" << std::setw(2) << energy1
      	// 	  << " energy2 loc : " << std::setw(2) << energy2_loc << " energy2 :" << std::setw(2) << energy2
      	//   // << " energy3 loc : " << std::setw(2) << energy3_loc << " energy3 :" << std::setw(2) << energy3
      	//   // << " energy4 loc : " << std::setw(2) << energy4_loc << " energy4 :" << std::setw(2) << energy4
      	// 	  << std::endl ;	    
	
      	if(bx_counter != prev_bx_counter){
      	  prev_bx_counter = bx_counter;
      	  if(index_stc!=0)
      	    index_ibx++;
      	  index_stc = 0;
      	}
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

      	index_stc++;
      }

      // if (nEvents < 2) 
      // 	event_dump(rEvent);

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

      for(int iect=0;iect<2;iect++){
      	if(iect==1 and skipMSB) continue;
      	for(int ibx=0;ibx<maxnbx;ibx++){
      	  for(int istc=0;istc<12;istc++){
      	    if(energy_raw[iect][ibx][istc]!=energy_unpkd[iect][ibx][istc]){
      	      std::cout << std::dec << std::setfill(' ');
      	      std::cerr << " Packed and unpacked energies does not match for (event,bx.stc,raw_energy,unpacked_energy) : (" << ev.eventId <<","<< ibx <<","<< istc <<","<< energy_raw[iect][ibx][istc] <<","<< energy_unpkd[iect][ibx][istc] <<") "<< std::endl;
      	      isGood = false;
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
      
    }//loop event  
  }
  delete r;

  
  for (int itrig=0; itrig<nTrigType; itrig++) {
    for (int ietc=0; ietc<2; ietc++) {
      if(ietc==1 and skipMSB) continue;
      for (int i=0;i<12;i++) {
	for(int ibin=1;ibin<=hloc[itrig][ietc][i]->GetNbinsX();ibin++){
	  if(hloc[itrig][ietc][i]->GetBinContent(ibin)==0.0){
	    if(itrig==0)
	      std::cerr << "No Trig: Empty TC " << ibin <<" for STC "<< i <<" for ECONT"<< ietc<< "."<< std::endl;
	    if(itrig==1)
	      std::cerr << "Phys Trig: Empty TC " << ibin <<" for STC "<< i <<" for ECONT"<< ietc<<"."<< std::endl;
	    if(itrig==2)
	      std::cerr << "Coincident Trig: Empty TC " << ibin <<" for STC "<< i <<" for ECONT"<< ietc<<"."<< std::endl;
	    isGood = false;
	  }
	}
      }
    }
  }
  
  if(isGood){
    std::cout << "All fine, no errors found." << std::endl;
  }

  //econt_events.clear();
  delete hloc;

  return 0;
}
