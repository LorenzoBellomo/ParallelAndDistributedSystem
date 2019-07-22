
#ifndef SUPER_STEP
#define SUPER_STEP

#include <iostream>
#include <functional>

#include <safeQueue.hpp>
#include <BSP.hpp>

using namespace std; 

template <typename T>
class super_step {
private:
    safe_queue<T> queue_in;
    function<void(void)> ss_func;
    BSP<T> bsp;

public:
    super_step(function<void(void)> func, BSP<T> _bsp): bsp(_bsp), ss_func(func) {
        queue_in = safe_queue<T>();
    }

    void offer_task(T task) {
        queue_in.push(task);
    }

};

#endif