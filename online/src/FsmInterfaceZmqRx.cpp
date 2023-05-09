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

int main() 
{
    //using namespace std::chrono_literals;
    std::chrono::seconds asec(1);
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REP (reply) socket and bind to interface
    zmq::socket_t socket{context, zmq::socket_type::rep};
    socket.bind("tcp://*:5555");

    // prepare some static data for responses
    const std::string data{"World"};

    FsmInterface local;
    
    ShmSingleton<FsmInterface> shmU;
    shmU.setup(ProcessorDaqLink2FsmShmKey);
    FsmInterface *prcfs=shmU.payload();
    prcfs->print();

    FsmInterface::HandshakeState hsOld(prcfs->commandHandshake());
    FsmInterface::HandshakeState hsNew(prcfs->commandHandshake());

    prcfs->print();

    zmq::message_t request;

    while(true) {
        // receive a request from client
        socket.recv(request, zmq::recv_flags::none);

	std::cout  << std::endl << "************ GOT REQUEST ******************" << std::endl << std::endl;

        //std::cout << "Received " << request.to_string() << std::endl;
	//std::memcpy(prcfs,request.data(),sizeof(RunControlFsmShm));

	std::memcpy(&local,request.data(),sizeof(FsmInterface));
	local.print();

	hsOld=local.commandHandshake();
	assert(hsOld==FsmInterface::Ping ||
	       hsOld==FsmInterface::Propose ||
	       hsOld==FsmInterface::Change ||
	       hsOld==FsmInterface::Repair ||
	       hsOld==FsmInterface::StartStatic);

	std::memcpy(prcfs,&local,sizeof(FsmInterface));
	prcfs->print();
    /*
	if((hsOld==RunControlFsmShm::Ping        && prcfs->_handshakeState==RunControlFsmShm::StaticState) ||
	   (hsOld==RunControlFsmShm::Prepare     && prcfs->_handshakeState==RunControlFsmShm::Accepted   ) ||
	   (hsOld==RunControlFsmShm::Prepare     && prcfs->_handshakeState==RunControlFsmShm::Rejected   ) ||
	   (hsOld==RunControlFsmShm::Change      && prcfs->_handshakeState==RunControlFsmShm::Changed    ) ||
	   (hsOld==RunControlFsmShm::Repair      && prcfs->_handshakeState==RunControlFsmShm::Repaired   ) ||
	   (hsOld==RunControlFsmShm::StartStatic && prcfs->_handshakeState==RunControlFsmShm::StaticState)) {
	*/
	while(hsOld==prcfs->commandHandshake()) usleep(10);

	std::cout  << std::endl << "************ FOUND TRANS ******************" << std::endl << std::endl;
	prcfs->print();

	// send the reply to the client
        //socket.send(zmq::buffer(data), zmq::send_flags::none);
        socket.send(zmq::buffer(prcfs,sizeof(FsmInterface)), zmq::send_flags::none);
    }

    return 0;
}
