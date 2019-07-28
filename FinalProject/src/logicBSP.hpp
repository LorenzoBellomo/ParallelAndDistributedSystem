
#ifndef LOGIC_BSP
#define LOGIC_BSP

#include <iostream>
#include <functional>

#include <safeQueue.hpp>

using namespace std;

template <typename T>
class logicBSP {

protected: 
    typedef shared_ptr<safe_queue<T>> ss_queue;
    typedef function<void(ss_queue, int, vector<ss_queue>)> ss_function;
    
public: 
    virtual ss_function switcher(int ss_index) = 0;

};

#endif