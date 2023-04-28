#include <string>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <zmq.hpp>

#include "FsmInterface.h"
#include "ShmKeys.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

int main()
{
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};
    std::chrono::seconds asec(1);

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::req};
    //socket.connect("tcp://localhost:5555");
    //socket.connect("tcp://serenity-2368-03-i5:5555");
    socket.connect("tcp://cebrown-desktop:5555");

    // set up some static data to send
    //const std::string data{"Hello"};
    uint64_t data[2*1024];
    uint64_t *p;

    //RunControlFsmShm rcfs;
    //RunControlFsmShm *prcfs(&rcfs);

    //RunControlFsmShm::HandshakeState hOld(prcfs->_handshakeState);

    //prcfs->_handshakeState=RunControlFsmShm::Ping;

    FsmInterface local;

    ShmSingleton<FsmInterface> shmU;
    shmU.setup(ProcessorDaqLink2ShmKey);
    FsmInterface *prcfs=shmU.payload();
    prcfs->print();

    FsmInterface::HandshakeState hsOld(FsmInterface::Idle);
      
    //std::memcpy(data,prcfs,sizeof(RunControlFsmShm));
    //std::cout << sizeof(RunControlCommand) << std::endl;
    //std::cout << sizeof(RunControlResponse) << std::endl;
    std::cout << sizeof(FsmInterface::HandshakeState) << std::endl;
    std::cout << sizeof(FsmInterface) << std::endl;


    //prcfs->_handshakeState=RunControlFsmShm::StaticState;
    //hsOld=prcfs->_handshakeState;

    while(true) {
      /*
      data[0]=prcfs->_handshakeState;

      unsigned n(rand()%10);
      data[0]=n;
      for(unsigned i(1);i<=n;i++) data[i]=1000*n+i;
      */

      //prcfs->_handshakeState=(RunControlFsmShm::HandshakeState)request_num;

      assert(hsOld==FsmInterface::Idle ||
	     hsOld==FsmInterface::Accepted ||
	     hsOld==FsmInterface::Rejected ||
	     hsOld==FsmInterface::Changed ||
	     hsOld==FsmInterface::Repaired);

        // send the request message
      //std::cout << "Sending Hello " << request_num << "..." << std::endl;
        //socket.send(zmq::buffer(data), zmq::send_flags::none);
        //socket.send(zmq::buffer(data,8*(n+1)), zmq::send_flags::none);
      //prcfs->print();
      /*
      if((hsOld==RunControlFsmShm::StaticState && prcfs->_handshakeState==RunControlFsmShm::Ping       ) ||
	 (hsOld==RunControlFsmShm::StaticState && prcfs->_handshakeState==RunControlFsmShm::Prepare    ) ||
	 (hsOld==RunControlFsmShm::Accepted    && prcfs->_handshakeState==RunControlFsmShm::Change     ) ||
	 (hsOld==RunControlFsmShm::Rejected    && prcfs->_handshakeState==RunControlFsmShm::Repair     ) ||
	 (hsOld==RunControlFsmShm::Changed     && prcfs->_handshakeState==RunControlFsmShm::StartStatic) ||
	 (hsOld==RunControlFsmShm::Repaired    && prcfs->_handshakeState==RunControlFsmShm::StartStatic)) {
      */
      while(hsOld==prcfs->commandHandshake());

	std::cout  << std::endl << "************ FOUND TRANS ******************" << std::endl << std::endl;
	prcfs->print();

        socket.send(zmq::buffer(prcfs,sizeof(FsmInterface)), zmq::send_flags::none);
        
        // wait for reply from server
        zmq::message_t reply{};
        socket.recv(reply, zmq::recv_flags::none);

	std::cout  << std::endl << "************ GOT REPLY ******************" << std::endl << std::endl;

	std::memcpy(&local,reply.data(),sizeof(FsmInterface));
	local.print();
	hsOld=local.commandHandshake();

	std::memcpy(prcfs,&local,sizeof(FsmInterface));
    }
    return 0;
}
