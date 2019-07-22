
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
    vector<super_step<T>> superSteps;
    int nw;
    typename vector<T>::iterator begin, end;
    
public:

    posixBSP(
        vector<function<void(void)>> ss, 
        int num_workers, 
        typename vector<T>::iterator _begin,
        typename vector<T>::iterator _end): 
            nw(num_workers), begin(_begin), end(_end) {

    }

    void send_to(
        typename vector<T>::iterator _begin, 
        typename vector<T>::iterator _end,
        int ss_lvl_index,
        int worker_index)   {
         
    }
};

#endif