#ifndef Hgcal10gLinkReceiver_FileWriter_h
#define Hgcal10gLinkReceiver_FileWriter_h

#include <iostream>
#include <fstream>

#include "FileContinuationCloseRecord.h"
#include "FileContinuationOpenRecord.h"
#include "FileNamer.h"

namespace Hgcal10gLinkReceiver {

  class FileWriter : public FileNamer {
  public:
    enum {
      MaximumBytesPerFile=2000000000
      //MaximumBytesPerFile=2000
    };
    
    FileWriter() : _protectFiles(false) { // While debugging
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

      _writeEnable=(r<0xffffffff);
      _numberOfBytesInFile=0;

      if(_writeEnable) {
	std::cout << "FileWrite::open() opening file "
		  << _fileName.c_str() << std::endl;
	_outputFile.open(_fileName.c_str(),std::ios::binary);
	if(!_outputFile) return false;
      }
      return true;
    }

    //bool write(uint64_t *d, unsigned n) {
    bool write(const RecordHeader* h) {
      if(_writeEnable) {
	_outputFile.write((char*)h,8*h->totalLength());
	_outputFile.flush();
      }

      _numberOfBytesInFile+=8*h->totalLength();

      if(_numberOfBytesInFile>MaximumBytesPerFile) {
	FileContinuationCloseRecord fccr;
	//fccr.setRunNumber(_runNumber);
	//fccr.setFileNumber(_fileNumber);

	if(_writeEnable) {
	  _outputFile.write((char*)(&fccr),sizeof(FileContinuationCloseRecord));
	  std::cout << "FileWrite::write() closing file "
		    << _fileName.c_str() << std::endl;
	  _outputFile.close();
	  
	  if(_protectFiles) {
	    if(system((std::string("chmod 444 ")+_fileName).c_str())!=0) return 1;
	  }
	}

	_fileNumber++;
	setRunFileName();

	FileContinuationOpenRecord fcor;
	//fcor.setRunNumber(_runNumber);
	//fcor.setFileNumber(_fileNumber);

	if(_writeEnable) {
	std::cout << "FileWrite::write() opening file "
		  << _fileName.c_str() << std::endl;
	  _outputFile.open(_fileName.c_str(),std::ios::binary);
	  _outputFile.write((char*)(&fcor),sizeof(FileContinuationOpenRecord));
	_outputFile.flush();
	}

	_numberOfBytesInFile=sizeof(FileContinuationOpenRecord);
      }
      
      return true;
    }
    
    bool close() {
      if(_outputFile.is_open()) {
	std::cout << "FileWrite::close() closing file "
		  << _fileName.c_str() << std::endl;
	_outputFile.close();
	if(_protectFiles) {
	  if(system((std::string("chmod 444 ")+_fileName).c_str())!=0) return 1;
	}
      }
      return true;
    }

    //bool write(const RecordHeader &h) {
    //  outputFile_.write((char*)(&h),8*(h.length()+1));
    //}
    
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
