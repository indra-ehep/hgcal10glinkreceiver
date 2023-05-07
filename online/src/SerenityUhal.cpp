#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <iostream>
#include <cassert>

#include "SerenityUhal.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char *argv[]) {

  // Turn off printing
  bool printEnable(false);

  for(int i(1);i<argc;i++) {
    if(std::string(argv[i])=="-p" ||
       std::string(argv[i])=="--printEnable")
      std::cout << "Setting printEnable to true" << std::endl;
    printEnable=true;
  }

  SerenityUhal::setUhalLogLevel();

  SerenityUhal su;
  su.makeTable();
  su.setDefaults();
  //su.uhalWrite("BLAH",0xdead);

  su.uhalWrite("lpgbt1.lpgbt_frame.shift_elink4",57&0xff);
  su.print();

  const unsigned offset(27); // RX
  //const unsigned offset(28); // TX

  system("ls -l data/");
  system("rm data/rx_summary.txt; rm data/tx_summary.txt");
  system("source ./emp_capture_single.sh 1");
  //sleep(1);
  system("ls -l data/");

  uint32_t _rxSummaryData[8][128],_phaseData[8]={0,0,0,0,0,0,0,0};
  uint64_t data64[8][127];
  

  std::ifstream fin;
  fin.open("data/rx_summary.txt");
  if(!fin) return false;
      
  char buffer[1024];
  fin.getline(buffer,1024);
  fin.getline(buffer,1024);
  fin.getline(buffer,1024);
  fin.getline(buffer,1024);
      
  std::cout << "rxSummary data" << std::endl;
  for(unsigned i(0);i<128;i++) {
    for(unsigned j(0);j<8;j++) {
      fin.getline(buffer,1024);
      if(buffer[15]=='1') _phaseData[j]++;

      buffer[offset+8]='\0';
      std::istringstream sin(buffer+offset);
      sin >> std::hex >> _rxSummaryData[j][i];

      std::cout << " 0x" << std::hex << std::setfill('0') 
		<< std::setw(8) << _rxSummaryData[j][i]
		<< std::dec << std::setfill(' ');
    }
    std::cout << std::endl;
  }      

  fin.close();

  for(unsigned j(0);j<8;j++) {
    std::cout << "Phase data for j =" << j << " = " << _phaseData[j] << std::endl;

    for(unsigned i(0);i<127;i++) {
      data64[j][i]=uint64_t(_rxSummaryData[j][i])<<32|_rxSummaryData[j][i+1];
      std::cout << " 0x" << std::hex << std::setfill('0')
		<< std::setw(16) << data64[j][i] << ", "
		<< std::setw(16) << (data64[j][i]>>7)
		<< std::dec << std::setfill(' ');
      std::cout << std::endl;
    }
  }

  unsigned ncount[8][256],mcount[8],kcount[8];
  bool _rxSummaryValid[8];




  for(unsigned k(0);k<32;k++) {
  for(unsigned j(0);j<8;j++) {
    ncount[j][k]=0;

    for(unsigned i(0);i<126;i++) {
      if(j==0 && i==10) std::cout << std::hex << ((data64[j][i]>>k)&0xfffffff) << std::endl;
      if(((data64[j][i]>>k)&0xffffffff)==0xaccccccc ||
	 ((data64[j][i]>>k)&0xffffffff)==0x9ccccccc) ncount[j][k]++;
    }
  }
        
  for(unsigned j(0);j<8;j++) {
    if(mcount[j]<ncount[j][k]) {
      mcount[j]=ncount[j][k];
      kcount[j]=k;
    }
  }
  }

for(unsigned j(0);j<8;j++) {

  _rxSummaryValid[j]=(mcount[j]>10 && kcount[j]<256);
 }
      
for(unsigned j(0);j<8;j++) {
  std::cout << "RX channel " << j << ", max matches = " << mcount[j]
	    << " out of 128, for offset " << kcount[j]
	    << ", valid = " << _rxSummaryValid[j] << std::endl;
 }


 su.csv(std::cout);
}
