#ifndef DataFifo_h
#define DataFifo_h

#include <iostream>
#include <iomanip>

#include "RecordHeader.h"

namespace Hgcal10gLinkReceiver {

template<unsigned PowerOfTwo, unsigned Width> class DataFifoT {
public:
  enum {
    BufferDepthPowerOfTwo=PowerOfTwo,
    BufferDepth=(1<<BufferDepthPowerOfTwo),
    BufferDepthMask=BufferDepth-1,
    BufferWidth=Width
  };

  DataFifoT() {
  }

  void ColdStart() {
    _writePtr=0;
    _readPtr=0;
  }

  bool write(uint16_t n, const uint64_t *p) {
    if(_writePtr==_readPtr+BufferDepth) return false;
    //std::memcpy(_buffer[_writePtr%BufferDepth],p,8*n);
    std::memcpy(_buffer[_writePtr&BufferDepthMask],p,8*n);
    _writePtr++;
    return true;
  }

  bool readable() {
    return _writePtr>_readPtr;
  }

  uint16_t read(uint64_t *p) {
    if(_writePtr==_readPtr) return 0;
    RecordHeader *h((RecordHeader*)_buffer[_readPtr&BufferDepthMask]);
    uint16_t n(h->totalLength());
    std::memcpy(p,h,8*n);
    _readPtr++;
    return n;
  }
  
  void print(std::ostream &o=std::cout) {
    o << "DataFifoT<" << PowerOfTwo << "," << Width << ">::print()" << std::endl;
    o << " Write pointer to memory  = " << std::setw(10) << _writePtr << std::endl
      << " Read pointer from memory = " << std::setw(10) << _readPtr << std::endl;    
    uint32_t diff(_writePtr>_readPtr?_writePtr-_readPtr:0);
    o << " Difference               = " << std::setw(10) << diff << std::endl;
  }
  
  uint32_t _writePtr;
  uint32_t _readPtr;
  uint64_t _buffer[BufferDepth][BufferWidth];
};

}
#endif
