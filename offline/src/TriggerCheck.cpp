#include <iostream>
#include <iomanip>
#include <cassert>

#include <yaml-cpp/yaml.h>

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

#include "TFileHandler.h"
#include "FileReader.h"

int main(int argc, char** argv) {
  if(argc<2) {
    std::cerr << argv[0] << ": no relay and/or run numbers specified" << std::endl;
    return 1;
  }

  // Handle run number
  unsigned relayNumber(0);
  std::istringstream issRelay(argv[1]);
  issRelay >> relayNumber;

  unsigned runNumber(relayNumber);
  if(argc>2) {
    std::istringstream issRun(argv[2]);
    issRun >> runNumber;
  }
  
  if(relayNumber==0 || runNumber==0) {
    std::cerr << argv[0] << ": relay and/or run numbers uninterpretable" << std::endl;
    return 2;
  }

  std::ostringstream sout;
  sout << "TriggerCheck_Relay" << relayNumber;
  TFileHandler tfh(sout.str().c_str());

  TH1D *hScintFirstWord,*hScintNwords;    
  TH2D *hScintWord;

  TH1D *hPayloadLength,*hSequence,*hSubpackets,*hSubpacketCount,*hEvents,*hScintLength,*hScintLengthS,*hScintLengthE,*hNtrains,*hScintFirst;
  TH2D *hPayloadLengthVsSeq,*hSubpacketCountVsPl,*hSequenceVsNe;
  TH2D *hStcVsCpd[2][12],*hToaVsCpd,*hUStcVsCpd[2][12],*hUStcVsCpdM2[2][12],*hScint,*hTpVsCpd[2],*hScintLengthVsFirst;
  TProfile *pUStcVsCpd[2];



  hScintFirstWord=new TH1D("ScintFirstWord",";Scintillator first word location;Number of events",
			   256,0,256);
  hScintNwords=new TH1D("ScintNwords",";Scintillator number of words;Number of events",
			256,0,256);
  hScintWord=new TH2D("ScintWord",";Scintillator word type;Number of events",
		      256,0,256,4,0,4);


  hScint=new TH2D("Scint",";Scintillator trigger delay;Scintillator bits;Number of events",
                              200,0,200,32,0,32);
  hScintFirst=new TH1D("ScintFirst",";Scintillator rising edge;Number of events",
                              32*9,0,32*9);

  hNtrains=new TH1D("Ntrains",";Scintillator trigger number of trains;Number of events",
                    100,0,100);
  hScintLength=new TH1D("ScintLength",";Scintillator trigger train length (units of 0.8ps);Number of events",
                        9*32,0,9*32);
  hScintLengthS=new TH1D("ScintLengthS",";Starting scintillator trigger train length (units of 0.8ps);Number of events",
                        9*32,0,9*32);
  hScintLengthE=new TH1D("ScintLengthE",";Ending scintillator trigger train length (units of 0.8ps);Number of events",
                        9*32,0,9*32);

  hScintLengthVsFirst=new TH2D("hScintLengthVsFirst",";Scintillator train rising edge;Scintillator train length;Number of events",
			       32*9,0,32*9,200,0,200);

  
  // Create the file reader
  Hgcal10gLinkReceiver::FileReader _fileReader;

  // Make the buffer space for the records
  Hgcal10gLinkReceiver::RecordT<4095> *r(new Hgcal10gLinkReceiver::RecordT<4095>);

  // Set up specific records to interpet the formats
  const Hgcal10gLinkReceiver::RecordStarting *rStart((Hgcal10gLinkReceiver::RecordStarting*)r);
  const Hgcal10gLinkReceiver::RecordStopping *rStop ((Hgcal10gLinkReceiver::RecordStopping*)r);
  const Hgcal10gLinkReceiver::RecordRunning  *rEvent((Hgcal10gLinkReceiver::RecordRunning*) r);

  // Defaults to the files being in directory "dat"
  // Can call setDirectory("blah") to change this
  //_fileReader.setDirectory("somewhere/else");
  _fileReader.setDirectory(std::string("dat/Relay")+argv[1]);
  _fileReader.openRun(runNumber,0);
  //_fileReader.openRun(runNumber,1);
  
  //bool anyError(false);
  unsigned nEvents(0);//,nErrors[10]={0,0,0,0,0,0,0,0,0,0};
  /*
  unsigned bc(0),oc(0),ec(0),seq(0);
  unsigned ocOffset(0);

  bool noCheck(false);
  uint32_t initialSeq;

  
  bool doubleEvents(false);
  unsigned nEventsPerOrbit(1);
  */

  std::ofstream fout("Aidan.csv");
  
  
  while(_fileReader.read(r)) {
    if(       r->state()==Hgcal10gLinkReceiver::FsmState::Starting) {
      rStart->print();
      std::cout << std::endl;

    } else if(r->state()==Hgcal10gLinkReceiver::FsmState::Stopping) {
      rStop->print();
      std::cout << std::endl;

    } else {

	// We have an event record
      nEvents++;
      bool print(nEvents<=1000);// || nEvents>2000);
      //anyError=false;

      const uint64_t *p64(((const uint64_t*)rEvent)+1);

      // Run 1691537835 only
      uint64_t RaghuWord(p64[ 96]);
      uint64_t AidanWord(p64[145]);

      const Hgcal10gLinkReceiver::SlinkBoe *b(rEvent->slinkBoe());
      assert(b!=nullptr);
      const Hgcal10gLinkReceiver::SlinkEoe *e(rEvent->slinkEoe());
      assert(e!=nullptr);

      fout << std::setw(4) << e->bxId() << "," << std::setw(10) << e->orbitId()
		<< "," << std::setw(10) << b->eventId() << ",0x"
		<< std::hex << std::setfill('0')
		<< std::setw(8) << (AidanWord>>32) << ",0x"
		<< std::setw(8) << (RaghuWord&0xffffffff)
	   << std::dec << std::setfill(' ') << std::endl;

      
      // Scintillator
      unsigned nWord(0),nFirstWord(0),nLastWord(0);
      for(unsigned i(0);i<rEvent->payloadLength();i++) {
	if((p64[i]>>32)==0xabcdfeed) {
	  if(nWord==0) {
	    nFirstWord=i;
	    hScintFirstWord->Fill(nFirstWord);
	  }
	  
	  uint32_t d(p64[i]&0xffffffff);
	  if(d==0x000000000)  hScintWord->Fill(i,1);
	  else if(d==0xffffffff)  hScintWord->Fill(i,3);
	  else hScintWord->Fill(i,2);

	  nWord++;
	}
      }
      hScintNwords->Fill(nWord);


      unsigned nCentre(nFirstWord-2+nWord/2);

      std::cout << "nCentre = " << nCentre << std::endl;
      if(nCentre<300) {
      bool inTrain(false);
        unsigned first,last;
        unsigned nTrains(0);
        
        for(unsigned i(0);i<9;i++) {
          //std::cout << "Scint Word " << i << " = " << std::hex << std::setw(16) << p64[117-20+5*i] << std::dec << std::endl;
          uint64_t pTemp=p64[nCentre-20+5*i]&0xffffffff;
          //uint64_t pTemp=p64[nCentre-20+5*i];
          
          for(unsigned j(0);j<32;j++) {
            //std::cout << "Scint Word in loop" << i << " = " << std::hex << std::setw(16) << pTemp << std::dec << std::endl;
            
            bool b((pTemp&uint64_t(1<<(31-j)))!=0);

            //std::cout << "Bit " << 31-j << " = " << std::setw(2) << (b?"True":"False") << ", mask "
            //        << std::hex << std::setw(16) << (1<<(31-j)) << std::dec << std::endl;
            
            if(b && !inTrain) {
              first=32*i+j;
              inTrain=true;
              nTrains++;
            }

            if(b) last=32*i+j;

            if(!b && inTrain) {
              inTrain=false;

              //if(last==first) std::cout << "F=L word = " << std::hex << std::setw(16) << p64[nCentre-20+5*i] << std::dec << std::endl;

              if(first==0) hScintLengthS->Fill(last-first+1);
              else {
                hScintFirst->Fill(first);
                hScintLength->Fill(last-first+1);
		hScintLengthVsFirst->Fill(first,last-first+1);
	      }
            }

            //std::cout << "InTrain = " << (inTrain?"True":"False") << std::endl;
          }
        }

        if(inTrain) hScintLengthE->Fill(last-first+1);
        //if(nTrains>0) std::cout << "Number of trains = " << nTrains << std::endl;
        hNtrains->Fill(nTrains);
      }

	
      
      if(print) {
	std::cout << std::endl;
	rEvent->print();

	// Access the Slink header ("begin-of-event")
	// This should always be present; check pattern is correct
	const Hgcal10gLinkReceiver::SlinkBoe *b(rEvent->slinkBoe());
	assert(b!=nullptr);
	//if(b->l1aType()==0x001c) b->print();
      
	// Access the Slink trailer ("end-of-event")
	// This should always be present; check pattern is correct
	const Hgcal10gLinkReceiver::SlinkEoe *e(rEvent->slinkEoe());
	assert(e!=nullptr);
	

	b->print();
	e->print();
      
	  std::cout << "Record " << nEvents << std::endl;
	  //rEvent->RecordHeader::print();

	  for(unsigned i(0);i<rEvent->payloadLength();i++) {
	    std::cout << "Word " << std::setw(3) << i << " ";
	    std::cout << std::hex << std::setfill('0');
	    std::cout << "0x" << std::setw(16) << p64[i] << std::endl;
	    std::cout << std::dec << std::setfill(' ');
	  }
	  std::cout << std::endl;
	  /*
	  for(unsigned i(0);i<2*rEvent->payloadLength()-4U;i++) {
	    std::cout << "Word " << std::setw(3) << i << " ";
	    std::cout << std::hex << std::setfill('0');
	    std::cout << "0x" << std::setw(8) << p32[i] << std::endl;
	    std::cout << std::dec << std::setfill(' ');
	  }

	  std::cout << std::endl;
	  */

      }
    }
  }   

  std::cout << "Total number of event records seen = "
	    << nEvents << std::endl;
  /*
  for(unsigned i(0);i<10;i++) {
    std::cout << "Total number of event records with error " << i << " = "
	      << nErrors[i] << std::endl;
  }
  std::cout << "Final OC offset = " << ocOffset << std::endl;
  */

  fout.close();
  delete r;

  return 0;
}
