#ifndef ShmKeys_h
#define ShmKeys_h

namespace Hgcal10gLinkReceiver {

  // ce = CE
  // cN or dN = control or data, case N
  // 1cd5 = TCDS2
  // 00, be, da = testing, BE, DAQ
  // 00, 01, 02 = links

  const uint32_t RunControlTestingShmKey(0xcec00000);
  const uint32_t RunControlFastControlShmKey(0xcec0befc);
  const uint32_t RunControlTcds2ShmKey(0xcec01cd5);
  const uint32_t RunControlDaqLink0ShmKey(0xcec0da00);
  const uint32_t RunControlDaqLink1ShmKey(0xcec0da01);
  const uint32_t RunControlDaqLink2ShmKey(0xcec0da02);
  
  const uint32_t DataFifoDaqLink0ShmKey(0xced0da00);
  const uint32_t DataFifoDaqLink1ShmKey(0xced0da01);
  const uint32_t DataFifoDaqLink2ShmKey(0xced0da02);
  
  const uint32_t ProcessorDummyShmKey(RunControlTestingShmKey);
  //const uint32_t ProcessorDummyShmKey(RunControlTestingShmKey | 0x00010000);
  const uint32_t ProcessorFastControlShmKey(RunControlFastControlShmKey);
  const uint32_t ProcessorTcds2ShmKey(RunControlTcds2ShmKey);
  const uint32_t ProcessorDaqLink0ShmKey(RunControlDaqLink0ShmKey);
  const uint32_t ProcessorDaqLink1ShmKey(RunControlDaqLink1ShmKey);
  const uint32_t ProcessorDaqLink2ShmKey(RunControlDaqLink2ShmKey);

  const uint32_t ProcessorFastControlDataShmKey(DataFifoDaqLink2ShmKey);
  const uint32_t ProcessorDaqLink0FifoShmKey(DataFifoDaqLink0ShmKey);
  const uint32_t ProcessorDaqLink1FifoShmKey(DataFifoDaqLink1ShmKey);
  const uint32_t ProcessorDaqLink2FifoShmKey(DataFifoDaqLink2ShmKey);
}

#endif
