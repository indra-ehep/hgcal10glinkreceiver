#ifndef Hgcal10gLinkReceiver_ProcessorFastControl_h
#define Hgcal10gLinkReceiver_ProcessorFastControl_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

#include "SerenityEncoder.h"
#include "SerenityLpgbt.h"
#include "SerenityMiniDaq.h"
#include "Serenity10g.h"
#include "Serenity10gx.h"
#include "SerenityTrgDaq.h"
#include "SerenityAligner.h"
#include "SerenitySlink.h"
#include "SerenityUnpacker.h"
#include "ShmSingleton.h"
#include "ProcessorBase.h"
#include "DataFifo.h"



#include "I2cInstruction.h"
#include "UhalInstruction.h"
#include "RecordConfigured.h"



#include "RecordPrinter.h"
#include "RecordHalted.h"

//#ifdef ProcessorHardware
//#include "uhal/uhal.hpp"
//#include "uhal/ValMem.hpp"
//#endif

namespace Hgcal10gLinkReceiver {

  class ProcessorFastControl : public ProcessorBase {
    
  public:
    ProcessorFastControl() {      
#ifdef DthHardware
      _nMiniDaqs=1;
#else
      _nMiniDaqs=2;
#endif
      _nUnpackers=2;

      std::system("empbutler -c etc/connections.xml do x0 info");

      uint32_t dna[3];
      dna[0]=_serenityTrgDaq.uhalTopRead("info.device.dna.word0");
      dna[1]=_serenityTrgDaq.uhalTopRead("info.device.dna.word1");
      dna[2]=_serenityTrgDaq.uhalTopRead("info.device.dna.word2");

      daqBoard=3;
      if(dna[2]==0x40020000 && dna[1]==0x01290ba6 && dna[0]==0x0430a085) daqBoard=4;
      std::cout << "DAQ board set to " << daqBoard << std::endl;
    }

    virtual ~ProcessorFastControl() {
    }
  
    void setUpAll(uint32_t rcKey, uint32_t fifoKey) {
      _serenityEncoder.makeTable();
      _serenityLpgbt.makeTable();
      _serenityMiniDaq[0].makeTable();
      if(_nMiniDaqs>1) _serenityMiniDaq[1].makeTable("1");
      _serenityTrgDaq.makeTable();
      _serenityAligner.makeTable();
      _serenityUnpacker[0].makeTable("0");
      if(_nUnpackers>1) _serenityUnpacker[1].makeTable("1");

#ifdef DthHardware
      _serenitySlink.makeTable();
#else
      _serenity10g.makeTable();
      //_serenity10gx.makeTable();
#endif
      
      ShmSingleton<RelayWriterDataFifo> shm2;
      ptrFifoShm2=shm2.setup(fifoKey);
      startFsm(rcKey);
    }

    virtual bool initializing() {
      _serenityEncoder.setDefaults();
      _serenityEncoder.print();

      _serenityLpgbt.setDefaults();
      _serenityLpgbt.print();
      
      _serenityMiniDaq[0].setDefaults();
      _serenityMiniDaq[0].print();
      if(_nMiniDaqs>1) {
	_serenityMiniDaq[1].setDefaults();
	_serenityMiniDaq[1].print();
      }
      _serenityTrgDaq.setDefaults();
      _serenityTrgDaq.print();

      //_serenityAligner.print(); // Does all payload

      _serenityUnpacker[0].setDefaults();
      _serenityUnpacker[0].print();
      if(_nUnpackers>1) {
	_serenityUnpacker[1].setDefaults();
	_serenityUnpacker[1].print();
      }

#ifdef DthHardware
      _serenitySlink.setDefaults();
      _serenitySlink.print();
#else
      _serenity10g.setDefaults();
      _serenity10g.print();
#endif


      ///////////////////////////////////////////////////////
      
      RecordYaml *r;
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(1000);
      r->setHeader(_cfgSeqCounter++);
      r->setState(FsmState::Constants);
      
      // Replace with "constants" call to SerenityTcds2
      YAML::Node n;
      n["Source"]="Serenity";
      n["HardwareVersion"]="PrototypeV1.1";
      n["PayloadVersion"]=_serenityEncoder.payloadVersion();
      n["ElectronicsId"]=daqBoard<<22|0x3fffff;
       
      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());
	
