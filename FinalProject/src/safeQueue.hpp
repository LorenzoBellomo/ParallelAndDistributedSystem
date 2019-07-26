
#ifndef SAFE_QUEUE
#define SAFE_QUEUE

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>
#include <optional>
#include <vector>

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
            //d_queue.insert(d_queue.begin(), begin, end);
            for(auto p = begin; p < end; p++)
                d_queue.push_front(*p);
        }
        this->d_condition.notify_one();
    }

    std::optional<T> try_pop() {
        if(this->d_queue.empty())
            return {};
        T rc(std::move(this->d_queue.back()));
        this->d_queue.pop_back();
        return rc;
    }

    void get_iterators(
        typename std::deque<T>::reverse_iterator& begin,
        typename std::deque<T>::reverse_iterator& end)
    {
        begin = d_queue.rbegin();
        end = d_queue.rend();
    }
};

#endif