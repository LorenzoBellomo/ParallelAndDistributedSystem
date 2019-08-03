
#ifndef UTIMER
#define UTIMER

/*
    filename: utimer.hpp
    author: Lorenzo Bellomo
    description: timer
*/

#include <iostream>
#include <chrono>

class utimer {
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
    std::string message; 
    using usecs = std::chrono::microseconds;
    using msecs = std::chrono::milliseconds;
  
public:

    utimer(const std::string m) : message(m) {
        start = std::chrono::system_clock::now();
    }

    ~utimer() {
        stop = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = stop - start;
        
        auto musec =
            std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        std::cout << message << " computed in " << musec << " usec " 
                << std::endl;

    }

    // static method, it takes a previous time and returns the long microseconds elapsed
    static long elapsed(std::chrono::system_clock::time_point before) {

        auto after = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = after - before;
        
        auto musec =
            std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

        return musec;
    }
};

#endif
