CXX = g++
CXXFLAGS = -std=c++17 -I . -O3
LDFLAGS = -pthread

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) 