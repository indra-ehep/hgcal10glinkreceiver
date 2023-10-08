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

int main(int argc, char** argv){
  
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
  
  int payload_version = 0;
  payload_version = read_Payload_Version(relayNumber);
  cout << "payload_version : " << payload_version << endl;

  cout<<endl;
  for(int i=0;i<80;i++) cout<<"=";
  cout<<endl;
  cout << "Summary of Relay " << relayNumber << " and Run " << runNumber << endl;
  cout<<endl;
  for(int i=0;i<80;i++) cout<<"=";
  cout<<endl;

  cout <<"Relay| Run| PV|"<<endl;
  cout << relayNumber << "|"
       << runNumber << "|"
       << std::hex << std::setfill('0')
       << "0x" << std::setw(4) << payload_version << "|"
       << std::dec << std::setfill(' ')
       <<endl;

  cout<<endl;
  for(int i=0;i<80;i++) cout<<"=";
  cout<<endl;

  return 0;
}
