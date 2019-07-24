
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
    vector<unique_ptr<safe_queue<T>>> in_queues;
    shared_ptr<super_step<T>> next_ss;
    barrier barr;
    int nw, ss_lvl;

public:

    super_step() {}

    void initialize(shared_ptr<super_step<T>> p, int num_w, int ss_level) {
        next_ss = p;
        nw = num_w;
        ss_lvl = ss_level;
        barr.set_target(nw);
        for(int i = 0; i < nw; i++)
            in_queues.push_back(make_unique<safe_queue<T>>());
    }

    void ss_func(
        typename deque<T>::reverse_iterator begin, 
        typename deque<T>::reverse_iterator end, 
        int idx, 
        shared_ptr<super_step<T>> next_ss)
    {
        cout << "default function, override ss_func in order to \
                    implement your business logic code" << endl;
    }

    void execute_ss(int worker_idx) {
        typename deque<T>::reverse_iterator begin, end;
        in_queues[worker_idx].get_iterators(begin, end);
        ss_func(begin, end, worker_idx, next_ss);
        join_barrier();
        cout << worker_idx << " ended super step" << endl;
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

    int get_nw() {
        return nw;
    }

};

#endif