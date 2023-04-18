#ifndef ShmObject_HH
#define ShmObject_HH

/******************************************************************************
 ShmObject - handles objects in shared memory

 Usage: e.g. to put 16 unsigned's into shared memory using an arbitrary
 key value of 34567

    ShmObject<unsigned,16> shmUnsigned(34567);
    unsigned *p(shmUnsigned.payload());
 
 The same code is used in all processes accessing the memory. The data
 can then be manipulated as usual using p[0], p[1], etc.

 *****************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <new>
#include <cassert>
#include <iostream>
#include <iomanip>

#include "ShmIdData.hh"

template <class Payload, unsigned Number=1> class ShmObject {
  
public:

  ShmObject(const key_t key, 
	    const bool allowCreation=true,
	    const int access = 0666)
    : _shmId(-1), _payload(0) {

    if(Number==0) return;

    bool createdArea(false);

    // Try to get an existing area; this fails if not yet existing
    // (or if existing but labelled for deletion)

    _shmId=shmget(key,1,access);
    
    if(_shmId<0) {
      if(!allowCreation) {
	std::cerr << "ShmObject<>::ctor() shmget(" << key << ", 1, 0" 
		  << std::oct << access << std::dec << ") ";
	perror(0);
	return;
      }

      // Try to create a new memory area

      _shmId=shmget(key,Number*sizeof(Payload),IPC_CREAT|access);

      if(_shmId<0) {
	std::cerr << "ShmObject<>::ctor() shmget(" << key << ", "
		  << Number*sizeof(Payload) << ", 0"
		  << std::oct << (IPC_CREAT|access) << std::dec << ") ";
	perror(0);
	return;
      }
      createdArea=true;
    }

    // Attach to the shared memory; this must only be called once
    // per process as it increments the shm_nattch counter
 
    _payload=static_cast<Payload*>(shmat(_shmId,0,0));

    if(_payload==0) {
      std::cerr << "ShmObject<>::ctor() shmat(" << _shmId << ", "
		<< 0 << ", " << 0 << ") ";
      perror(0);
      return;
    }
	
    // If this was a new memory area, initialise the objects within it
    // by calling their constructors; this cannot new whole array in one
    // go using "new Payload[Number]" as it might (?) not fit

    if(createdArea) {
      for(unsigned i(0);i<Number;i++) {
	if((new(_payload+i) Payload)==0) {
	  std::cerr << "ShmObject<>::ctor() new " << _payload+i << " ";
	  perror(0);
	  return;
	}
      }
    }
  }


  ~ShmObject() {
    if(_shmId<0) return;

    // Get status of shared memory and find number of attached processes

    unsigned n(0);

    ShmIdData d;
    if(shmctl(_shmId,IPC_STAT,&d)<0) {
	std::cerr << "ShmObject<>::dtor() shmctl(" << _shmId << ", "
		  << IPC_STAT << ", " << &d << ") ";
	perror(0);

    } else {
      n=d.numberAttached();
    }

    // If this is the last process still attached to the memory, then
    // label the memory for deletion after this processes detaches and
    // call the destructors for the objects in the memory

    if(n<=1) {
      if(shmctl(_shmId,IPC_RMID,0)<0) {
	std::cerr << "ShmObject<>::dtor() shmctl(" << _shmId << ", "
		  << IPC_RMID << ", 0) ";
	perror(0);
      }

      if(_payload!=0) {
	for(unsigned i(0);i<Number;i++) {
	  std::cout << "Deleting payloads" << std::endl;

	  //delete (_payload+i);
	  (_payload+i)->~Payload();
	}
      }
    }

    // Finally detact memory from the process

    if(_payload==0) return;

    if(shmdt(_payload)<0) {
      std::cerr << "ShmObject<>::dtor() shmdt ";
      perror(0);
      return;
    }
  }


  Payload* const payload() {
    return _payload;
  }

  const Payload* const payload() const {
    return _payload;
  }

  Payload* const payload(unsigned n) {
    if(_payload==0 || n>=Number) return 0;
    return _payload+n;
  }

  const Payload* const payload(unsigned n) const {
    if(_payload==0 || n>=Number) return 0;
    return _payload+n;
  }


private:
  int _shmId;
  Payload* _payload;
};

#endif