      if(_printEnable) r->print();

      if(_printEnable) ptrFifoShm2->print();
      ptrFifoShm2->writeIncrement();

      writeContinuing();

#ifdef DthHardware
      _serenitySlink.setSourceId(0,0xce000000|daqBoard<<4|1); // DAQ is on channel 0 = Link 1
      _serenitySlink.setSourceId(1,0xce000000|daqBoard<<4|0); // TRG is on channel 1 = Link 0
#else
      _serenityTrgDaq.uhalWrite( "trigger_ro.SLink.source_id"  ,0xce000000|daqBoard<<4|0,true);
      _serenityEncoder.uhalWrite("DAQ_SLink_readout.source_id" ,0xce000000|daqBoard<<4|1,true);
      _serenityEncoder.uhalWrite("DAQ_SLink_readout1.source_id",0xce000000|daqBoard<<4|2,true);
#endif

      return true;
    }

    bool configuring() {
      if(_printEnable) {
	std::cout << "Configuring" << std::endl;
	RecordPrinter(_ptrFsmInterface->record());
	std::cout << std::endl;
      }

      if(_checkEnable) {
	if(_ptrFsmInterface->record().state()!=FsmState::Configuring) {
	  std::cerr << "State does not match" << std::endl;
	  std::cout << "State does not match" << std::endl;
	  if(_assertEnable) assert(false);
	}
      }
      
      //assert(_ptrFsmInterface->commandPacket().record().state()==FsmState::Configuring);

      _cfgSeqCounter=1;
      _evtSeqCounter=1;
      _configuringBCounter=0;

      RecordConfiguring &r((RecordConfiguring&)(_ptrFsmInterface->record()));
      if(_printEnable) r.print();

      //_keyCfgA=r.processorKey(RunControlTcds2FsmShmKey);
      YAML::Node nRsa(YAML::Load(r.string()));
      _keyCfgA=nRsa["ProcessorKey"].as<uint32_t>();
      _strCfgA=nRsa["RunType"].as<std::string>();

      return allConfiguring();
    }

    
    bool reconfiguring() {
      _configuringBCounter++;

      return allConfiguring();
    }

    bool allConfiguring() {

      if(_strCfgA=="EcontTriggerCalPulseIntTest") {
        _serenityUnpacker[0].uhalWrite("ctrl_stat.ctrl0.trig_threshold", 80);
      }
 
      if(_strCfgA=="EcontTriggerThresholdScan") {
        _serenityUnpacker[0].uhalWrite("ctrl_stat.ctrl0.trig_threshold",127);
        _serenityUnpacker[1].uhalWrite("ctrl_stat.ctrl0.trig_threshold",127);

	unsigned unpacker((_configuringBCounter%20)/10);
	unsigned threshold((unpacker==0?79:99)-((_configuringBCounter%20)%10));
        _serenityUnpacker[unpacker].uhalWrite("ctrl_stat.ctrl0.trig_threshold",threshold);
      }

      if(_keyCfgA==125) {
	_serenityEncoder.uhalWrite("ctrl.l1a_stretch",_configuringBCounter/32);
      }

      /*
	if(_keyCfgA==126) {
	_serenityLpgbt.uhalWrite("ctrl.user_bx",3);
	_serenityLpgbt.uhalWrite("fc_cmd.user",0xa9);
	_serenityLpgbt.uhalWrite("ctrl.loop_user_cmd",1);
	}
      */
      // Do configuration; ones which could have been changed
      //_serenityEncoder.uhalWrite("calpulse_ctrl.calpulse_int_del",8);


 	if(_keyCfgA==127 || _keyCfgA==128) {
	  if(_nMiniDaqs==1) _serenityMiniDaq[0].setNumberOfEconds(2);
	}

 	if(_keyCfgA==133 || _keyCfgA==135) {
	  //_saveData=_serenityTrgDaq.uhalRead("daq_ro.DAQro2.latency");
	  _saveData=3;
	  _serenityTrgDaq.uhalWrite("daq_ro.DAQro2.latency",_saveData+_serenityTrgDaq.uhalRead("daq_ro.DAQro2.event_size")*(_configuringBCounter%50));
	}

 	if(_keyCfgA==999) {
	}

      return true;
    }

