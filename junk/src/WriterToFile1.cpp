/*
g++ -O3 -I inc -I hgcal10glinkreceiver src/WriterToFile1.cpp -o bin/WriterToFile1.exe
*/

#include "RunFileShm.h"

#define LINKNUMBER 1
typedef RunFileShm1 RunFileShm;

#include "WriterToFile.cc"
