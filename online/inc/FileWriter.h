#ifndef Hgcal10gLinkReceiver_FileWriter_h
#define Hgcal10gLinkReceiver_FileWriter_h

#include <cstdio>
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
    
    FileWriter() : MaximumBytesPerFile(4000000000),
		   MaximumEventsPerFile(1000000),
		   MaximumOrbitsPerFile(2000000),
		   _directory("dat/"), _protectFiles(false), _flushRecords(true), _rawCcode(false) {      
      _outputFilePtr=nullptr;
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

      _ecOld=1;
      _ocOld=0;
      
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
	  if(_outputFilePtr==nullptr) return false;

	} else {
	  _outputFile.open("/dev/null",std::ios::binary);
	}
      }

      return true;
    }

    //bool write(uint64_t *d, unsigned n) {
    bool write(const Record* h) {
      std::cout << "FileWriter HERE4!" << std::endl;

      bool newFile(false);

      if(h->state()==FsmState::Running) {
	const RecordRunning *r((const RecordRunning*)h);

	uint64_t ec(0);
	uint32_t oc(0);

	if(r->slinkBoe()!=nullptr && r->slinkEoe()!=nullptr) {
	  if(r->slinkBoe()->validPattern() && r->slinkEoe()->validPattern()) {
	    ec=r->slinkBoe()->eventId();
	    oc=r->slinkEoe()->orbitId();
	    
	    if((ec-_ecOld)>=MaximumEventsPerFile ||
	       (oc-_ocOld)>=MaximumOrbitsPerFile) {
	      newFile=true;
	      _ecOld=ec;
	      _ocOld=oc;
	    }
	  }
	}
	
	std::cout << "ec, ecOld, oc, ocOld, newFile = "
		  << ec << "," << _ecOld  << "," 
		  << oc << "," << _ocOld  << "," 
		  << (newFile?"true":"false") << std::endl;
      }

      // Obsolete; does not work with two files for DPG
      //if(_numberOfBytesInFile>MaximumBytesPerFile) {

      if(newFile) {

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
	    if(_outputFilePtr!=nullptr) {
	      fwrite((char*)(&rc),1,sizeof(RecordContinuing),_outputFilePtr);
	    }
	    
	  } else {
	    _outputFile.write((char*)(&rc),sizeof(RecordContinuing));
	  }
	}
	
	close();

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
	    if(_outputFilePtr==nullptr) return false;

	  } else {
	    _outputFile.open("/dev/null",std::ios::binary);
	  }
	}
      }
      
      //if(_writeEnable) {
      if(_rawCcode) {
	if(_outputFilePtr!=nullptr) 
	  fwrite((char*)h,1,8*h->totalLength(),_outputFilePtr);
      } else {
	_outputFile.write((char*)h,8*h->totalLength());
	if(_flushRecords) _outputFile.flush();
      }
      
      _numberOfBytesInFile+=8*h->totalLength();
      //}

      return true;
    }
    
    bool close() {
      std::cout << "FileWriter::close() closing file "
		<< _fileName.c_str() << std::endl;
      
      if(_rawCcode) {
	if(_outputFilePtr!=nullptr) {
	  fclose(_outputFilePtr);
	  _outputFilePtr=nullptr;
	}
	
      } else {
	if(_outputFile.is_open()) {
	  _outputFile.close();
	}
      }
	
      if(_writeEnable && _protectFiles) {
	system((std::string("chmod 444 ")+_fileName).c_str());
      }

      return true;
    }
    
  private:
    const uint32_t MaximumBytesPerFile;
    const uint64_t MaximumEventsPerFile;
    const uint32_t MaximumOrbitsPerFile;
    uint64_t _ecOld;
    uint32_t _ocOld;
    
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
