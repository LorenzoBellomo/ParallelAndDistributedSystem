
#ifndef SUPER_STEP
#define SUPER_STEP

#include <iostream>
#include <functional>
#include <thread>

#include <safeQueue.hpp>
#include <BSP.hpp>
#include <barrier.hpp>

using namespace std; 

template <typename T>
class super_step {
private:
    function<void(void)> ss_func;
    BSP<T> *bsp;
    vector<thread> thds;
    vector<safe_queue<T>> in_queues;
    barrier barr;
    int nw, ss_lvl;

public:
    super_step(function<void(void)> func, BSP<T> *_bsp, int _nw, int _ss_lvl): 
        bsp(_bsp), ss_func(func), nw(_nw), ss_lvl(_ss_lvl)
    {
        barr = barrier();
        in_queues = vector<safe_queue<T>>(nw);
    }

    void start_ss() {
        for(int i = 0; i < nw; i++)
            thds.push_back(func);
    }

    /*void end_ss() {
        for(auto t : thds)
            t.join();
    }*/

    void offer_task(T task, int worker_index) {
        in_queues[worker_index].push(task);
    }

    void offer_tasks(
        typename vector<T>::iterator begin, 
        typename vector<T>::iterator end
        int worker_index) 
    {
        in_queues[worker_index].push_multiple(begin, end);
    }

    void join_barrier() {
        barr.barrier_wait();
    }

};

#endif