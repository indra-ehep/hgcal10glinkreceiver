#!/usr/bin/env bash
#g++ -Icommon/inc econt_data_validation.cpp -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o econt_data_validation.exe
#g++ -I common/inc emul_econt.cpp -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_econt.exe
g++ -I common/inc read_pedestal.cpp -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o read_pedestal.exe
