CXX = g++
CXXFLAGS = -std=c++17 -Wall
DEBUG = -pg # not used when calling make all, but only make profile 
INCLUDES = -I .
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
	
.PHONY: all clean cleanall profile tseq

%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) 

clean: 
	rm -f $(TARGETS)

cleanall: clean
	rm -f benchmark/data/*
	rm -f benchmark/gprof-helper.so

all: clean
	make $(TARGETS)

profile: clean
	gcc -shared -fPIC benchmark/gprof-helper.c -o benchmark/gprof-helper.so -lpthread -ldl 
	$(CXX) $(CXXFLAGS) $(DEBUG) -DPROFILE $(INCLUDES) -Os -o sorter sorter.cpp $(LDFLAGS) 

tseq: clean
	$(CXX) $(CXXFLAGS) -DTSEQ $(INCLUDES) $(OPTFLAGS) -o sorter sorter.cpp $(LDFLAGS) 