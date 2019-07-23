
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
    
public:

    posixBSP(
        vector<super_step<T>> &ss, 
        int num_workers, 
        function<void(super_step<T> *)> init_logic): 
            super_steps(ss), nw(num_workers), current_ss(0)
    {
        init_logic(&super_steps[0]);
        for(size_t i = 0; i < ss.size() - 1; i++)
            super_steps[i].initialize(&super_steps[i+1], nw, i);
        super_steps[ss.size()-1].initialize(nullptr, nw, ss.size()-1);
    }

    void start_and_wait() {
        super_steps[0].join_barrier();
        cout << "Ended ss 1" << endl;

    }
};

#endif