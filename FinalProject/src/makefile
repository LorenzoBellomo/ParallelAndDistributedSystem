CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wfatal-errors
INCLUDES = -I . 
OPTFLAGS = -O3
LDFLAGS = -pthread

TARGETS = sorter

INCLUDE_FILES = superStep.hpp \
		posixBSP.hpp \
		safeQueue.hpp \
		utimer.hpp \
		barrier.hpp
	
.PHONY: all clean cleanall debug

%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) 

all:
	make $(TARGETS)

clean: 
	rm -f $(TARGETS)

cleanall:
	make clean

debug:
	rm -f prova
	make prova