#ifndef Hgcal10gLinkReceiver_FileWriter_h
#define Hgcal10gLinkReceiver_FileWriter_h

#include <stdio.h>
#include <iostream>
#include <fstream>

//#include "FileContinuationCloseRecord.h"
//#include "FileContinuationOpenRecord.h"
#include "FileReader.h"

namespace Hgcal10gLinkReceiver {

  class FileWriter {
  public:
    //enum {
    //MaximumBytesPerFile=2000000000
      ////MaximumBytesPerFile=2000
    //};
    
  FileWriter() : MaximumBytesPerFile(4000000000), _directory("dat/"), _protectFiles(false), _flushRecords(false), _rawCcode(true) {
      
      
    }

    void setDirectory(const std::string &s) {
      _directory=s+"/";
    }

    bool openRelay(uint32_t r) {
      std::cout << "FileWriter HERE1!" << std::endl;
      return open(r,2,true);
    }

    bool openRun(uint32_t r, uint32_t l) {
      std::cout << "FileWriter HERE2!" << std::endl;
      return open(r,l,false);
    }

    bool open(uint32_t r, uint32_t l, bool s=false) {
      std::cout << "FileWriter HERE3!" << std::endl;
      _runNumber=r;
      _linkNumber=l;
      _relay=s;
      _fileNumber=0;
      
      if(_relay) _fileName=FileReader::setRelayFileName(_runNumber);
      else       _fileName=FileReader::setRunFileName(_runNumber,_linkNumber,_fileNumber);

      _writeEnable=(r<0xffffffff);
      _numberOfBytesInFile=0;

      std::cout << "FileWriter HERE!" << std::endl;
      
      if(_writeEnable) {
	std::cout << "FileWriter::open() opening file "
		  << _fileName.c_str() << std::endl;

	if(_rawCcode) {
	  _outputFilePtr=fopen((_directory+_fileName).c_str(),"wb");
	  if (!_outputFilePtr) {
	    perror("File opening failed");
	    return false;
	  }
	  if(_outputFilePtr==nullptr) return false;

	} else {
	  _outputFile.open((_directory+_fileName).c_str(),std::ios::binary);
	  if(!_outputFile) return false;
	}
	
      } else {
	if(_rawCcode) {
	  _outputFilePtr=fopen("/dev/null","wb");
	} else {
	  _outputFile.open("/dev/null",std::ios::binary);
	}
      }

      return true;
    }

    //bool write(uint64_t *d, unsigned n) {
    bool write(const Record* h) {
      std::cout << "FileWriter HERE4!" << std::endl;
      //if(_writeEnable) {
      if(_rawCcode) {
	fwrite((char*)h,1,8*h->totalLength(),_outputFilePtr);
      } else {
	_outputFile.write((char*)h,8*h->totalLength());
	if(_flushRecords) _outputFile.flush();
      }
      //}

      _numberOfBytesInFile+=8*h->totalLength();

      if(_numberOfBytesInFile>MaximumBytesPerFile) {
	RecordContinuing rc;
	rc.setHeader();
	rc.setRunNumber(_runNumber);
	rc.setNumberOfEvents(0xffffffff);

	_numberOfBytesInFile+=sizeof(RecordContinuing);

	rc.setFileNumber(_fileNumber);
	rc.setNumberOfBytes(_numberOfBytesInFile);

	if(_writeEnable) {
	  std::cout << "FileWrite::write() closing file "
		    << _fileName.c_str() << std::endl;

	  if(_rawCcode) {
	    fwrite((char*)(&rc),1,sizeof(RecordContinuing),_outputFilePtr);
	    fclose(_outputFilePtr);
	  } else {
	    _outputFile.write((char*)(&rc),sizeof(RecordContinuing));
	    _outputFile.close();
	  }
	  
	  if(_protectFiles) {
	    if(system((std::string("chmod 444 ")+_fileName).c_str())!=0) return 1;
	  }

	} else {
	  if(_rawCcode) {
	    fclose(_outputFilePtr);
	  } else {
	    _outputFile.close();
	  }
	}

	_fileNumber++;
	_fileName=FileReader::setRunFileName(_runNumber,_linkNumber,_fileNumber);
		
	_numberOfBytesInFile=sizeof(RecordContinuing);

	rc.setFileNumber(_fileNumber);
	rc.setNumberOfBytes(_numberOfBytesInFile);

	if(_writeEnable) {
	  std::cout << "FileWrite::write() opening file "
		    << _fileName.c_str() << std::endl;

	  if(_rawCcode) {
	    _outputFilePtr=fopen((_directory+_fileName).c_str(),"wb");
	    if(_outputFilePtr==nullptr) return false;

	    fwrite((char*)(&rc),1,sizeof(RecordContinuing),_outputFilePtr);
	    
	  } else {
	    _outputFile.open((_directory+_fileName).c_str(),std::ios::binary);
	    
	    _outputFile.write((char*)(&rc),sizeof(RecordContinuing));
	    if(_flushRecords) _outputFile.flush();
	  }
	  
	} else {
	  if(_rawCcode) {
	    _outputFilePtr=fopen("/dev/null","wb");
	  } else {
	    _outputFile.open("/dev/null",std::ios::binary);
	  }
	}
      }
      
      return true;
    }
    
    bool close() {
      if(_rawCcode) {
	fclose(_outputFilePtr);
	if(_protectFiles) {
	  if(system((std::string("chmod 444 ")+_fileName).c_str())!=0) return 1;
	}
	
      } else {
	if(_outputFile.is_open()) {
	  std::cout << "FileWriter::close() closing file "
		    << _fileName.c_str() << std::endl;
	  _outputFile.close();
	
	  if(_protectFiles) {
	    if(system((std::string("chmod 444 ")+_fileName).c_str())!=0) return 1;
	  }
	}
      }
      return true;
    }
    
  private:
    const uint32_t MaximumBytesPerFile;
    
    std::string _directory;
    std::string _fileName;
    
    std::ofstream _outputFile;
    FILE *_outputFilePtr;
    
    bool _writeEnable;
    bool _protectFiles;
    bool _flushRecords;
    bool _rawCcode;

    bool _relay;
    unsigned _runNumber;
    unsigned _linkNumber;
    unsigned _fileNumber;
    unsigned _numberOfBytesInFile;
  };

}

#endif
