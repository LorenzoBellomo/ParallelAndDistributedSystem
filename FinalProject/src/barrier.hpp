#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

#ifndef BARRIER
#define BARRIER

using namespace std; 

class barrier {
private:

    mutex d_mutex;
    condition_variable d_condition;
    int counter, nw;

public:
    barrier(int nw_barr): nw(nw_barr), counter(0) {}

    void barrier_wait() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        counter++;
        if(counter == nw)
            this->d_condition.notify_all();
        else 
            this->d_condition.wait(lock, []{ return counter == nw; });
    }
}

#endif