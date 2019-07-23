
#ifndef SUPER_STEP
#define SUPER_STEP

#include <iostream>
#include <functional>
#include <thread>
#include <deque>

#include <safeQueue.hpp>
#include <barrier.hpp>

using namespace std; 

template <typename T>
class super_step {
private:
    vector<thread> thds;
    vector<safe_queue<T>> in_queues;
    super_step<T> *next_ss;
    barrier barr;
    int nw, ss_lvl;

    void worker(int worker_idx) {
        typename deque<T>::reverse_iterator begin, end;
        in_queues[worker_idx].get_iterators(begin, end);
        ss_func(begin, end, worker_idx, next_ss);
        join_barrier();
        cout << worker_idx << "out of barrier" << endl;
    }

public:
    super_step() {}

    void initialize(super_step<T> *p, int num_w, int ss_level) {
        next_ss = p;
        nw = num_w;
        ss_lvl = ss_level;
        barr.set_target(nw);
        in_queues.resize(nw);
    }

    void ss_func(
        typename deque<T>::reverse_iterator begin, 
        typename deque<T>::reverse_iterator end, 
        int idx, 
        super_step<T> *next_ss)
    {
        cout << "default function" << endl;
    }

    void start_ss() {
        for(int i = 0; i < nw; i++) 
            thds.push_back(thread(worker, i));
        cout << "started threads" << endl;
    }

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

};

#endif