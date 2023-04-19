#ifndef Hgcal10gLinkReceiver_FileWriter_h
#define Hgcal10gLinkReceiver_FileWriter_h

#include <iostream>
#include <fstream>

#include "FileContinuationCloseRecord.h"
#include "FileContinuationOpenRecord.h"

namespace Hgcal10gLinkReceiver {

  class FileWriter : public FileNamer {
  public:
    enum {
      MaximumBytesPerFile=2000000000
    };
    
    FileWriter() : _protectFiles(false) { // While debugging
    }

    bool open(uint32_t r, uint32_t l, bool s=false, bool w=true) {
      _runNumber=r;
      _linkNumber=l;
      _superRun=s;
      _fileNumber=0;
      
      setRunFileName();

      _writeEnable=w;
      _numberOfBytesInFile=0;

      if(_writeEnable) {
	_outputFile.open(_fileName.c_str(),std::ios::binary);
	return _outputFile;
      }
      return true;
    }

    bool write(uint64_t *d, unsigned n) {
      if(_writeEnable) {
	_outputFile.write((char*)d,8*n);
      }

      _numberOfBytesInFile+=8*n;

      if(_numberOfBytesInFile>MaximumBytesPerFile) {
	FileContinuationCloseRecord fccr;
	fccr.setRunNumber(_runNumber);
	fccr.setFileNumber(_fileNumber);

	if(_writeEnable) {
	  _outputFile.write((char*)(&fccr),sizeof(FileContinuationCloseRecord));
	  _outputFile.close();
	  
	  if(_protectFiles) {
	    if(system((std::string("chmod 444 ")+_fileName).c_str())!=0) return 1;
	  }
	}

	_fileNumber++;
	setRunFileName();

	FileContinuationOpenRecord fcor;
	fcor.setRunNumber(_runNumber);
	fcor.setFileNumber(_fileNumber);

	if(_writeEnable) {
	  _outputFile.open(_fileName.c_str(),std::ios::binary);
	  _outputFile.write((char*)(&fcor),sizeof(FileContinuationOpenRecord));
	}

	_numberOfBytesInFile=sizeof(FileContinuationOpenRecord);
      }
      
      return true;
    }
    
    bool close() {
      _outputFile.close();
      if(_protectFiles) {
	if(system((std::string("chmod 444 ")+_fileName).c_str())!=0) return 1;
      }
      return true;
    }

    bool write(const RecordHeader &h) {
      write((char*)(&h),8*(h.length()+1));
    }
    
  private:
    std::ofstream _outputFile;

    //std::string _fileName;
    //unsigned _fileNumber;

    bool _writeEnable;
    unsigned _numberOfBytesInFile;
    bool _protectFiles;
  };

}

#endif
