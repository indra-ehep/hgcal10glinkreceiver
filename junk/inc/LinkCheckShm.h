#ifndef LinkCheckShm_h
#define LinkCheckShm_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cassert>
#include <vector>

#include "ShmSingleton.h"

class LinkCheckShm {

 public:
  enum {
    NumberOfSocketPackets,
    BytesOfSocketPackets,
    MinBytesOfSocketPackets,
    MaxBytesOfSocketPackets,
    NumberOfSocketSizeEq0k,
    NumberOfSocketSizeGt4k,

    NumberOfSocketSizeMod8,
    NumberOfSocketSizeGtPacket,
    NumberOfSocketSizeLtPacket,
    NumberOfSocketSizeEqPacket,
    NumberOfSocketsGtOnePacket,
    
    NumberOfPacketBadHeaders,
    NumberOfPacketSkips,

    NumberOfPacketSkipsGt0,
    NumberOfPacketSkippedLtP10,
    NumberOfPacketSkippedLtP100,
    NumberOfPacketSkippedLtP1000,
    NumberOfPacketSkippedLtP10000,
    NumberOfPacketSkippedLtP100000,
    NumberOfPacketSkippedGeP100000,
    NumberOfTotalSkippedPacketsGt0,

    NumberOfPacketSkipsLt0,
    NumberOfPacketSkippedGtM10,
    NumberOfPacketSkippedGtM100,
    NumberOfPacketSkippedGtM1000,
    NumberOfPacketSkippedGtM10000,
    NumberOfPacketSkippedGtM100000,
    NumberOfPacketSkippedLeM100000,
    NumberOfTotalSkippedPacketsLt0,
    
    NumberOfEnums,
    ArraySize=100
  };

  LinkCheckShm() {
    assert(NumberOfEnums<ArraySize);
  }
  
  virtual ~LinkCheckShm() {
  }

  void reset() {
    for(unsigned i(0);i<NumberOfEnums;i++) {
      _array[i]=0;
    }
  }