    bool starting() {
      /*
	_pauseCounter=0;
	_eventNumberInRun=0;

	RecordStarting *r;
	while((r=(RecordStarting*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(10);
	r->deepCopy(_ptrFsmInterface->commandPacket().record());

	_runNumber=r->runNumber();
	if(_printEnable) r->print();
	ptrFifoShm2->writeIncrement();

	//ptrFifoShm2->print();
	//assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
	*/

      RecordYaml *ry;
      while((ry=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      ry->setHeader(_cfgSeqCounter++);
      ry->setState(FsmState::Configuration);
      ry->print();
      
      YAML::Node total;
      total["Source"]="Serenity";
      total["DaqBoard"]=daqBoard;
      total["ElectronicsId"]=daqBoard<<22|0x3fffff;

      YAML::Node ne;
      _serenityEncoder.configuration(ne);
      total["FcEncoder"]=ne;

      YAML::Node ntd;
      _serenityTrgDaq.configuration(ntd);
      total["TriggerDaq"]=ntd;
      
      YAML::Node na;
      _serenityAligner.configuration(na);
      total["Aligner"]=na;

#ifdef DthHardware
      YAML::Node nsl;
      _serenitySlink.configuration(nsl);
      total["Slink"]=nsl;
#else
      YAML::Node n10g;
      _serenity10g.configuration(n10g);
      total["Eth10G"]=n10g;
#endif

      /////////////////////////
      
      total["LpgbtPair"]["0"]["Id"]=0;

      YAML::Node nm00;
      _serenityMiniDaq[0].configuration(nm00);
      total["LpgbtPair"]["0"]["MiniDaq"]=nm00;

      YAML::Node nu00;
      _serenityUnpacker[0].configuration(nu00);
      total["LpgbtPair"]["0"]["Unpacker"]=nu00;

      YAML::Node nl00;
      _serenityLpgbt.configuration(nl00);
      total["LpgbtPair"]["0"]["FcStream"]=nl00;
      
      /////////////////////////

      total["LpgbtPair"]["1"]["Id"]=1;

      if(_nMiniDaqs>1) {
	YAML::Node nm01;
	_serenityMiniDaq[1].configuration(nm01);
	total["LpgbtPair"]["1"]["MiniDaq"]=nm01;
      }

      if(_nUnpackers>1) {
	YAML::Node nu01;
	_serenityUnpacker[1].configuration(nu01);
	total["LpgbtPair"]["1"]["Unpacker"]=nu01;
      }

      YAML::Node nl01;
      _serenityLpgbt.configuration(nl01);
      total["LpgbtPair"]["1"]["FcStream"]=nl01;
      
      std::ostringstream sout;
      sout << total;
      ry->setString(sout.str());
      ry->print();

      ptrFifoShm2->writeIncrement();

      writeContinuing();

      ///////////////////////////////////////////////////
      
      _serenityMiniDaq[0].reset();
      if(_nMiniDaqs>1) _serenityMiniDaq[1].reset();

      _serenityUnpacker[0].reset();
      if(_nUnpackers>1) _serenityUnpacker[1].reset();

      // L1A FIFO; should this be reset before MiniDaq?
      _serenityEncoder.resetSlinkFifo();

      // Now in MiniDaq
      //_serenityEncoder.resetDaqReadout();
      //_serenityEncoder.resetTrgReadout();

#ifdef DthHardware
      _serenitySlink.channelReset();
      usleep(2*1000000);
#else
      _serenity10g.reset();
      usleep(2*1000000);
      _serenity10g.setHeartbeat(false);
      _serenity10g.resetCounters();
#endif

      return true;
    }

    bool pausing() {
      /* NOT FOR Shm2
	 RecordPausing rr;
	 rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	 rr.print();
	 ptrFifoShm2->print();
	 assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      */
      return true;
    }
    
    bool resuming() {
      /*
	RecordResuming rr;
	rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	rr.print();
	ptrFifoShm2->print();
	assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
      */
      return true;
    }
    
    bool stopping() {
#ifndef DthHardware
      _serenity10g.setHeartbeat(true);
#endif

      //_serenity10gx.uhalWrite("ctrl.reg.en",0);
      /*
	_eventNumberInConfiguration+=_eventNumberInRun;

	RecordStopping *r;
	while((r=(RecordStopping*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(10);
	r->deepCopy(_ptrFsmInterface->commandPacket().record());

	r->setNumberOfEvents(_eventNumberInRun);
	r->setNumberOfPauses(_pauseCounter);
	if(_printEnable) r->print();
	ptrFifoShm2->writeIncrement();

	//RecordStopping rr;
	//rr.deepCopy(_ptrFsmInterface->commandPacket().record());
	//rr.setNumberOfEvents(_eventNumberInRun);
	//rr.setNumberOfPauses(_pauseCounter);
	//rr.print();
	//ptrFifoShm2->print();
	//assert(ptrFifoShm2->write(rr.totalLength(),(uint64_t*)(&rr)));
	*/

      /////////////////////////////////////////////////////
      
      // Status at run end
      RecordYaml *r; 
      while((r=(RecordYaml*)(ptrFifoShm2->getWriteRecord()))==nullptr) usleep(1000);
	
      r->setHeader(_cfgSeqCounter++);
      r->setState(FsmState::Status);
      
      YAML::Node total;
      total["Source"]="Serenity";
      total["DaqBoard"]=daqBoard;
      total["ElectronicsId"]=daqBoard<<22|0x3fffff;

      YAML::Node ne;
      _serenityEncoder.status(ne);
      total["FcEncoder"]=ne;
      
      ////////////////////

      total["LpgbtPair"]["0"]["Id"]=0;

      YAML::Node nm00;
      _serenityMiniDaq[0].status(nm00);
      total["LpgbtPair"]["0"]["MiniDaq"]=nm00;

      YAML::Node nu00;
      _serenityUnpacker[0].status(nu00);
      total["LpgbtPair"]["0"]["Unpacker"]=nu00;

      ////////////////////

      total["LpgbtPair"]["1"]["Id"]=1;

      if(_nMiniDaqs>1) {
	YAML::Node nm01;
	_serenityMiniDaq[1].status(nm01);
	total["LpgbtPair"]["1"]["MiniDaq"]=nm01;
      }

      if(_nUnpackers>1) {
	YAML::Node nu01;
	_serenityUnpacker[1].status(nu01);
	total["LpgbtPair"]["1"]["Unpacker"]=nu01;
      }

      if(_printEnable) std::cout << "Yaml status" << std::endl << total << std::endl;
      
      std::ostringstream sout;
      sout << total;
      r->setString(sout.str());
      
      if(_printEnable) r->print();
      
      ptrFifoShm2->writeIncrement();
      
      writeContinuing();

      return true;
    }

    bool halting() {
      _serenityEncoder.setDefaults();
      _serenityEncoder.print();
      
      _serenityLpgbt.setDefaults();
      _serenityLpgbt.print();
      
      _serenityMiniDaq[0].setDefaults();
      _serenityMiniDaq[0].print();
      if(_nMiniDaqs>1) {
      _serenityMiniDaq[1].setDefaults();
      _serenityMiniDaq[1].print();
      }

      _serenityUnpacker[0].setDefaults();
      _serenityUnpacker[0].print();
      if(_nUnpackers>1) {
      _serenityUnpacker[1].setDefaults();
      _serenityUnpacker[1].print();
      }

      //_serenity10g.setDefaults();
      //_serenity10g.print();

      //_serenity10gx.setDefaults();
      //_serenity10gx.print();

      return true;
    }
    
    bool resetting() {
      return true;
    }

    bool ending() {
      ptrFifoShm2->end();
      if(_printEnable) {
	std::cout << "Ending" << std::endl;
	ptrFifoShm2->print();
      }
      return true;
    }

    //////////////////////////////////////////////

    virtual void halted() {
      /*
      RecordHalted *r;
      if(_printEnable) {
	std::cout << "halted() waiting for record" << std::endl;
	ptrFifoShm2->print();
      }
      while((r=(RecordHalted*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      r->setHeader(_cfgSeqCounter++);

      // Replace with "constants" call to Serenity
      YAML::Node n;
      n["Source"]="Serenity";
      n["HardwareVersion"]="PrototypeV1.1";
      n["PayloadVersion"]=_serenityEncoder.payloadVersion();
      n["ElectronicsId"]=daqBoard<<22|0x3fffff;
       
      std::ostringstream sout;
      sout << n;
      r->setString(sout.str());
      r->print();
      
      ptrFifoShm2->writeIncrement();

      writeContinuing();
      */
    }
    
    virtual void configured() {

      std::cout << "configured() relay = " << _relayNumber << std::endl;

#ifdef HGCROC_JUNK
      RecordConfigured *r;
      for(unsigned i(1);i<=3 && false;i++) {

	while((r=(RecordConfigured*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
	r->setHeader(_cfgSeqCounter++);
	r->setState(FsmState::Configured);
	r->setType(RecordConfigured::HGCROC);
	r->setLocation(0xfe00+i);
	if(_printEnable) r->print();
      
	I2cInstruction i2c;
	
	std::ostringstream sout;
	sout << "HgcrocCfg_ROCs" << i << ".cfg";
	
	std::ifstream fin;
	fin.open(sout.str().c_str());
	if(fin) {
	  char buffer[16];
	  
	  uint16_t add;
	  uint16_t val;
	  uint8_t mask(0xff);

	  std::cout << std::hex << std::setfill('0');

	  fin.getline(buffer,16);
	  while(fin) {
	    assert(buffer[ 1]=='x');
	    assert(buffer[10]=='x');	

	    buffer[ 6]='\0';
	    buffer[13]='\0';
	  
	    std::istringstream sAdd(buffer+ 2);
	    std::istringstream sVal(buffer+11);
	  
	    sAdd >> std::hex >> add;
	    sVal >> std::hex >> val;

	    i2c.setAddress(add);
	    i2c.setMask(mask);
	    i2c.setValue(val&0xff);
	    if(_printEnable) {
	      i2c.print();
	  
	      std::cout << "HGCROC cfg address 0x" << std::setw(4) << add
			<< ", mask 0x" << std::setw(2) << unsigned(mask)
			<< ", value 0x" << std::setw(2) << unsigned(val)
			<< std::endl;
	    }
	    
	    r->addData32(i2c.data());
	    if(_printEnable && add<16) r->print();
	  
	    fin.getline(buffer,16);
	  }
	
	  std::cout << std::dec << std::setfill(' ');
	}
	fin.close();
	if(_printEnable) r->print();

	ptrFifoShm2->writeIncrement();
      }
#endif

#ifdef USE_CONFIGURED
      RecordYaml *ry;
      while((ry=(RecordYaml*)ptrFifoShm2->getWriteRecord())==nullptr) usleep(1000);
      ry->setHeader(_cfgSeqCounter++);
      ry->setState(FsmState::Configured);
      ry->print();
      
      YAML::Node total;
      total["Source"]="Serenity";
      total["DaqBoard"]=daqBoard;
      total["ElectronicsId"]=daqBoard<<22|0x3fffff;

      YAML::Node ne;
      _serenityEncoder.configuration(ne);
      total["FcEncoder"]=ne;

      total["LpgbtPair"]["0"]["Id"]=0;

      YAML::Node nm;
      //nm[0]="NULL";
      _serenityMiniDaq[0].configuration(nm);
      total["LpgbtPair"]["0"]["MiniDaq"]=nm;

      YAML::Node nl;
      _serenityLpgbt.configuration(nl);
      total["LpgbtPair"]["0"]["FcStream"]=nl;
      /*
	total["LpgbtPair"][0]["FcStream"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream0"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream0"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream1"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream1"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream1"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream2"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream2"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream2"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream3"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream3"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream3"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream4"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream4"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream4"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream5"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream5"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream5"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream6"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream6"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream6"]["ctrl.calpulse_type"]=0;
	total["LpgbtPair"][0]["FcStream7"]["fc_cmd.user"]=0x36;
	total["LpgbtPair"][0]["FcStream7"]["ctrl.user_bx"]=0xfff;
	total["LpgbtPair"][0]["FcStream7"]["ctrl.calpulse_type"]=0;
      */
      total["LpgbtPair"]["0"]["Unpacker"]["0"]["Id"]=0;
      total["LpgbtPair"]["0"]["Unpacker"]["1"]["Id"]=1;
      total["LpgbtPair"]["0"]["Unpacker"]["2"]["Id"]=2;
      total["LpgbtPair"]["0"]["Unpacker"]["3"]["Id"]=3;


      total["LpgbtPair"]["1"]["Id"]=1;

      total["LpgbtPair"]["1"]["MiniDaq"]="XXX: YYY";

      total["LpgbtPair"]["1"]["FcStream0"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream0"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream0"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream1"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream1"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream1"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream2"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream2"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream2"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream3"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream3"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream3"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream4"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream4"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream4"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream5"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream5"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream5"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream6"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream6"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream6"]["ctrl.calpulse_type"]=0;
      total["LpgbtPair"]["1"]["FcStream7"]["fc_cmd.user"]=0x36;
      total["LpgbtPair"]["1"]["FcStream7"]["ctrl.user_bx"]=0xfff;
      total["LpgbtPair"]["1"]["FcStream7"]["ctrl.calpulse_type"]=0;

      total["LpgbtPair"]["1"]["Unpacker"]["0"]["Id"]=0;
      total["LpgbtPair"]["1"]["Unpacker"]["1"]["Id"]=1;
      total["LpgbtPair"]["1"]["Unpacker"]["2"]["Id"]=2;
      total["LpgbtPair"]["1"]["Unpacker"]["3"]["Id"]=3;


      YAML::Node lp;
      lp["cmd"]=0x36;
      //_serenityLpgbt.configuration(lp);


      YAML::Node md;
      md["header"]=0x154;
      //_serenityLpgbt.configuration(md);
      //total["LpgbtPair"][1]=md;
      //total["MiniDaq"]=md;

      /*
	YAML::Node pair0;
	pair0["FcStream_0"]="Stuff0";
	total["LpgbtPair"][2]=pair0;

	YAML::Node pair1;
	pair1["FcStream_1"]="Stuff1";
	total["LpgbtPair"][3]=pair1;
      
	YAML::Node unp;
	unp["Unpacker_0"]="Stuff0";
	total["Unpacker"][0]=unp;
	unp["Unpacker_1"]="Stuff1";
	total["Unpacker"][1]=unp;
      */
      //total["LpgbtPair"][0]=lp;
      //total["LpgbtPair"][1]=md;

      
      std::ostringstream sout;
      sout << total;
      ry->setString(sout.str());
      ry->print();

      ptrFifoShm2->writeIncrement();
            
#endif
    }

    virtual void running() {
      _serenityEncoder.uhalWrite("ctrl.tts",1);

      _ptrFsmInterface->setProcessState(FsmState::Running);

      while(_ptrFsmInterface->systemState()==FsmState::Running) usleep(1000);
      
      _serenityEncoder.uhalWrite("ctrl.tts",0);
    }
    
    void paused() {
      _pauseCounter++;
    }

    void writeContinuing() {
      Record *r;
      while((r=ptrFifoShm2->getWriteRecord())==nullptr) usleep(10);
      r->reset(FsmState::Continuing);
      ptrFifoShm2->writeIncrement();
    }

    
  protected:
    RelayWriterDataFifo *ptrFifoShm2;

    unsigned daqBoard;
    uint32_t _saveData;

    uint32_t _cfgSeqCounter;
    uint32_t _evtSeqCounter;
    uint32_t _pauseCounter;

    uint32_t _configuringBCounter;
    uint32_t _keyCfgA;
    std::string _strCfgA;

    uint32_t _relayNumber;
    uint32_t _runNumber;

    uint32_t _runNumberInSuperRun;

    uint32_t _eventNumberInRun;
    uint32_t _eventNumberInConfiguration;
    uint32_t _eventNumberInSuperRun;

    unsigned _nMiniDaqs;
    unsigned _nUnpackers;

    SerenityEncoder _serenityEncoder;
    SerenityLpgbt _serenityLpgbt;
    SerenityMiniDaq _serenityMiniDaq[2];  
    SerenityUnpacker _serenityUnpacker[2];  
    Serenity10g _serenity10g;
    Serenity10gx _serenity10gx;
    SerenityTrgDaq _serenityTrgDaq;
    SerenityAligner _serenityAligner;
    SerenitySlink _serenitySlink;
  };

}

#endif
