
#ifndef SUPER_STEP
#define SUPER_STEP

#include <iostream>
#include <functional>
#include <thread>

#include <safeQueue.hpp>
#include <barrier.hpp>
//#include <posixBSP.hpp>

using namespace std; 

template <typename T>
class super_step {
private:
    function<void(super_step<T> *, super_step<T> *)> ss_func;
    //posixBSP<T> *bsp;
    vector<thread> thds;
    vector<safe_queue<T>> in_queues;
    barrier barr;
    int nw, ss_lvl;

    void worker(function<void(super_step *current_ss, super_step *next_ss)>) {

        
    }

public:
    super_step(
        function<void(super_step<T> *, super_step<T> *)> func, 
        //posixBSP<T> *_bsp, 
        int _nw, 
        int _ss_lvl): 
            ss_func(func), nw(_nw), ss_lvl(_ss_lvl) //, bsp(_bsp)
    {
        barr = barrier();
        in_queues = vector<safe_queue<T>>(nw);
    }

    void start_ss() {
        for(int i = 0; i < nw; i++)
            thds.push_back(thread(ss_func));
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
        typename vector<T>::iterator end,
        int worker_index) 
    {
        in_queues[worker_index].push_multiple(begin, end);
    }

    void join_barrier() {
        barr.barrier_wait();
    }

    void get_my_queue_data(
        const typename deque<T>::reverse_iterator &begin,
        const typename deque<T>::reverse_iterator &end,
        int worker_idx) 
    {
        in_queues[worker_idx].get_iterators(begin, end);
    }

};

#endif