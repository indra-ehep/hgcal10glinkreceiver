/*
g++ -O3 -I inc -I hgcal10glinkreceiver src/WriterToFile0.cpp -o bin/WriterToFile0.exe
*/

#include "RunFileShm.h"

#define LINKNUMBER 0
typedef RunFileShm0 RunFileShm;

#include "WriterToFile.cc"
