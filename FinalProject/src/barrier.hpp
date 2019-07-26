
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
    int counter, nw, iteration;

public:
    barrier(): counter(0), iteration(0) {}

    void set_target(int num_w) {
        nw = num_w;
        counter = nw;
    }

    void barrier_wait() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        auto prev_iter = iteration;
        if (!--counter) {
            iteration++;
            counter = nw;
            d_condition.notify_all();
        } else {
            d_condition.wait(lock, [this, prev_iter] { return prev_iter != iteration; });
        }
    }
};

#endif