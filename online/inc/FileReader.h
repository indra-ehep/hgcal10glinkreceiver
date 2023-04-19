#ifndef Hgcal10gLinkReceiver_FileReader_h
#define Hgcal10gLinkReceiver_FileReader_h

#include <iostream>
#include <fstream>

#include "FileContinuationCloseRecord.h"
#include "FileContinuationOpenRecord.h"

namespace Hgcal10gLinkReceiver {

  class FileReader : public FileNamer {
  public:
    
    FileReader() {
    }

    bool open(uint32_t r, uint32_t l, bool s=false) {
      _runNumber=r;
      _linkNumber=l;
      _superRun=s;
      _fileNumber=0;
      
      setRunFileName();

      _inputFile.open(_fileName.c_str(),std::ios::binary);
      return _inputFile;
    }

    bool read(uint64_t *d, unsigned n) {
      _inputFile.read((char*)d,8);
      _inputFile.read((char*)(d+1),8*);
      
      _numberOfBytesInFile+=8*n;
      
      if(FCCR) {
	FileContinuationCloseRecord fccr;
	fccr.setRunNumber(_runNumber);
	fccr.setFileNumber(_fileNumber);
	
	_inputFile.close();
	
	_fileNumber++;
	setRunFileName();
	
	_inputFile.open(_fileName.c_str(),std::ios::binary);
	
	
	FileContinuationOpenRecord fcor;
	fcor.setRunNumber(_runNumber);
	fcor.setFileNumber(_fileNumber);
	
	_inputFile.read((char*)(&fcor),sizeof(FileContinuationOpenRecord));
      }
      
      
      _inputFile.read((char*)(&fcor),sizeof(FileContinuationOpenRecord));
      
      return true;
    }
    
    bool close() {
      _inputFile.close();
      return true;
    }

    bool read(RecordHeader *h) {
      _inputFile.read((char*)h,8);
      _inputFIle.read((char*)(h+1),8*h->length());
    }
    
  private:
    std::ifstream _inputFile;
  };

}

#endif
