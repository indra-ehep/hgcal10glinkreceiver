#include <string>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>

#include <zmq.hpp>

#include "DataFifo.h"
#include "SystemParameters.h"
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
    //socket.connect("tcp://serenity-2368-03-i5:5556");
    socket.connect("tcp://cebrown-desktop:5556");

    ShmSingleton< DataFifoT<6,1024> > shmU;
    shmU.setup(ProcessorFastControlDataShmKey);
    DataFifoT<6,1024> *prcfs=shmU.payload();
    prcfs->print();

    uint64_t buffer[1024];

    while(true) {
      while(!prcfs->readable()) usleep(10);
      
	std::cout  << std::endl << "************ FOUND TRANS ******************" << std::endl << std::endl;
	prcfs->print();

	uint16_t n(prcfs->read(buffer));
	std::cout << "Size = " << n << std::endl;

        socket.send(zmq::buffer(buffer,n), zmq::send_flags::none);
    }
    return 0;
}
