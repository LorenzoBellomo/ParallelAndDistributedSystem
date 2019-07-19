#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>
#include <optional>

#ifndef SAFE_QUEUE
#define SAFE_QUEUE

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
  
  T pop() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
    T rc(std::move(this->d_queue.back()));
    this->d_queue.pop_back();
    return rc;
  }

  bool is_empty() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    return(d_queue.empty());
  }

  std::pair<bool, std::optional<T>> try_pop() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    if(this->d_queue.empty())
      return std::pair<bool, std::optional<T>>(false, {});
    T rc(std::move(this->d_queue.back()));
    this->d_queue.pop_back();
    return std::pair<bool, std::optional<T>>(true, rc);
  }
};

#endif