#ifndef Hgcal10gLinkReceiver_FileReader_h
#define Hgcal10gLinkReceiver_FileReader_h

#include <iostream>
#include <fstream>

#include "FileContinuationCloseRecord.h"
#include "FileContinuationOpenRecord.h"
#include "FileNamer.h"

namespace Hgcal10gLinkReceiver {

  class FileReader : public FileNamer {
  public:
    
    FileReader() {
    }

    bool openSuperRun(uint32_t r) {
      return open(r,2,true);
    }

    bool openRun(uint32_t r, uint32_t l) {
      return open(r,l,false);
    }

    bool open(uint32_t r, uint32_t l, bool s=false) {
      _runNumber=r;
      _linkNumber=l;
      _superRun=s;
      _fileNumber=0;
      
      setRunFileName();

      _inputFile.open(_fileName.c_str(),std::ios::binary);
      return (_inputFile?true:false);
    }

    //bool read(uint64_t *d, unsigned n) {
    bool read(RecordHeader *h) {
      _inputFile.read((char*)h,8);
      if(!_inputFile) return false;
      _inputFile.read((char*)(h+1),8*h->payloadLength());
      
      //_inputFile.read((char*)d,8);
      //_inputFile.read((char*)(d+1),8*);
      
      if(h->identifier()==RecordHeader::FileContinuationEof) {
	
	_inputFile.close();
	
	_fileNumber++;
	setRunFileName();
	
	_inputFile.open(_fileName.c_str(),std::ios::binary);

	_inputFile.read((char*)h,8);
	assert(h->identifier()==RecordHeader::FileContinuationBof);
	_inputFile.read((char*)(h+1),8*h->payloadLength());

	_inputFile.read((char*)h,8);
      h->print();
	_inputFile.read((char*)(h+1),8*h->payloadLength());
      }
      
      return true;
    }
    
    bool close() {
      _inputFile.close();
      return true;
    }

    bool closed() {
      if(_inputFile) return false;      
      return true;
    }

    //bool read(RecordHeader *h) {
    //  _inputFile.read((char*)h,8);
    //  _inputFIle.read((char*)(h+1),8*h->payloadLength());
    //}
    
  private:
    std::ifstream _inputFile;
  };

}

#endif
