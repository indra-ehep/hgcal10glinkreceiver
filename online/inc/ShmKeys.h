#ifndef ShmKeys_h
#define ShmKeys_h

namespace Hgcal10gLinkReceiver {

  // ce = CE
  // cN or dN = control or data, case N
  // 1cd5 = TCDS2
  // 00, be, da = testing, BE, DAQ
  // 00, 01, 02 = links

  // RUN CONTROL
  
  // Used by Run Controller
  const uint32_t RunControlDummyFsmShmKey(      0xcec00000);
  const uint32_t RunControlFrontEndFsmShmKey(   0xcec0befe);
  const uint32_t RunControlFastControlFsmShmKey(0xcec0befc);
  const uint32_t RunControlTcds2FsmShmKey(      0xcec01cd5);
  const uint32_t RunControlRelayFsmShmKey(      0xcec0da02);
  const uint32_t RunControlStageFsmShmKey(      0xcec051ae);
  const uint32_t RunControlDaqLink0FsmShmKey(   0xcec0da00);
  const uint32_t RunControlDaqLink1FsmShmKey(   0xcec0da01);

  const uint32_t RunControlDaqLink2FsmShmKey(RunControlRelayFsmShmKey); // TEMP


  const uint32_t RunControlCfgLink0FsmShmKey(   0xcec0cf00);
  const uint32_t RunControlCfgLink1FsmShmKey(   0xcec0cf01);
  const uint32_t RunControlCfgLink2FsmShmKey(   0xcec0cf02);
  const uint32_t RunControlCfgLink3FsmShmKey(   0xcec0cf03);

  const uint32_t RunControlDummyFsmPort(      0xcec3);
  const uint32_t RunControlFastControlFsmPort(0xcec4);
  const uint32_t RunControlTcds2FsmPort(      0xcec5);
  const uint32_t RunControlDaqLink0FsmPort(   0xcec0);
  const uint32_t RunControlDaqLink1FsmPort(   0xcec1);
  const uint32_t RunControlDaqLink2FsmPort(   0xcec2);

  // Used by Processors
  const uint32_t ProcessorDummyFsmShmKey(      RunControlDummyFsmShmKey);
//const uint32_t ProcessorDummyFsmShmKey(      RunControlDummyFsmShmKey | 0x00010000); // Local testing
  const uint32_t ProcessorFastControlFsmShmKey(RunControlFastControlFsmShmKey);
  const uint32_t ProcessorTcds2FsmShmKey(      RunControlTcds2FsmShmKey);
  const uint32_t ProcessorRelayFsmShmKey(      RunControlRelayFsmShmKey);
  const uint32_t ProcessorDaqLink0FsmShmKey(   RunControlDaqLink0FsmShmKey);
  const uint32_t ProcessorDaqLink1FsmShmKey(   RunControlDaqLink1FsmShmKey);
  const uint32_t ProcessorDaqLink2FsmShmKey(   RunControlDaqLink2FsmShmKey);
//const uint32_t ProcessorDaqLink2FsmShmKey(   RunControlDaqLink2FsmShmKey | 0x00010000); // Local testing

  const uint32_t ProcessorDummyFsmPort(RunControlDummyFsmPort);
  const uint32_t ProcessorFastControlFsmPort(RunControlFastControlFsmPort);
  const uint32_t ProcessorTcds2FsmPort(RunControlTcds2FsmPort);
  const uint32_t ProcessorDaqLink0FsmPort(RunControlDaqLink0FsmPort);
  const uint32_t ProcessorDaqLink1FsmPort(RunControlDaqLink1FsmPort);
  const uint32_t ProcessorDaqLink2FsmPort(RunControlDaqLink2FsmPort);


  // DATA TRANSFER
  
  //const uint32_t DataFifoDaqLink0FsmShmKey(0xced0da00);
  //const uint32_t DataFifoDaqLink1FsmShmKey(0xced0da01);
  //const uint32_t DataFifoDaqLink2FsmShmKey(0xced0da02);
  
  // Used by Fast Control Processor
  const uint32_t ProcessorFastControlDl0FifoShmKey(0xced0da00);
  const uint32_t ProcessorFastControlDl1FifoShmKey(0xced0da01);
  const uint32_t ProcessorFastControlDl2FifoShmKey(0xced0da02);
  const uint32_t ProcessorFastControlCl0FifoShmKey(0xced0cf00);

  const uint32_t ProcessorRelayCl0FifoShmKey(ProcessorFastControlCl0FifoShmKey);
  const uint32_t ProcessorRelayCl1FifoShmKey(0xced0cf01);

  const uint32_t ProcessorFastControlDl0FifoPort(0xcee0da0);
  const uint32_t ProcessorFastControlDl1FifoPort(0xcee0da1);
  const uint32_t ProcessorFastControlDl2FifoPort(0xcee0da2);

  // Used by Data Link Processors
  const uint32_t ProcessorDaqLink0FifoShmKey(ProcessorFastControlDl0FifoShmKey);
  const uint32_t ProcessorDaqLink1FifoShmKey(ProcessorFastControlDl1FifoShmKey);
  const uint32_t ProcessorDaqLink2FifoShmKey(ProcessorFastControlDl2FifoShmKey);
//const uint32_t ProcessorDaqLink2FifoShmKey(ProcessorFastControlDl2FifoShmKey | 0x00010000); // Local testing

  const uint32_t ProcessorDaqLink0FifoPort(ProcessorFastControlDl0FifoPort);
  const uint32_t ProcessorDaqLink1FifoPort(ProcessorFastControlDl1FifoPort);
  const uint32_t ProcessorDaqLink2FifoPort(ProcessorFastControlDl2FifoPort);
}

#endif
