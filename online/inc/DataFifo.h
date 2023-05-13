#ifndef Hgcal10gLinkReceiver_DataFifo_h
#define Hgcal10gLinkReceiver_DataFifo_h

#include <iostream>
#include <iomanip>

#include "Record.h"

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

  void initialize() {
    _backPressure=false;
    _writePtr=0;
    _readPtr=0;
    // RESET ALL RECORDHEADERS?
  }
  
  void end() {
    _writePtr=0xffffffff;
    _readPtr=0xffffffff;
  }

  bool isEnded() {
    return _writePtr==0xffffffff && _readPtr==0xffffffff;
  }
  
  void ColdStart() {
    _writePtr=0;
    _readPtr=0;
  }

  // Processor writing into FIFO
  Record* getWriteRecord() {
    if(!writeable()) return nullptr;
    return (Record*)_buffer[_writePtr&BufferDepthMask];
  }

  void writeIncrement() {
    _writePtr++;
  }
  
  // Reading from FIFO to disk without or with modification in FIFO first
  const Record* readRecord() {
    if(!readable()) return nullptr;
    return (const Record*)_buffer[_readPtr&BufferDepthMask];
  }

  Record* getReadRecord() {
    if(!readable()) return nullptr;
    return (Record*)_buffer[_readPtr&BufferDepthMask];
  }

  void readIncrement() {
    _readPtr++;
  }
  

  
  bool writeRecord(const Record *r) {
    return write(r->payloadLength(),(const uint64_t*)r);
  }


  bool write(uint16_t n, const uint64_t *p) {
    if(_writePtr==_readPtr+BufferDepth) return false;
    //std::memcpy(_buffer[_writePtr%BufferDepth],p,8*n);
    std::memcpy(_buffer[_writePtr&BufferDepthMask],p,8*n);
    _writePtr++;
    return true;
  }

  uint32_t writeable() const {
    return _readPtr+BufferDepth-_writePtr;
  }











  
  uint32_t readable() {
    return _writePtr-_readPtr;
  }

  uint16_t read(uint64_t *p) {
    if(_writePtr==_readPtr) return 0;
    RecordHeader *h((RecordHeader*)_buffer[_readPtr&BufferDepthMask]);
    uint16_t n(h->totalLength());
    std::memcpy(p,h,8*n);
    _readPtr++;
    return n;
  }
  
  bool backPressure() const {
    return _backPressure;
  }

  void setBackPressure(bool b) {
    _backPressure=b;
  }
  
  void print(std::ostream &o=std::cout) {
    o << "DataFifoT<" << PowerOfTwo << "," << Width << ">::print()" << std::endl;
    o << " Write pointer to memory  = " << std::setw(10) << _writePtr << std::endl
      << " Read pointer from memory = " << std::setw(10) << _readPtr << std::endl;    
    uint32_t diff(_writePtr>_readPtr?_writePtr-_readPtr:0);
    o << " Difference               = " << std::setw(10) << diff << std::endl;
    o << " Back pressure = " << (_backPressure?"on":"off") << std::endl;
  }

  bool _backPressure;
  uint32_t _writePtr;
  uint32_t _readPtr;
  uint64_t _buffer[BufferDepth][BufferWidth];
};

}
#endif
