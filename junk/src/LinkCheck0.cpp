#include "LinkCheckShm.h"
template<> const key_t ShmSingleton<LinkCheckShm>::theKey=0xce2302;

#define PORT    1234

#include "LinkCheck.cc"
