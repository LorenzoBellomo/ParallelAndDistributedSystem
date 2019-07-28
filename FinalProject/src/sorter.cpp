
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <deque>

#include <utimer.hpp>
#include <posixBSP.hpp>
#include <logicBSP.hpp>

using namespace std;

struct sorter_logic: logicBSP<long> {

    sorter_logic(vector<long>::iterator begin, vector<long>::iterator end, int num_w): nw(num_w) {
        auto range = (end - begin) / nw;
        auto extra = (end - begin) % nw;
        int prev = 0;
        for(int i = 0; i < extra; ++i) {
            input_iterators.push_back(begin + prev);
            prev += (range + 1);
        }
        for(int i = 0; i < nw - extra; ++i) {
            input_iterators.push_back(begin + prev);
            prev += range;
        }
        input_iterators.push_back(end);
    }

    void ss1(
        logicBSP::ss_queue my_queue, 
        int worker_idx, 
        vector<logicBSP::ss_queue> next_queue)
    {
        for(auto p = input_iterators[worker_idx]; p < input_iterators[worker_idx+1]; p++)
    }

    void ss2(
        logicBSP::ss_queue my_queue, 
        int worker_idx, 
        vector<logicBSP::ss_queue> next_queue)
    {
        
    }

    void ss3(
        logicBSP::ss_queue my_queue, 
        int worker_idx, 
        vector<logicBSP::ss_queue> next_queue)
    {
        
    }

    logicBSP::ss_function switcher(int idx) {
        switch(idx) {
            using namespace std::placeholders;
            case 0: 
                {
                    auto f = bind(&sorter_logic::ss1, this, _1, _2, _3);
                    return f;
                }
            case 1: 
                {
                    auto f = bind(&sorter_logic::ss2, this, _1, _2, _3);
                    return f;
                }
            case 2: 
                {
                    auto f = bind(&sorter_logic::ss3, this, _1, _2, _3);
                    return f;
                }
        }
        return nullptr;
    }

    int ss_count() {
        return 3;
    }

    int nw;
    vector<vector<long>::iterator> input_iterators;

};

int main(int argc, char *argv[]) {

    if(argc != 3 && argc != 4) {
        cout << "Usage is ./sorter n nw (opt:seed)" << endl
            << "Where:" << endl 
            << "- n is the number of elements" << endl
            << "- nw is the number of workers" << endl
            << "- seed is the RNG seed (and it is optional)" << endl;
        return -1;
    }
    
    long n = atoi(argv[1]);
    int nw = atoi(argv[2]);
    int seed = (argc == 4)? atoi(argv[3]) : time(NULL);

    // Generate random unique vector (size n) with elements in range [0, 5*n)
    vector<long> in_tasks;
    vector<long> out_tasks(n);
    for(long i = 0; i < n*5; i++)
        in_tasks.push_back(i);
    shuffle(in_tasks.begin(), in_tasks.end(), default_random_engine(seed));
    in_tasks.erase(in_tasks.begin() + n, in_tasks.end());

    {
        utimer timer("parallel version");
        
        sorter_logic logic(in_tasks.begin(), in_tasks.end(), nw);
        posixBSP bsp(&logic, nw);
        bsp.start_and_wait();
    }

    {
        utimer timer("std::sort");
        sort(in_tasks.begin(), in_tasks.end());
    }

    return 0;
}