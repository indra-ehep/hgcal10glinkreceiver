#ifndef Hgcal10gLinkReceiver_FileReader_h
#define Hgcal10gLinkReceiver_FileReader_h

#include <iostream>
#include <fstream>
#include <sstream>

#include "FileContinuationCloseRecord.h"
#include "FileContinuationOpenRecord.h"
//#include "FileNamer.h"
#include "RecordPrinter.h"

namespace Hgcal10gLinkReceiver {

  class FileReader {
  public:
    
  FileReader() : _directory("dat/") {
    }

    void setDirectory(const std::string &s) {
      _directory=s+"/";
    }

    bool openRelay(uint32_t r) {
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
      
      if(_superRun) _fileName=setRelayFileName(r);
      else          _fileName=setRunFileName(r,l,_fileNumber);

      _inputFile.open((_directory+_fileName).c_str(),std::ios::binary);
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
	setRunFileName(_runNumber,_linkNumber,_fileNumber);
	
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

    static std::string setRelayFileName(uint32_t r) {
      std::ostringstream sRelayFileName;
      sRelayFileName << std::setfill('0')
		   << "Relay" << std::setw(10) << r
		   << ".bin";
      return sRelayFileName.str();
    }
    
    static std::string setRunFileName(uint32_t r, uint32_t l, uint32_t f) {
      std::ostringstream sRunFileName;
      sRunFileName << std::setfill('0')
		   << "Run" << std::setw(10) << r
		   << "_Link" << l
		   << "_File" << std::setw(10) << f
		   << ".bin";
      return sRunFileName.str();
    }
    
    protected:
    bool _superRun;
    uint32_t _runNumber;
    uint32_t _linkNumber;
    uint32_t _fileNumber;
    
    std::string _directory;
    std::string _fileName;
    std::ifstream _inputFile;
  };
}

#endif
