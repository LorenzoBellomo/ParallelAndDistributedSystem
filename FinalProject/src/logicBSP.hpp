
#ifndef LOGIC_BSP
#define LOGIC_BSP

#include <iostream>
#include <functional>

#include <safeQueue.hpp>

using namespace std;

template <typename T>
class logicBSP {
private:
protected: 
    typedef shared_ptr<safe_queue<T>> ss_queue;
    typedef function<void(ss_queue, int, ss_queue)> ss_function;
    typedef vector<ss_function> function_collection;
public: 
    virtual void caller(int ss_index, ss_queue prev, int worker_idx, ss_queue next) = 0;
    virtual int ss_count() = 0;
};

#endif