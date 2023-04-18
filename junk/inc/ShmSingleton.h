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

#include "ShmIdData.hh"

template <class Payload> class ShmSingleton {

 public:

  ShmSingleton(int access = 0666) : thePayload(0) {//, theKey(123456789) {
    bool created(false);

    shmId = shmget(theKey, 1, access);
    if(shmId == -1) {
      std::cerr << "ShmSingleton<>::ctor() shmget(" << theKey << ","
		<< 1 << "," << std::oct << access
		<< std::dec << ")" << std::endl;
      //perror(0);
    
      std::cerr << "shmget successful" << std::endl;

      shmId = shmget(theKey, sizeof(Payload), IPC_CREAT | access);
      //shmId = shmget(theKey, 1, IPC_CREAT | access);

      if(shmId == -1) {
	//      perror("ShmSingleton<>::ctor() shmget");
	std::cerr << "ShmSingleton<>::ctor() shmget(" << theKey << ","
		  << sizeof(Payload) << "," << std::oct << (IPC_CREAT | access) 
	  		  << std::dec << ")" << std::endl;
	perror(0);
	return;
      }
      std::cerr << "Created successfully" << std::endl;
      created = true;
      
    } else {
      std::cerr << "Connected successfully" << std::endl;
    }
    
    thePayload=(Payload*)shmat(shmId, 0, 0);
    if(thePayload == 0) {
      perror("ShmSingleton<>::ctor() shmat");
      return;
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
  }

  ~ShmSingleton() {
    if(thePayload != 0) {
      ShmIdData ds;
      shmctl(shmId, IPC_STAT, &ds);
      std::cout << "Number attached = " << ds.numberAttached() << std::endl;
      if(ds.numberAttached()==1) delete thePayload;

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
  static const key_t theKey;

 private:
  int shmId;
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
