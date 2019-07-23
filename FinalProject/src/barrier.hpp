
#ifndef BARRIER
#define BARRIER

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

using namespace std; 

class barrier {
private:

    mutex d_mutex;
    condition_variable d_condition;
    int counter, nw;

public:
    barrier(): counter(0) {}

    void set_target(int num_w) {
        nw = num_w;
    }

    void barrier_wait() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        counter++;
        if(counter == nw)
            this->d_condition.notify_all();
        else 
            this->d_condition.wait(lock, [this]{ return counter == nw; });
    }
};

#endif