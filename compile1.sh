#!/usr/bin/env bash
g++ -Icommon/inc serenity_version.cpp -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o serenity_version.exe
