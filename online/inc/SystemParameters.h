#ifndef Hgcal10gLinkReceiver_SystemParameters_h
#define Hgcal10gLinkReceiver_SystemParameters_h

#include <cstdint>

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
  const uint32_t RunControlDaqLink2FsmShmKey(   0xcec0da02);

  const uint16_t RunControlDummyFsmPort(      0xcec0);
  const uint16_t RunControlFrontEndFsmPort(   0xcec1);
  const uint16_t RunControlFastControlFsmPort(0xcec2);
  const uint16_t RunControlTcds2FsmPort(      0xcec3);
  const uint16_t RunControlRelayFsmPort(      0xcec4);
  const uint16_t RunControlStageFsmPort(      0xcec5);
  const uint16_t RunControlDaqLink0FsmPort(   0xcec6);
  const uint16_t RunControlDaqLink1FsmPort(   0xcec7);
  const uint16_t RunControlDaqLink2FsmPort(   0xcec8);

  // Used by Processors
  const uint32_t ProcessorDummyFsmShmKey(      RunControlDummyFsmShmKey);
  //const uint32_t ProcessorDummyFsmShmKey(      RunControlDummyFsmShmKey | 0x00010000); // Local testing
  const uint32_t ProcessorFrontEndFsmShmKey(   RunControlFrontEndFsmShmKey);
  const uint32_t ProcessorFastControlFsmShmKey(RunControlFastControlFsmShmKey);
  const uint32_t ProcessorTcds2FsmShmKey(      RunControlTcds2FsmShmKey);
  const uint32_t ProcessorRelayFsmShmKey(      RunControlRelayFsmShmKey);
  const uint32_t ProcessorStageFsmShmKey(      RunControlStageFsmShmKey);
  const uint32_t ProcessorDaqLink0FsmShmKey(   RunControlDaqLink0FsmShmKey);
  const uint32_t ProcessorDaqLink1FsmShmKey(   RunControlDaqLink1FsmShmKey);
  const uint32_t ProcessorDaqLink2FsmShmKey(   RunControlDaqLink2FsmShmKey);

  const uint16_t ProcessorDummyFsmPort(      RunControlDummyFsmPort);
  const uint16_t ProcessorFrontEndFsmPort(   RunControlFrontEndFsmPort);
  const uint16_t ProcessorFastControlFsmPort(RunControlFastControlFsmPort);
  const uint16_t ProcessorTcds2FsmPort(      RunControlTcds2FsmPort);
  const uint16_t ProcessorRelayFsmPort(      RunControlRelayFsmPort);
  const uint16_t ProcessorStageFsmPort(      RunControlStageFsmPort);
  const uint16_t ProcessorDaqLink0FsmPort(   RunControlDaqLink0FsmPort);
  const uint16_t ProcessorDaqLink1FsmPort(   RunControlDaqLink1FsmPort);
  const uint16_t ProcessorDaqLink2FsmPort(   RunControlDaqLink2FsmPort);

  const uint16_t UdpProcessorDaqLink0FifoPort(1234);
  const uint16_t UdpProcessorDaqLink1FifoPort(1235);
  const uint16_t UdpProcessorDaqLink2FifoPort(1236);


  // Junk
  
  const uint32_t RunControlCfgLink0FsmShmKey(   0xcec0cf00);
  const uint32_t RunControlCfgLink1FsmShmKey(   0xcec0cf01);
  const uint32_t RunControlCfgLink2FsmShmKey(   0xcec0cf02);
  const uint32_t RunControlCfgLink3FsmShmKey(   0xcec0cf03);



  // DATA TRANSFER
  
  //const uint32_t DataFifoDaqLink0FsmShmKey(0xced0da00);
  //const uint32_t DataFifoDaqLink1FsmShmKey(0xced0da01);
  //const uint32_t DataFifoDaqLink2FsmShmKey(0xced0da02);
  
  // Used by Serenity and Stage Processors
  const uint32_t ProcessorFastControlDl0FifoShmKey(0xced0da00);
  const uint32_t ProcessorFastControlDl1FifoShmKey(0xced0da01);
  const uint32_t ProcessorFastControlDl2FifoShmKey(0xced0da02);
  const uint32_t ProcessorFastControlCl0FifoShmKey(0xced0cf00);
  const uint32_t ProcessorTcds2Cl1FifoShmKey(      0xced0cf01);
  const uint32_t ProcessorFrontEndCl2FifoShmKey(   0xced0cf02);
  const uint32_t ProcessorStageCl3FifoShmKey(      0xced0cf03);

  const uint16_t ProcessorFastControlDl0FifoPort(0xced0);
  const uint16_t ProcessorFastControlDl1FifoPort(0xced1);
  const uint16_t ProcessorFastControlDl2FifoPort(0xced2);
  const uint16_t ProcessorFastControlCl0FifoPort(0xced3);
  const uint16_t ProcessorTcds2Cl1FifoPort(      0xced4);
  const uint16_t ProcessorFrontEndCl2FifoPort(   0xced5);
  const uint16_t ProcessorStageCl3FifoPort(      0xced6);

  // Used by PC Processors
  const uint32_t ProcessorDaqLink0FifoShmKey(ProcessorFastControlDl0FifoShmKey);
  const uint32_t ProcessorDaqLink1FifoShmKey(ProcessorFastControlDl1FifoShmKey);
  const uint32_t ProcessorDaqLink2FifoShmKey(ProcessorFastControlDl2FifoShmKey);
  const uint32_t ProcessorRelayCl0FifoShmKey(ProcessorFastControlCl0FifoShmKey);
  const uint32_t ProcessorRelayCl1FifoShmKey(ProcessorTcds2Cl1FifoShmKey);
  const uint32_t ProcessorRelayCl2FifoShmKey(ProcessorFrontEndCl2FifoShmKey);
  const uint32_t ProcessorRelayCl3FifoShmKey(ProcessorStageCl3FifoShmKey);

  const uint16_t ProcessorDaqLink0FifoPort(ProcessorFastControlDl0FifoPort);
  const uint16_t ProcessorDaqLink1FifoPort(ProcessorFastControlDl1FifoPort);
  const uint16_t ProcessorDaqLink2FifoPort(ProcessorFastControlDl2FifoPort);
  const uint16_t ProcessorRelayCl0FifoPort(ProcessorFastControlCl0FifoPort);
  const uint16_t ProcessorRelayCl1FifoPort(ProcessorTcds2Cl1FifoPort);
  const uint16_t ProcessorRelayCl2FifoPort(ProcessorFrontEndCl2FifoPort);
  const uint16_t ProcessorRelayCl3FifoPort(ProcessorStageCl3FifoPort);
}

#endif
