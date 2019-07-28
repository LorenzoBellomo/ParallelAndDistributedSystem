CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wfatal-errors 
INCLUDES = -I . #-I /usr/local/lib/fastflow
OPTFLAGS = -O3
LDFLAGS = -pthread

TARGETS = sorter

INCLUDE_FILES = logicBSP.hpp \
		posixBSP.hpp \
		safeQueue.hpp \
		utimer.hpp \
		barrier.hpp \
		queueMatrix.hpp
	
.PHONY: all clean cleanall debug

%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) 

clean: 
	rm -f $(TARGETS)

cleanall:
	make clean

all:
	make cleanall
	make $(TARGETS)

debug:
	rm -f prova
	make prova