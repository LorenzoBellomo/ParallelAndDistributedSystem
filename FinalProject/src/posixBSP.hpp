
#ifndef POSIX_BSP
#define POSIX_BSP

#include <iostream>
#include <vector>
#include <functional>

#include <superStep.hpp>

using namespace std;

template <typename T>
class posixBSP {

private:
    vector<super_step<T>> super_steps;
    int nw, current_ss;
    typename vector<T>::iterator begin, end;
    
public:

    posixBSP(
        vector<function<void(void)>> ss, 
        int num_workers, 
        typename vector<T>::iterator _begin,
        typename vector<T>::iterator _end): 
            nw(num_workers), current_ss(0), begin(_begin), end(_end) 
    {
        super_steps = vector<super_step<T>>();
        for(int i = 0; i < ss.size(); i++) 
            super_steps.push_back(ss[i], this, nw, i);

    }

    void start_and_wait() {

    }

    super_step<T> *get_next_ss(int ss_idx) {
        return (& super_steps[ss_idx]);
    }
};

#endif