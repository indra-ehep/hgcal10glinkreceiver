#ifndef ShmSingleton_HH
#define ShmSingleton_HH

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <new>
#include <cassert>
#include <iostream>
#include <iomanip>

#include "ShmIdData.h"

template <class Payload> class ShmSingleton {

 public:

 ShmSingleton() : thePayload(0) {
  }
  /*
    ShmSingleton(bool sup=false, int access = 0666) : thePayload(0) {
    if(sup) {
    theNewKey=theKey;
    setup(theNewKey,access);
    }
    }
  */
  Payload* setup(key_t theNewKey, int access = 0666) {
    std::cout << "ShmSingleton<>::setup key = 0x"
	      << std::hex << std::setfill('0')
	      << std::setw(8) << theNewKey
	      << std::dec << std::setfill(' ')
	      << std::endl;

    bool created(false);

    shmId = shmget(theNewKey, 1, access);
    if(shmId == -1) {
      std::cerr << "ShmSingleton<>::ctor() shmget(" << unsigned(theNewKey)
		<< "," << 1 << "," << std::oct << access
		<< std::dec << ")" << std::endl;
      //perror(0);
    
      std::cerr << "shmget successful" << std::endl;

      shmId = shmget(theNewKey, sizeof(Payload), IPC_CREAT | access);
      //shmId = shmget(theNewKey, 1, IPC_CREAT | access);

      if(shmId == -1) {
	//      perror("ShmSingleton<>::ctor() shmget");
	std::cerr << "ShmSingleton<>::ctor() shmget(" << theNewKey << ","
		  << sizeof(Payload) << "," << std::oct << (IPC_CREAT | access) 
		  << std::dec << ")" << std::endl;
	perror(0);
	return nullptr;
      }
      std::cerr << "Created successfully" << std::endl;
      created = true;
      
    } else {
      std::cerr << "Connected successfully" << std::endl;
    }
    
    thePayload=(Payload*)shmat(shmId, 0, 0);
    if(thePayload == 0) {
      perror("ShmSingleton<>::ctor() shmat");
      return nullptr;
    }

    if(created) {
      //      thePayload->reset();
      Payload *p=new(thePayload) Payload;
      assert(p==thePayload);
    }
    
    //remove();
    /*
      ShmIdData ds;
      shmctl(shmId, IPC_STAT, &ds);
      std::cout << "Number attached = " << theShmIdData->numberAttached() << std::endl;
    */
    return thePayload;
  }

  ~ShmSingleton() {
    if(thePayload != 0) {
      ShmIdData ds;
      shmctl(shmId, IPC_STAT, &ds);
      //std::cout << "Dtor: number attached = "
      //<< ds.numberAttached() << std::endl;
      //if(ds.numberAttached()==1) delete thePayload;

      if(shmdt(thePayload) == -1) {
	perror("ShmSingleton<>::dtor() shmdt");
	return;
      }
    }
  }

  unsigned numberAttached() const {
    ShmIdData ds;
    shmctl(shmId, IPC_STAT, &ds);
    return ds.numberAttached();
  }

  Payload* payload() {
    return thePayload;
  }

  void remove() {
    if(shmId == -1) {
      perror("ShmSingleton<>::remove() shmget");
      return;
    }
    shmctl(shmId, IPC_RMID, 0);
  }

  // Needs to be public
  //static const key_t theKey;

 private:
  int shmId;
  key_t theNewKey;
  Payload* thePayload;
};

//class RunFileShm;
//template<> const key_t ShmSingleton<RunFileShm>::theKey=123456789;
/*
  class Command;
  const key_t ShmSingleton<Command>::theKey(123456789);

  class RecordBuffer;
  const key_t ShmSingleton<RecordBuffer>::theKey(123456788);

  class VmeCrate;
  const key_t ShmSingleton<VmeCrate>::theKey(123456787);

  class VmeInterrupt;
  const key_t ShmSingleton<VmeInterrupt>::theKey(123456786);
*/
#endif
