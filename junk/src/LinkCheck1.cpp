#include "LinkCheckShm.h"
template<> const key_t ShmSingleton<LinkCheckShm>::theKey=0xce2303;

#define PORT    1235

#include "LinkCheck.cc"
