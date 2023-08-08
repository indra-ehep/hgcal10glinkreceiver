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
    std::cout << "Initial ZMQ_IPV6 = " << context.get(zmq::ctxopt::ipv6) << std::endl;
    //context.set(zmq::ctxopt::ipv6, 1); // DOES NOT WORK; FAILS TO CONNECT
    std::cout << "Final   ZMQ_IPV6 = " << context.get(zmq::ctxopt::ipv6) << std::endl;

    //std::chrono::seconds asec(1);

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::req};
    //socket.connect("tcp://localhost:5555");
    //socket.connect("tcp://serenity-2368-03-i5:5555");
    //socket.connect("tcp://cebrown-desktop:5555");

    std::ostringstream sout;
    if(key==ProcessorStageFsmShmKey) {
      sout << "tcp://rasppitifr:" << port;
    } else {
      sout << "tcp://hgcbeamtestpc:" << port;
    }

    std::cout << "Connecting to " << sout.str() << std::endl;
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

    const Record *r((const Record*)&(prcfs->record()));
    const RecordHeader *rh((const RecordHeader*)&(prcfs->record()));

    RecordHeader rhOld(*rh);

    FsmState::State pState;

    FsmInterface::Handshake hsOld(FsmInterface::Idle);
      
    bool continueLoop(true);

    while(continueLoop) {

      assert(hsOld==FsmInterface::Idle ||
	     hsOld==FsmInterface::Ready ||
	     hsOld==FsmInterface::Completed);

      // Wait for change from Run Control
      //while(hsOld==prcfs->handshake()) usleep(1000);
      while(rhOld==(*rh)) usleep(1000);
      rhOld=(*rh);
      
      std::cout  << std::endl << "************ FOUND TRANS ******************" << std::endl << std::endl;
      prcfs->print();

      // Send change to Processor
      //socket.send(zmq::buffer(prcfs,sizeof(FsmInterface)), zmq::send_flags::none);
      socket.send(zmq::buffer(r,r->totalLengthInBytes()), zmq::send_flags::none);
      
      // wait for reply from Processor
      zmq::message_t reply{};
      socket.recv(reply, zmq::recv_flags::none);
      
      std::cout  << std::endl << "************ GOT REPLY ******************" << std::endl << std::endl;
      
      // Copy reply to local cache
      //std::memcpy(&local,reply.data(),sizeof(FsmInterface));
      std::memcpy(&pState,reply.data(),sizeof(FsmState::State));
            
      if(pState==FsmState::Shutdown) {
	std::cout  << std::endl << "************ SEEN SHUTDOWN ******************" << std::endl << std::endl;
	continueLoop=false;
      }
      
      // Copy reply to shared memory
      prcfs->setProcessState(pState);
      //std::memcpy(prcfs,&local,sizeof(FsmInterface));
      prcfs->print();
    }
    return true;
}
