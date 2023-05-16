#include <string>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>

#include <zmq.hpp>

#include "SystemParameters.h"
#include "DataFifo.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

bool ZmqSerenityProcessorFifo(uint32_t key, uint16_t port) {
  std::cout << "ZmqSerenityProcessorFifo called with key = 0x"
	    << std::hex << std::setfill('0')
	    << std::setw(8) << key << ", port = 0x"
	    << std::setw(4) << port
	    << std::dec << std::setfill(' ')
	    << std::endl;

  // initialize the zmq context with a single IO thread
    zmq::context_t context{1};
    //std::chrono::seconds asec(1);

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::req};
    //socket.connect("tcp://localhost:5555");
    //socket.connect("tcp://serenity-2368-03-i5:5556");
    //socket.connect("tcp://cebrown-desktop:5556");

    std::ostringstream sout;
    sout << "tcp://cebrown-desktop:" << port;
    socket.connect(sout.str());


    ShmSingleton< DataFifoT<6,1024> > shmU;
    shmU.setup(key);
    DataFifoT<6,1024> *prcfs=shmU.payload();
    prcfs->ColdStart();
    prcfs->print();

    uint64_t buffer[1024];

    bool continueLoop(true);

    while(continueLoop) {

      // Wait for FIFO to have data to be read
      while(prcfs->readable()==0) {
	if(prcfs->isEnded()) return true;
	usleep(10);
      }

      std::cout  << std::endl << "************ DATA READY ******************" << std::endl << std::endl;
      prcfs->print();
      
      uint16_t n(prcfs->read(buffer));
      std::cout << "Size in 64-bit words = " << n << std::endl;
      prcfs->print();
      
      // Send data to PC
      socket.send(zmq::buffer(buffer,8*n), zmq::send_flags::none);

      zmq::message_t reply{};
      socket.recv(reply, zmq::recv_flags::none);

    }
    return true;
}
