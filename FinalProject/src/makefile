CXX = g++
CXXFLAGS = -std=c++17 -Wall
DEBUG = -pg
INCLUDES = -I . -I /usr/local/fastflow
OPTFLAGS = -O3
LDFLAGS = -pthread

TARGETS = sorter

INCLUDE_FILES = logicBSP.hpp \
		posixBSP.hpp \
		safeQueue.hpp \
		utimer.hpp \
		barrier.hpp \
		queueMatrix.hpp \
		sorterLogic.hpp
	
.PHONY: all clean cleanall profile

%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) 

clean: 
	rm -f $(TARGETS)

cleanall:
	make clean

all:
	make cleanall
	make $(TARGETS)

profile: 
	make cleanall
	gcc -shared -fPIC benchmark/gprof-helper.c -o benchmark/gprof-helper.so -lpthread -ldl 
	$(CXX) $(CXXFLAGS) $(DEBUG) -DPROFILE $(INCLUDES) -Os -o sorter sorter.cpp $(LDFLAGS) 