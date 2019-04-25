CXX = g++
CXXFLAGS = -std=c++17 -I . -O3
LDFLAGS = -lX11 -pthread

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) 