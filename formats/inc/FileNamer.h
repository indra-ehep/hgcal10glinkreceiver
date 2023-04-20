#ifndef Hgcal10gLinkReceiver_FileNamer_h
#define Hgcal10gLinkReceiver_FileNamer_h

#include <iostream>
#include <sstream>

namespace Hgcal10gLinkReceiver {

  class FileNamer {
  public:
    FileNamer() {
    }

    void setRunFileName() {
      std::ostringstream sRunFileName;
      sRunFileName << std::setfill('0')
		   << "RunDatFiles/" << (_superRun?"Super":"")
		   << "Run" << std::setw(10) << _runNumber
		   << "_Link" << _linkNumber
		   << "_File" << std::setw(10) << _fileNumber
		   << ".bin";
      _fileName=sRunFileName.str();
    }
    
  protected:
    bool _superRun;
    uint32_t _runNumber;
    uint32_t _linkNumber;
    uint32_t _fileNumber;
    
    std::string _fileName;
  };
}

#endif
