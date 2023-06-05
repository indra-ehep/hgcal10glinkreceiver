#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>

#include <zmq.hpp>

#include "SystemParameters.h"
#include "DataFifo.h"
#include "ShmSingleton.h"

using namespace Hgcal10gLinkReceiver;

bool ZmqPcProcessorFifo(uint32_t key, uint16_t port) {
  std::cout << "ZmqPcProcessorFifo called with key = 0x"
            << std::hex << std::setfill('0')
            << std::setw(8) << key << ", port = 0x"
            << std::setw(4) << port
            << std::dec << std::setfill(' ')
            << std::endl;
  
  //using namespace std::chrono_literals;
  //std::chrono::seconds asec(1);
  // initialize the zmq context with a single IO thread
  zmq::context_t context{1};
  
  // construct a REP (reply) socket and bind to interface
  zmq::socket_t socket{context, zmq::socket_type::rep};
  std::ostringstream sout;
  sout << "tcp://*:" << port;
  //socket.bind("tcp://*:5556");
  //socket.connect("tcp://serenity-2368-03-i5:5556");
  socket.bind(sout.str());
  
  ShmSingleton<RelayWriterDataFifo> shmU;
  shmU.setup(key);
  RelayWriterDataFifo *prcfs=shmU.payload();
  prcfs->print();
  
  zmq::message_t request;

  char z('z');
  
  bool continueLoop(true);

  while(continueLoop) {

    // Receive data from Serenity
    socket.recv(request, zmq::recv_flags::none);
    
    std::cout  << std::endl << "************ GOT DATA ******************" << std::endl << std::endl;
    
    std::cout << "Size = " << request.size() << std::endl;
    
    //std::memcpy(,request.data(),request.size());
    prcfs->print();

    // Write into local PC FIFO
    //assert(prcfs->write((request.size()+7)/8,(uint64_t*)request.data()));
    assert(prcfs->writeable()>0);
    prcfs->getWriteRecord()->deepCopy((Record*)request.data());
    prcfs->print();

    socket.send(zmq::buffer(&z,1), zmq::send_flags::none);

    if(prcfs->isEnded()) continueLoop=false;
  }

  return true;
}
