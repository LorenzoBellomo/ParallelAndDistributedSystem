CXX = g++
CXXFLAGS = -std=c++17 -Wall
INCLUDES = -I . 
OPTFLAGS = -O3
LDFLAGS = -pthread

TARGETS = sorter

INCLUDE_FILES = superStep.hpp \
		BSP.hpp \
		posixBSP.hpp \
		safeQueue.hpp \
		superStep.hpp \
		utimer.hpp 
	
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