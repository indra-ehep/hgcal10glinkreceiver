g++ -Wall -std=c++11 -Icommon/inc -Ionline/inc -Ijunk/inc online/src/ProcessorDummy.cpp -o bin/ProcessorDummy.exe
g++ -Wall -std=c++11 -Icommon/inc -Ionline/inc -Ijunk/inc online/src/ProcessorFastControl.cpp -o bin/ProcessorFastControl.exe
g++ -Wall -std=c++11 -Icommon/inc -Ionline/inc -Ijunk/inc online/src/ProcessorDaqLink2.cpp -o bin/ProcessorDaqLink2.exe
g++ -Wall -std=c++11 -Icommon/inc -Ionline/inc -Ijunk/inc online/src/RunControlSystem.cpp -o bin/RunControlSystem.exe
g++ -Wall -std=c++11 -Icommon/inc -Ioffline/inc -Ijunk/inc offline/src/BasicRunFileCheck.cpp -o bin/BasicRunFileCheck.exe
