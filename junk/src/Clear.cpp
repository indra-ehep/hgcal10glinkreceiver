/*
g++ -I inc src/Clear.cpp -o bin/Clear.exe
 */

#include <iostream>

#include "RunFileShm.h"

int main(int argc, char *argv[]) {
  system("ipcs -m");

  std::ostringstream s0;
  s0 << "ipcrm -M " << ShmSingleton<RunFileShm0>::theKey;
  system(s0.str().c_str());

  std::ostringstream s1;
  s1 << "ipcrm -M " << ShmSingleton<RunFileShm1>::theKey;
  system(s1.str().c_str());

  //system("ipcrm -M 0x075bcd15");

  system("ipcs -m");
  return 0;
}
