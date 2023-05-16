#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>

#include <zmq.hpp>

#include "DataFifo.h"
#include "SystemParameters.h"
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
    socket.bind("tcp://*:5556");
    //socket.connect("tcp://serenity-2368-03-i5:5556");
    
    // prepare some static data for responses
    const std::string data{"World"};

    ShmSingleton< DataFifoT<6,1024> > shmU;
    shmU.setup(ProcessorDaqLink2FifoShmKey);
    DataFifoT<6,1024> *prcfs=shmU.payload();
    prcfs->print();

    zmq::message_t request;

    while(true) {
        // receive a request from client
        socket.recv(request, zmq::recv_flags::none);

	std::cout  << std::endl << "************ GOT REQUEST ******************" << std::endl << std::endl;

	std::cout << "Size = " << request.size() << std::endl;
	
        //std::cout << "Received " << request.to_string() << std::endl;
	//std::memcpy(prcfs,request.data(),sizeof(RunControlFsmShm));

	//std::memcpy(,request.data(),request.size());
	prcfs->print();
	assert(prcfs->write((request.size()+7)/8,(uint64_t*)request.data()));
	prcfs->print();
    }

    return 0;
}
