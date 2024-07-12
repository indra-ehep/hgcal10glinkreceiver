#!/usr/bin/env bash
g++ -I common/inc emul_test-beam_Sep23.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_test-beam_Sep23.exe
#g++ -I common/inc compress_decompress.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o compress_decompress.exe
g++ -I common/inc findEMax.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o findEMax.exe
#g++ -Icommon/inc econt_data_validation.cpp -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o econt_data_validation.exe
#g++ -I common/inc emul_econt.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_econt.exe
#g++ -I common/inc emul_bc.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_bc.exe
#g++ -I common/inc emul_stc4.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o emul_stc4.exe
#g++ -I common/inc read_pedestal.cpp -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o read_pedestal.exe
# g++ -I common/inc offline/src/RunDump0.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o run_dump_0.exe -Ioffline/inc
# g++ -I common/inc offline/src/RunDump1.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o run_dump_1.exe -Ioffline/inc -Iexpert/inc
# g++ -I common/inc offline/src/RunDump2.cpp  -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o run_dump_2.exe -Ioffline/inc -Iexpert/inc
