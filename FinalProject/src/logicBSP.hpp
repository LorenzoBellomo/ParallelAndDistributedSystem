
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
    //typedef void (logicBSP::*ss_function)(ss_queue my_queue, int worker_idx, vector<ss_queue> next);
    typedef function<void(ss_queue, int, vector<ss_queue>)> ss_function;
public: 
    virtual ss_function switcher(int ss_index) = 0;
    virtual int ss_count() = 0;

    bool check_end() {
        return true;
    }

    int cycle_to_ss() {
        return 0;
    }
};

#endif