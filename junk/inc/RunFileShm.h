#ifndef RunFileShm_h
#define RunFileShm_h

#include <iostream>
#include <iomanip>

#include "RunControlFsmShm.h"
#include "ShmSingleton.h"

namespace Hgcal10gLinkReceiver {

template<unsigned PowerOfTwo, unsigned Width> class RunFileShmT {
public:
  enum {
    BufferDepthPowerOfTwo=PowerOfTwo,
    BufferDepth=(1<<BufferDepthPowerOfTwo),
    BufferDepthMask=BufferDepth-1,
    BufferWidth=Width
  };

  RunFileShmT() : _requestActive(false), _requestShutDown(false), _writeEnable(true) {
  }

  void ColdStart() {
    _runStateActive=false;
    _writePtr=0;
    _readPtr=0;
  }
  
  void print(std::ostream &o=std::cout) {
    uint64_t one64(1);

    o << "RunFileShmT<" << PowerOfTwo << "," << Width << ">::print()" << std::endl;

    _runControlFsmShm.print(o);
    o << std::endl;
    
    o << " Run request = " << (_requestActive?"Active":"Inactive")
      << ", shutdown request = " << (_requestShutDown?"Shutdown":"Alive")
      << ", run state = " << (_runStateActive?"Active":"Inactive")
      << std::endl;

    o << " Read enable from link = " << (_readEnable?"Enabled":"Disabled")
      << std::endl;
    o << " Write enable to file  = " << (_writeEnable?"Enabled":"Disabled")
      << std::endl;
    
    o << " Run number  " << std::setw(10) << _runNumber << ", packets = "
      << std::setw(10) << _packetNumberInRun << ", bytes = "
      << std::setw(12) << _bytesInRun << ", average packet size = "
      << std::setw(6) << _bytesInRun/std::max(_packetNumberInRun,one64) << " bytes"
      << std::endl;

    o << " File number " << std::setw(10) << _fileNumber << ", packets = "
      << std::setw(10) << _packetNumberInFile << ", bytes = "
      << std::setw(12) << _bytesInFile << ", average packet size = "
      << std::setw(6) << _bytesInFile/std::max(_packetNumberInFile,one64) << " bytes"
      << std::endl;

    o << " Write pointer to memory  = " << std::setw(10) << _writePtr << std::endl
      << " Read pointer from memory = " << std::setw(10) << _readPtr << std::endl;    
    uint32_t diff(_writePtr>_readPtr?_writePtr-_readPtr:0);
    o << " Difference               = " << std::setw(10) << diff << std::endl;
  }
  
  RunControlFsmShm _runControlFsmShm;

  bool _requestActive;
  bool _requestShutDown;
  bool _runStateActive;

  bool _readEnable;
  bool _writeEnable;

  bool _readPrint;
  bool _writePrint;

  uint32_t _runNumber;
  uint64_t _packetNumberInRun;
  uint64_t _bytesInRun;
  
  uint32_t _fileNumber;
  uint64_t _packetNumberInFile;
  uint64_t _bytesInFile;
  
  uint32_t _writePtr;
  uint32_t _readPtr;
  uint64_t _buffer[BufferDepth][BufferWidth];
};

typedef RunFileShmT<8,256> RunFileShm0;
typedef RunFileShmT<7,256> RunFileShm1;
typedef RunFileShmT<4,1024> RunFileShm2;

//template<> const key_t ShmSingleton<RunFileShm0>::theKey=0xce2300;
//template<> const key_t ShmSingleton<RunFileShm1>::theKey=0xce2301;
//template<> const key_t ShmSingleton<RunFileShm2>::theKey=0xce23c0;
}
#endif
