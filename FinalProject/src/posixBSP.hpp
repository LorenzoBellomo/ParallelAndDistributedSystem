
#ifndef POSIX_BSP
#define POSIX_BSP

#include <iostream>
#include <vector>
#include <functional>

#include <BSP.hpp>
#include <superStep.hpp>

using namespace std;

template <typename T>
class posixBSP : public BSP<T> {

private:
    vector<super_step<T>> super_steps;
    int nw, current_ss;
    typename vector<T>::iterator begin, end;
    
public:

    posixBSP(
        vector<function<void(void)>*> ss, 
        int num_workers, 
        typename vector<T>::iterator _begin,
        typename vector<T>::iterator _end): 
            nw(num_workers), current_ss(0), begin(_begin), end(_end) 
    {
        super_steps = vector<super_step<T>>();
        for(int i = 0; i < ss; i++) 
            super_steps.push_back(ss[i], this, nw, i);

    }

    void send_coll_to(
        typename vector<T>::iterator _begin, 
        typename vector<T>::iterator _end,
        int worker_index)   
    {
        super_steps[current_ss+1].offer_tasks(_begin, _end, worker_index);
         
    }

    void send_to(T elem, int worker_index) {
        super_steps[current_ss+1].offer_task(elem, worker_index);
    }

    void sync(int from) {
        super_steps[current_ss].join_barrier();
    }
};

#endif