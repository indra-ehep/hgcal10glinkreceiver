/*
  g++ -I inc src/ReaderFromLink0.cpp -o bin/ReaderFromLink0.exe
*/

#include "RunFileShm.h"

typedef RunFileShm0 RunFileShm;

#define PORT  1234

#include "ReaderFromLink.cc"
