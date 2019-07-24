
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

    typedef vector<shared_ptr<super_step<T>>> ss_collection;

    ss_collection super_steps;
    int nw, current_ss;
    vector<thread> thds;

    void worker(int worker_idx) {
        for(auto ss : super_steps) {
            ss->execute_ss(worker_idx);
            ss->join_barrier();
        }
    }
    
public:

    posixBSP(
        ss_collection ss, 
        int num_workers, 
        function<void()> init_logic): 
            nw(num_workers), current_ss(0)
    {
        super_steps = ss;
        init_logic();
        for(size_t i = 0; i < ss.size() - 1; i++)
            super_steps[i]->initialize(super_steps[i+1], nw, i);
        super_steps[ss.size()-1]->initialize(nullptr, nw, ss.size()-1);
    }

    void start_and_wait() {
        for(int i = 0; i < nw; i++)
            thds.push_back(thread(worker, i));

        for(auto t : thds)
            t.join();
        cout << "Joined threads" << endl;

        for(auto a : super_steps)
            a.reset();
    }
};

#endif