
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <deque>

#include <utimer.hpp>
#include <logicBSP.hpp>
#include <posixBSP.hpp>

using namespace std;

struct sorter_worker: logicBSP<long> {

    sorter_worker(
        int num_w,
        vector<long>::iterator begin, 
        vector<long>::iterator end): nw(num_w), elem(begin, end) {}

    void ss1(
        logicBSP::ss_queue my_queue, 
        int worker_idx, 
        vector<logicBSP::ss_queue> next_queue)
    {
        sort(elem.begin(), elem.end());
        vector<long> samples;
        auto range = elem.size() / (nw + 1);
        auto extra = elem.size() % (nw + 1);
        auto prev = elem.begin();
        for(int i = 0; i < extra; i++) {
            samples.push_back(*prev);
            prev += (range + 1);
        }
        for(int i = 0; i < nw - extra; i++) {
            samples.push_back(*prev);
            prev += range;
        }
        samples.push_back(*(elem.end() - 1));

        for(auto q : next_queue)
            q->push_multiple(samples.begin(), samples.end());
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
                    auto f = bind(&sorter_worker::ss1, this, _1, _2, _3);
                    return f;
                }
            case 1: 
                {
                    auto f = bind(&sorter_worker::ss2, this, _1, _2, _3);
                    return f;
                }
            case 2: 
                {
                    auto f = bind(&sorter_worker::ss3, this, _1, _2, _3);
                    return f;
                }
        }
        return nullptr;
    }

    int nw;
    vector<long> elem;

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
    vector<long> comparison_task;
    vector<long> out_tasks(n);
    for(long i = 0; i < n*5; i++)
        in_tasks.push_back(i);
    shuffle(in_tasks.begin(), in_tasks.end(), default_random_engine(seed));
    in_tasks.erase(in_tasks.begin() + n, in_tasks.end());
    comparison_task = in_tasks;

    {
        utimer timer("parallel version");

        vector<logicBSP<long>*> workers;
        auto range = in_tasks.size() / nw;
        auto extra = in_tasks.size() % nw;
        auto prev = in_tasks.begin();
        for(int i = 0; i < extra; i++) {
            workers.push_back(new sorter_worker(nw, prev, prev + range + 1));
            prev += (range + 1);
        }
        for(int i = 0; i < nw - extra; i++) {
            workers.push_back(new sorter_worker(nw, prev, prev + range));
            prev += range;
        }

        posixBSP<long> bsp(workers, nw, 3);
        bsp.start_and_wait();
    }

    {
        utimer timer("std::sort");
        sort(comparison_task.begin(), comparison_task.end());
    }

    cout << "input" << endl;
    for(auto i : in_tasks)
        cout << i << " ";
    cout << endl;

    /*cout << "output" << endl;
    for(auto i : out_tasks)
        cout << i << " ";
    cout << endl;*/
    
    cout << "comparison" << endl;
    for(auto i : comparison_task)
        cout << i << " ";
    cout << endl;

    return 0;
}