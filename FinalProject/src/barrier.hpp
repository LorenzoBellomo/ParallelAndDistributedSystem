
#ifndef BARRIER
#define BARRIER

/*
    filename: barrier.hpp
    author: Lorenzo Bellomo
    description: implementation of a syncronization barrier
*/

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

class barrier {
private:

    std::mutex d_mutex;
    std::condition_variable d_condition;
    int counter, nw, iteration;
    // iteration is the variable that keeps the barrier valid and reusable.
    // This way eventual "early birds"(workers that pass from a barrier to the next
    // before all the others wake up) are not causing deadlocks.

public:
    barrier(int num_w): counter(num_w), nw(num_w), iteration(0) {}

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