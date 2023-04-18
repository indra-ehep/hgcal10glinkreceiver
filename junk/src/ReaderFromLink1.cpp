/*
  g++ -I inc src/ReaderFromLink1.cpp -o bin/ReaderFromLink1.exe
*/

#include "RunFileShm.h"

typedef RunFileShm1 RunFileShm;

#define PORT  1235

#include "ReaderFromLink.cc"
