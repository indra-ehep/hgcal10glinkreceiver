/*
 g++ -I hgcal10glinkreceiver hgcal10glinkreceiver/offline/src/BasicRunFileCheck.cpp -o bin/BasicRunFileCheck.exe
*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <cassert>
#include <sys/types.h>
#include <getopt.h>

#include "FileReader.h"
#include "RecordHeader.h"

using namespace Hgcal10gLinkReceiver;

int main(int argc, char** argv) {
  if(argc<3) {
    std::cerr << argv[0] << ": no run number specified" << std::endl;
    return 1;
  }
  
  unsigned runNumber(0);
  unsigned linkNumber(0);
  unsigned fileNumber(0);
  unsigned packetNumber(0);
  unsigned i(0),j(0);
  
  uint64_t initialLF(0);
  const uint64_t maskLF(0xff00ffffffffffff);
  
  // Handle run number
  std::istringstream issRun(argv[1]);
  issRun >> runNumber;
  std::istringstream issLink(argv[2]);
  issLink >> linkNumber;

  FileReader _fileReader;
  RecordHeader *h;
  _fileReader.open(runNumber,linkNumber,true);
  while(_fileReader.read(h)) {
    h->print();
  }
  return 0;
  /*
  
  std::ostringstream ossRun;
  ossRun << std::setfill('0') << std::setw(9) << runNumber;
  
  std::string sRun("Run"); // FIX!!!
  sRun+=ossRun.str()+"_File";
  




  bool doPrint(false);
  */
  uint64_t p64[512*16];

  uint64_t predictedPH(0);
  uint64_t maskPH(0xff00ffffffffffff);

  bool endOfRun(false);
  
  while(!endOfRun) {
    /*
    // Handle file number
    std::ostringstream ossFile;
    ossFile << std::setfill('0') << std::setw(9) << fileNumber;
    */
    //std::cout << "Directly reading " << sRun+ossFile.str()+".bin" << std::endl;

    // Open file
    std::ifstream fin;

    // tellg, seekg?
    
    unsigned p(0);
    unsigned length;
    bool endOfFile(false);

    // Read starting packet
    bool startOfFile(true);
    
    while(!endOfFile && !endOfRun) {
      if(startOfFile) {
	fin.open(RunFileName(runNumber,linkNumber,fileNumber).c_str(),std::ios::binary);
	if(!fin) {
	  std::cerr << argv[0] << ": cannot open file " << RunFileName(runNumber,0,fileNumber) << std::endl;
	  return 2;
	}
      }
      
      assert(fin);

      fin.read((char*)p64,8);
      if(startOfFile) {
	if(fileNumber==0) assert((p64[0]>>56)==0xbb);
	else              assert((p64[0]>>56)==0xdd);
      }
      
      if((p64[0]>>56)!=0xac) {
	std::cout << "  Word " << std::setw(10) << j << ", data = 0x"
		  << std::hex << std::setfill('0')
		  << std::setw(16) << p64[0]
		  << std::dec << std::setfill(' ')
		  << std::endl;
      } else {
	if(predictedPH==0) {
	  predictedPH=p64[0]+1;
	  
	} else {
	  if((p64[0]&maskPH)!=(predictedPH&maskPH)) {
	    std::cout << "  Packet header " << std::setw(10) << j << ", data = 0x"
		      << std::hex << std::setfill('0')
		      << std::setw(16) << p64[0] << ", predicted = 0x"
		      << std::setw(16) << predictedPH
		      << std::dec << std::setfill(' ');

	    if((p64[0]&maskPH)>(predictedPH&maskPH)) {
	      std::cout << ", difference = " << (p64[0]&maskPH)-(predictedPH&maskPH) << std::endl;
	    } else {
	      std::cout << ", difference = -" << (predictedPH&maskPH)-(p64[0]&maskPH) << std::endl;
	    }
	  }
	  predictedPH=p64[0]+1;
	}
      }
      
      j++;
      length=(p64[0]>>48)&0xff;
      fin.read((char*)(p64+1),8*length);
      j+=length;

      startOfFile=false;

      if((p64[0]>>56)==0xcc) {
	fin.close();

	startOfFile=true;
	fileNumber++;
      } else {
      }
      
      if((p64[0]>>56)==0xee) {
	fin.close();
	endOfRun=true;
      }

    }

  }
  /*
  if(fileNumber==0) assert((p64[0]>>56)==0xbb);
    else              assert((p64[0]>>56)==0xcc);
    
    
    fin.read((char*)(p64+1),8);
    j++;
    
    std::cout << "Run " << (p64[1]>>32)
	      << ", UTC " << (p64[1]&0xffffffff)
	      << ", file number = " << fileNumber
	      << std::endl;
    
    // Events
    
    fin.read((char*)p64,8);
    j++;
    
    if(fileNumber==0) initialLF=p64[0];
    assert((p64[0]>>56)==0xac);
    
    while(fin && !endOfFile && !endOfRun) {
      if(doPrint) std::cout << "New  Word " << std::setw(6) << 0 << ", data = 0x"
			    << std::hex << std::setfill('0')
			    << std::setw(16) << p64[0]
			    << std::dec << std::setfill(' ')
			    << std::endl;
      
      assert((p64[0]>>56)==0xac);
      
      if((p64[0]&maskLF)!=((initialLF+packetNumber)&maskLF)) {
	std::cout << "Bad  Word " << std::setw(6) << 0 << ", data = 0x"
		  << std::hex << std::setfill('0')
		  << std::setw(16) << p64[0] << ", expected = 0x"
		  << std::setw(16) << initialLF+packetNumber
		  << std::dec << std::setfill(' ')
		  << ", difference = " << (p64[0]&maskLF)-((initialLF+packetNumber)&maskLF)
		  << std::endl;
	
	initialLF=p64[0]-packetNumber;
      }
      
      //assert((p64[0]&maskLF)==((initialLF+p)&maskLF));
      packetNumber++;

      unsigned length((p64[0]>>48)&0xff);
  
      std::cout << "Length = " << length << " words, "
		<< "  Word " << std::setw(6) << 0 << ", data = 0x"
		<< std::hex << std::setfill('0')
		<< std::setw(16) << p64[0]
		<< std::dec << std::setfill(' ')
		<< std::endl;
      
      fin.read((char*)(p64+1),8*length);
      j+=length;
      
      if(doPrint || j< 500) {
	for(unsigned i(1);i<=length;i++) {
	  std::cout << "  Word " << std::setw(6) << i << ", data = 0x"
		    << std::hex << std::setfill('0')
		    << std::setw(16) << p64[i]
		    << std::dec << std::setfill(' ')
		    << std::endl;
	}
      }
      
      fin.read((char*)p64,8);
      j++;
      if((p64[0]>>56)==0xcc) endOfFile=true;
      if((p64[0]>>56)==0xee) endOfRun=true;
    }
    
    length=(p64[0]>>48)&0xff;
    assert(length==1);
    
    fin.read((char*)(p64+1),8);
    j++;
    
    std::cout << "Run " << (p64[1]>>32)
	      << ", UTC " << (p64[1]&0xffffffff)
	      << std::endl;
    
    fin.close();
    fileNumber++;
  }

  std::cout << "Directly read " << j << " words" << std::endl;
  */
  return 0;
}
