#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include <zmq.hpp>

#include "FsmInterface.h"
#include "ShmKeys.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

bool ZmqProcessorFsm(uint32_t key, uint16_t port) {
  
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
  
  FsmInterface local;
    
  ShmSingleton<FsmInterface> shmU;
  shmU.setup(key);
  FsmInterface *prcfs=shmU.payload();
  prcfs->print();
  
  FsmInterface::HandshakeState hsOld(prcfs->commandHandshake());
  FsmInterface::HandshakeState hsNew(prcfs->commandHandshake());
  
  prcfs->print();
  
  zmq::message_t request;
  
  bool continueLoop(true);
  
  while(continueLoop) {

    // Get a change from Run Control
    socket.recv(request, zmq::recv_flags::none);
    
    std::cout  << std::endl << "************ GOT REQUEST ******************" << std::endl << std::endl;

    std::memcpy(&local,request.data(),sizeof(FsmInterface));
    local.print();
    
    hsOld=local.commandHandshake();
    assert(hsOld==FsmInterface::Ping ||
	   hsOld==FsmInterface::Propose ||
	   hsOld==FsmInterface::Change ||
	   hsOld==FsmInterface::Repair ||
	   hsOld==FsmInterface::StartStatic);

    // Copy change to local shared memory
    std::memcpy(prcfs,&local,sizeof(FsmInterface));
    prcfs->print();

    // Wait for the processor to respond
    while(hsOld==prcfs->commandHandshake()) usleep(10);

    std::cout  << std::endl << "************ FOUND TRANS ******************" << std::endl << std::endl;
    prcfs->print();

    if(prcfs->processState()==FsmState::Shutdown) {
      std::cout  << std::endl << "************ SEEN SHUTDOWN ******************" << std::endl << std::endl;
      continueLoop=false;
    }
    
    // Send processor response back to Run Control
    socket.send(zmq::buffer(prcfs,sizeof(FsmInterface)), zmq::send_flags::none);
  }

  return false;
}
