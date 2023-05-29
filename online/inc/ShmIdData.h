#ifndef ShmIdData_HH
#define ShmIdData_HH

#include <iostream>

class ShmIdData : public shmid_ds {

public:
  short numberAttached() const {
    return shm_nattch;
  }
  
private:
};

#endif
