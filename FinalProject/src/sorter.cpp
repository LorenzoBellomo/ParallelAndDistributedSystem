
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <deque>

#include <utimer.hpp>
#include <posixBSP.hpp>
#include <superStep.hpp>

using namespace std;

struct business_logic: logicBSP<long> {

    business_logic() {}

    void ss1(
        logicBSP::ss_queue my_queue, 
        int worker_idx, 
        vector<logicBSP::ss_queue> next_queue)
    {
        
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
            case 0: 
                return ss1;
            case 1: 
                return ss2;
            case 2: 
                return ss3;
        }
        return nullptr;
    }

    int ss_count() {
        return 3;
    }

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
    }

    {
        utimer timer("std::sort");
        sort(input.begin(), input.end());
    }

    return 0;
}