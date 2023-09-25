#!/usr/bin/env bash
g++ -Icommon/inc econt_data_validation.cpp -l yaml-cpp `root-config --libs --cflags` -o econt_data_validation.exe -O3
