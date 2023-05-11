#ifndef Hgcal10gLinkReceiver_FileWriter_h
#define Hgcal10gLinkReceiver_FileWriter_h

#include <iostream>
#include <fstream>

//#include "FileContinuationCloseRecord.h"
//#include "FileContinuationOpenRecord.h"
#include "FileReader.h"

namespace Hgcal10gLinkReceiver {

  class FileWriter {
  public:
    enum {
      MaximumBytesPerFile=2000000000
      //MaximumBytesPerFile=2000
    };
    
  FileWriter() : _directory("dat/"), _protectFiles(false) {
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
      _relay=s;
      _fileNumber=0;
      
      if(_relay) _fileName=FileReader::setRelayFileName(_runNumber);
      else       _fileName=FileReader::setRunFileName(_runNumber,_linkNumber,_fileNumber);

      _writeEnable=(r<0xffffffff);
      _numberOfBytesInFile=0;

      if(_writeEnable) {
	std::cout << "FileWriter::open() opening file "
		  << _fileName.c_str() << std::endl;
	_outputFile.open(_directory+_fileName.c_str(),std::ios::binary);
	if(!_outputFile) return false;
      }
      return true;
    }

    //bool write(uint64_t *d, unsigned n) {
    bool write(const Record* h) {
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
	_fileName=FileReader::setRunFileName(_runNumber,_linkNumber,_fileNumber);
	  
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
	std::cout << "FileWriter::close() closing file "
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
    std::string _directory;
    std::string _fileName;
    std::ofstream _outputFile;

    bool _writeEnable;
    bool _protectFiles;

    bool _relay;
    unsigned _runNumber;
    unsigned _linkNumber;
    unsigned _fileNumber;
    unsigned _numberOfBytesInFile;
  };

}

#endif
