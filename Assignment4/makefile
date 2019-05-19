CXX = g++
CXXFLAGS = -std=c++17 -I . -I /usr/local/lib/fastflow -Wfatal-errors #-O3
LDFLAGS = -pthread

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) 