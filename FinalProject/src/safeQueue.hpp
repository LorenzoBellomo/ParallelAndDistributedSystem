
#ifndef SAFE_QUEUE
#define SAFE_QUEUE

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>
#include <optional>
#include <vector>

/*
    filename: safeQueue.hpp
    author: Lorenzo Bellomo
    description: A queue that is syncronized in writes but not in reads
*/

template <typename T>
class safe_queue {
private:
    std::mutex              d_mutex;
    std::condition_variable d_condition;
    std::deque<T>           d_queue;
public:

    safe_queue() {}
  
    void push(T const& value) {
        {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            d_queue.push_front(value);
        }
        this->d_condition.notify_one();
    }

    void push_multiple(
        typename std::vector<T>::iterator begin,
        typename std::vector<T>::iterator end) 
    {
        {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            for(auto p = begin; p < end; p++)
                d_queue.push_front(*p);
        }
        this->d_condition.notify_one();
    }

    // returns nullopt if the queue is empty
    std::optional<T> try_pop() {
        if(this->d_queue.empty())
            return {};
        T rc(std::move(this->d_queue.back()));
        this->d_queue.pop_back();
        return rc;
    }

};

#endif