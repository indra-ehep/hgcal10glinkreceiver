#include <string>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <zmq.hpp>

#include "SystemParameters.h"
#include "FsmInterface.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

bool ZmqRunControlFsm(uint32_t key, uint16_t port) {

    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};
    //std::chrono::seconds asec(1);

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::req};
    //socket.connect("tcp://localhost:5555");
    //socket.connect("tcp://serenity-2368-03-i5:5555");
    //socket.connect("tcp://cebrown-desktop:5555");

    std::ostringstream sout;
    sout << "tcp://cebrown-desktop:" << port;
    socket.connect(sout.str());

    // set up some static data to send
    //const std::string data{"Hello"};
    uint64_t data[2*1024];
    uint64_t *p;

    FsmInterface local;

    ShmSingleton<FsmInterface> shmU;
    shmU.setup(key);
    FsmInterface *prcfs=shmU.payload();
    prcfs->coldStart();
    prcfs->print();

    FsmInterface::Handshake hsOld(FsmInterface::Idle);
      
    bool continueLoop(true);

    while(continueLoop) {

      assert(hsOld==FsmInterface::Idle ||
	     hsOld==FsmInterface::Ready ||
	     hsOld==FsmInterface::Completed);

      // Wait for change from Run Control
      while(hsOld==prcfs->handshake()) usleep(10);

	std::cout  << std::endl << "************ FOUND TRANS ******************" << std::endl << std::endl;
	prcfs->print();

	// Send change to Processor
        socket.send(zmq::buffer(prcfs,sizeof(FsmInterface)), zmq::send_flags::none);
        
        // wait for reply from Processor
        zmq::message_t reply{};
        socket.recv(reply, zmq::recv_flags::none);

	std::cout  << std::endl << "************ GOT REPLY ******************" << std::endl << std::endl;

	// Copy reply to local cache
	std::memcpy(&local,reply.data(),sizeof(FsmInterface));
	local.print();

	if(local.processState()==FsmState::Shutdown) {
	  std::cout  << std::endl << "************ SEEN SHUTDOWN ******************" << std::endl << std::endl;
	  continueLoop=false;
	}

	hsOld=local.handshake();

	// Copy reply to shared memory
	std::memcpy(prcfs,&local,sizeof(FsmInterface));
    }
    return true;
}