  void print(std::ostream &o=std::cout) const {
    o << "LinkCheckShm::print()" << std::endl;

    o << " Socket checks:" << std::endl;
    o << "  Total number of socket packets                     = "
      << std::setw(20) << _array[NumberOfSocketPackets] << std::endl;
    o << "  Total bytes of socket packets                      = "
      << std::setw(20) << _array[BytesOfSocketPackets] << "  average = "
      << double(_array[BytesOfSocketPackets])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << "  Minimum bytes of socket packets                    = "
      << std::setw(20) << _array[MinBytesOfSocketPackets] << std::endl;
    o << "  Maximum bytes of socket packets                    = "
      << std::setw(20) << _array[MaxBytesOfSocketPackets] << std::endl;
    o << "  Number of socket packets with zero bytes           = "
      << std::setw(20) << _array[NumberOfSocketSizeEq0k]  << "  fraction = "
      << double(_array[NumberOfSocketSizeEq0k])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << "  Number of socket packets with more than 4k bytes   = "
      << std::setw(20) << _array[NumberOfSocketSizeGt4k] << "  fraction = "
      << double(_array[NumberOfSocketSizeGt4k])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << std::endl;
    
    o << " Event checks:" << std::endl;
    o << "  Number of socket packets not multiples of 8 bytes  = "
      << std::setw(20) << _array[NumberOfSocketSizeMod8]  << "  fraction = "
      << double(_array[NumberOfSocketSizeGt4k])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << "  Number of socket packets larger than event size    = "
      << std::setw(20) << _array[NumberOfSocketSizeGtPacket]  << "  fraction = "
      << double(_array[NumberOfSocketSizeGtPacket])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << "  Number of socket packets smaller than event size   = "
      << std::setw(20) << _array[NumberOfSocketSizeLtPacket]  << "  fraction = "
      << double(_array[NumberOfSocketSizeLtPacket])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << "  Number of socket packets equal to event            = "
      << std::setw(20) << _array[NumberOfSocketSizeEqPacket]  << "  fraction = "
      << double(_array[NumberOfSocketSizeEqPacket])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << "  Number of socket packets with more than one event  = "
      << std::setw(20) << _array[NumberOfSocketsGtOnePacket]  << "  fraction = "
      << double(_array[NumberOfSocketsGtOnePacket])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << "  Number of socket packets with bad header (!= 0xac) = "
      << std::setw(20) << _array[NumberOfPacketBadHeaders]  << "  fraction = "
      << double(_array[NumberOfPacketBadHeaders])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << std::endl;

    o << " Skip checks:" << std::endl;
    o << "  Number of socket packet skips                      = "
      << std::setw(20) << _array[NumberOfPacketSkips] << std::endl;
    o << std::endl;

    o << "  Number of positive socket packet skips             = "
      << std::setw(20) << _array[NumberOfPacketSkipsGt0] << std::endl;
    o << "  Number of positive socket packet skips < 10        = "
      << std::setw(20) << _array[NumberOfPacketSkippedLtP10] << "  fraction = "
      << double(_array[NumberOfPacketSkippedLtP10])/std::max(_array[NumberOfPacketSkipsGt0],1LU)
      << std::endl;
    o << "  Number of positive socket packet skips < 100       = "
      << std::setw(20) << _array[NumberOfPacketSkippedLtP100] << "  fraction = "
      << double(_array[NumberOfPacketSkippedLtP100])/std::max(_array[NumberOfPacketSkipsGt0],1LU)
      << std::endl;
    o << "  Number of positive socket packet skips < 1000      = "
      << std::setw(20) << _array[NumberOfPacketSkippedLtP1000] << "  fraction = "
      << double(_array[NumberOfPacketSkippedLtP1000])/std::max(_array[NumberOfPacketSkipsGt0],1LU)
      << std::endl;
    o << "  Number of positive socket packet skips < 10000     = "
      << std::setw(20) << _array[NumberOfPacketSkippedLtP10000] << "  fraction = "
      << double(_array[NumberOfPacketSkippedLtP10000])/std::max(_array[NumberOfPacketSkipsGt0],1LU)
      << std::endl;
    o << "  Number of positive socket packet skips < 100000    = "
      << std::setw(20) << _array[NumberOfPacketSkippedLtP100000] << "  fraction = "
      << double(_array[NumberOfPacketSkippedLtP100000])/std::max(_array[NumberOfPacketSkipsGt0],1LU)
      << std::endl;
    o << "  Number of positive socket packet skips >= 100000   = "
      << std::setw(20) << _array[NumberOfPacketSkippedGeP100000] << "  fraction = "
      << double(_array[NumberOfPacketSkippedGeP100000])/std::max(_array[NumberOfPacketSkipsGt0],1LU)
      << std::endl;
    o << "  Total number of positive socket skipped packets    = "
      << std::setw(20) << _array[NumberOfTotalSkippedPacketsGt0] << "  fraction = "
      << double(_array[NumberOfTotalSkippedPacketsGt0])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
    o << std::endl;

    o << "  Number of negative socket packet skips             = "
      << std::setw(20) << _array[NumberOfPacketSkipsLt0] << std::endl;
    o << "  Number of negative socket packet skips > -10       = "
      << std::setw(20) << _array[NumberOfPacketSkippedGtM10] << "  fraction = "
      << double(_array[NumberOfPacketSkippedGtM10])/std::max(_array[NumberOfPacketSkipsLt0],1LU)
      << std::endl;
    o << "  Number of negative socket packet skips > -100      = "
      << std::setw(20) << _array[NumberOfPacketSkippedGtM100] << "  fraction = "
      << double(_array[NumberOfPacketSkippedGtM10])/std::max(_array[NumberOfPacketSkipsLt0],1LU)
      << std::endl;
    o << "  Number of negative socket packet skips > -1000     = "
      << std::setw(20) << _array[NumberOfPacketSkippedGtM1000] << "  fraction = "
      << double(_array[NumberOfPacketSkippedGtM1000])/std::max(_array[NumberOfPacketSkipsLt0],1LU)
      << std::endl;
    o << "  Number of negative socket packet skips > -10000    = "
      << std::setw(20) << _array[NumberOfPacketSkippedGtM10000] << "  fraction = "
      << double(_array[NumberOfPacketSkippedGtM10000])/std::max(_array[NumberOfPacketSkipsLt0],1LU)
      << std::endl;
    o << "  Number of negative socket packet skips > -100000   = "
      << std::setw(20) << _array[NumberOfPacketSkippedGtM100000] << "  fraction = "
      << double(_array[NumberOfPacketSkippedGtM100000])/std::max(_array[NumberOfPacketSkipsLt0],1LU)
      << std::endl;
    o << "  Number of negative socket packet skips <= -100000  = "
      << std::setw(20) << _array[NumberOfPacketSkippedLeM100000] << "  fraction = "
      << double(_array[NumberOfPacketSkippedLeM100000])/std::max(_array[NumberOfPacketSkipsLt0],1LU)
      << std::endl;
    o << "  Total number of negative socket skipped packets    = "
      << std::setw(20) << _array[NumberOfTotalSkippedPacketsLt0] << "  fraction = "
      << double(_array[NumberOfTotalSkippedPacketsLt0])/std::max(_array[NumberOfSocketPackets],1LU)
      << std::endl;
  }
      
  uint64_t _array[ArraySize];

 private:
};

//template<> const key_t ShmSingleton<LinkCheckShm>::theKey=0xce2302;

#endif
