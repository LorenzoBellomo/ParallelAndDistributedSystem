CXX = g++
CXXFLAGS = -std=c++17 -I . -O3
LDFLAGS = -L/usr/X11R6/lib -lm -lX11 -pthread

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) 