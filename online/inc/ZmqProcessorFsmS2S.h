#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <unordered_map>

#include <zmq.hpp>
#include <hgcal-zmqserver.h>

#include "SystemParameters.h"
#include "FsmInterface.h"
#include "ShmSingleton.h"
#include "RecordYaml.h"

using namespace Hgcal10gLinkReceiver;

bool ZmqProcessorFsmS2S(uint32_t key, uint16_t port) {

  const std::unordered_map<std::string,std::string> replyMap = {
    {"reset",      "initial"},
    {"initialize", "halted"},
    {"configure",  "configured"},
    {"reconfigure","configured"},
    {"start",      "running"},
    {"stop",       "configured"},
    {"halt",       "halted"},
    {"pause",      "paused"},
    {"resume",     "running"},
    {"end",        "shutdown"}
  };
  
  const std::unordered_map<std::string,FsmState::State> stateMap = {
    {"reset",      FsmState::Resetting},
    {"initialize", FsmState::Initializing},
    {"configure",  FsmState::Configuring},
    {"reconfigure",FsmState::Reconfiguring},
    {"start",      FsmState::Starting},
    {"stop",       FsmState::Stopping},
    {"halt",       FsmState::Halting},
    {"pause",      FsmState::Pausing},
    {"resume",     FsmState::Resuming},
    {"end",        FsmState::Ending}
  };

  /*  
  //using namespace std::chrono_literals;
  //std::chrono::seconds asec(1);
  // initialize the zmq context with a single IO thread
  zmq::context_t context{1};
  
  // construct a REP (reply) socket and bind to interface
  std::ostringstream sout;
  sout << "tcp://*:" << port;
  
  zmq::socket_t socket{context, zmq::socket_type::rep};
  //socket.bind("tcp://*:5555");
  socket.bind(sout.str());
  */
  zmqserver server(port);

  
  FsmInterface local;
    
  ShmSingleton<FsmInterface> shmU;
  shmU.setup(key);
  FsmInterface *prcfs=shmU.payload();
  prcfs->print();
  
  //FsmInterface::Handshake hsOld(prcfs->handshake());
  //FsmInterface::Handshake hsNew(prcfs->handshake());


  Record *r((Record*)&(prcfs->getRecord()));
  RecordYaml *ry((RecordYaml*)&(prcfs->getRecord()));

  FsmState::State *pState(prcfs->getProcessState());
  FsmState::State psOld(*pState);

  //*pState=FsmState::Resetting;
  prcfs->print();
  
  //zmq::message_t request;
  
  bool continueLoop(true);
  reply rep;
  
  while(continueLoop) {

    // Get a change from Run Control
    //socket.recv(request, zmq::recv_flags::none);

    auto req = server.getRequest();

    if( req.is_valid() ){
      std::cout << "got valid request" << std::endl;
      std::cout << "command = " << req.get_command() << std::endl;
      std::cout << "utc_or_counter = " << req.get_utc_or_counter() << std::endl;
      std::cout << "config = " << req.get_config() << std::endl;

      const YAML::Node &yNode(req.get_config());
      //std::string s(yNode.as<std::string>());
      //std::cout << "config string = " << s << std::endl;

      /*
      switch (yNode.Type()) {
      case YAML::Null: {std::cout << "Null" << std::endl;break;}
      case YAML::Scalar: {std::cout << "Scalar" << std::endl;break;}
      case Sequence: {std::cout << "Sequence" << std::endl;break;}
      case Map: {std::cout << "Map" << std::endl;break;}
      case Undefined: {std::cout << "Undefined" << std::endl;break;}
      default: {std::cout << "Default" << std::endl;break;}
      }
      */

      // Special cases
      if(req.get_command()=="Ping") {
	
	std::cout  << std::endl << "************ GOT PING ******************" << std::endl << std::endl;

	std::string repStr;

	if((*pState)==FsmState::Initial   ) repStr="Initial";
	if((*pState)==FsmState::Halted    ) repStr="Halted";
	if((*pState)==FsmState::Configured) repStr="Configured";
	if((*pState)==FsmState::Running   ) repStr="Running";
	if((*pState)==FsmState::Paused    ) repStr="Paused";

	rep = reply( repStr, 0xC0FFE, "Pong" );
	
	std::cout  << std::endl << "************ SENT PONG ******************" << std::endl << std::endl;

      } else if(req.get_command()=="ColdStart") {
	
	std::cout  << std::endl << "************ GOT COLDSTART ******************" << std::endl << std::endl;

	r->reset(FsmState::Initial);

	  
	  prcfs->print();
	  
	  // Wait for the processor to respond
	  while(psOld==(*pState)) usleep(1000);
	  psOld=(*pState);
	  
	  std::cout  << std::endl << "************ PREPARED ******************" << std::endl << std::endl;
	  prcfs->print();

	/*
	std::string repStr;

	if((*pState)==FsmState::Initial   ) repStr="Initial";
	if((*pState)==FsmState::Halted    ) repStr="Halted";
	if((*pState)==FsmState::Configured) repStr="Configured";
	if((*pState)==FsmState::Running   ) repStr="Running";
	if((*pState)==FsmState::Paused    ) repStr="Paused";

	rep = reply( repStr, 0xC0FFE, "Pong" );
	*/

	  rep = reply( "Initial", 0xC0FFE, "no comment" );
	std::cout  << std::endl << "************ SENT PONG ******************" << std::endl << std::endl;

      } else {

	auto it = replyMap.find( req.get_command() );
	if( it!=replyMap.end() ) {
	  
	  std::cout  << std::endl << "************ GOT REQUEST ******************" << std::endl << std::endl;
	  
	  // Put prepare transient in local shared memory
	  auto it2 = stateMap.find(req.get_command());
	  FsmState::State trans(it2->second);
	  r->reset(trans);
	  
	  prcfs->print();
	  
	  // Wait for the processor to respond
	  while(psOld==(*pState)) usleep(1000);
	  psOld=(*pState);
	  
	  std::cout  << std::endl << "************ PREPARED ******************" << std::endl << std::endl;
	  prcfs->print();
	  ry->print();
	  
	  // Set true transient in local shared memory
	  r->setUtc(req.get_utc_or_counter());
	  ry->print();

	  if(trans==FsmState::Configuring || trans==FsmState::Starting) {
	    std::ostringstream sout;
	    sout << req.get_config();
	    ry->setString(sout.str());
	    ry->print();
	  }

	  prcfs->print();
	  
	  // Wait for the processor to respond
	  while(psOld==(*pState)) usleep(1000);
	  psOld=(*pState);
	  
	  std::cout  << std::endl << "************ FINISHED TRANSIENT ******************" << std::endl << std::endl;
	  prcfs->print();
	  
	  // Put static in local shared memory
	  r->reset(FsmState::staticStateAfterTransient(trans));
	  prcfs->print();
	  
	  // Wait for the processor to respond
	  while(psOld==(*pState)) usleep(1000);
	  psOld=(*pState);
	  
	  std::cout  << std::endl << "************ FINISHED FINITE STATIC ******************" << std::endl << std::endl;
	  prcfs->print();
	  
	  
	  /*
	    if(prcfs->processState()==FsmState::Shutdown) {
	    std::cout  << std::endl << "************ SEEN SHUTDOWN ******************" << std::endl << std::endl;
	    continueLoop=false;
	    }
	  */
	  
	  
	  // Send processor response back to Run Control
	  //socket.send(zmq::buffer(pState,sizeof(FsmState::State)), zmq::send_flags::none);
	  
	  rep = reply( it->second, 0xC0FFE, "no comment" );
	  
	} else { // ! command is in map
	  std::cout  << std::endl << "************ NOT COMMAND ******************" << std::endl << std::endl;
	  rep = reply( "ERROR", 0xC0FFE, "no comment" );
	}
      }	
    
      server.sendReply( rep );

    } else { // ! reply is_valid    
      std::cout  << std::endl << "************ NOT VALID ******************" << std::endl << std::endl;
      rep = reply( "ERROR", 0xC0FFE, "no comment" );
    }
  }
  return false;
}
