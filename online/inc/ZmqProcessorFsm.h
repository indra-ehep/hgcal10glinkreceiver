#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include <unistd.h>

#include <zmq.hpp>

#include "SystemParameters.h"
#include "FsmInterface.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

bool ZmqProcessorFsm(uint32_t key, uint16_t port) {
  
  //using namespace std::chrono_literals;
  //std::chrono::seconds asec(1);
  // initialize the zmq context with a single IO thread
  zmq::context_t context{1};
  std::cout << "Initial ZMQ_IPV6 = " << context.get(zmq::ctxopt::ipv6) << std::endl;
  //context.set(zmq::ctxopt::ipv6, 1);
  std::cout << "Final   ZMQ_IPV6 = " << context.get(zmq::ctxopt::ipv6) << std::endl;
  
  // construct a REP (reply) socket and bind to interface
  std::ostringstream sout;
  sout << "tcp://*:" << port;
  
  zmq::socket_t socket{context, zmq::socket_type::rep};
  //socket.bind("tcp://*:5555");

  std::cout << "Binding with " << sout.str() << std::endl;
  socket.bind(sout.str());
  
  FsmInterface local;
    
  ShmSingleton<FsmInterface> shmU;
  shmU.setup(key);
  FsmInterface *prcfs=shmU.payload();
  prcfs->print();
  
  //FsmInterface::Handshake hsOld(prcfs->handshake());
  //FsmInterface::Handshake hsNew(prcfs->handshake());


  Record *r((Record*)&(prcfs->getRecord()));

  FsmState::State *pState(prcfs->getProcessState());
  FsmState::State psOld(*pState);

  prcfs->print();
  
  zmq::message_t request;
  
  bool continueLoop(true);
  
  while(continueLoop) {

    // Get a change from Run Control
    socket.recv(request, zmq::recv_flags::none);
    
    std::cout  << std::endl << "************ GOT REQUEST ******************" << std::endl << std::endl;

    std::memcpy(r,request.data(),request.size());
    //local.print();
    /*
    hsOld=local.handshake();
    assert(hsOld==FsmInterface::Ping ||
	   hsOld==FsmInterface::Prepare ||
	   hsOld==FsmInterface::GoToTransient ||
	   hsOld==FsmInterface::GoToStatic);
    */
    // Copy change to local shared memory
    //std::memcpy(prcfs,&local,sizeof(FsmInterface));
    prcfs->print();

    // Wait for the processor to respond
    //while(hsOld==prcfs->handshake()) usleep(1000);
    while(psOld==(*pState)) usleep(1000);
    psOld=(*pState);

    std::cout  << std::endl << "************ FOUND TRANS ******************" << std::endl << std::endl;
    prcfs->print();

    if(prcfs->processState()==FsmState::Shutdown) {
      std::cout  << std::endl << "************ SEEN SHUTDOWN ******************" << std::endl << std::endl;
      continueLoop=false;
    }
    
    // Send processor response back to Run Control
    //socket.send(zmq::buffer(prcfs,sizeof(FsmInterface)), zmq::send_flags::none);
    socket.send(zmq::buffer(pState,sizeof(FsmState::State)), zmq::send_flags::none);
  }

  return false;
}
