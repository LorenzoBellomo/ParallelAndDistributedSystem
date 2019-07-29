
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <optional> 

#include <ff/ff.hpp>
#include <ff/all2all.hpp>

#include <utimer.hpp>
#include <logicBSP.hpp>
#include <posixBSP.hpp>

struct sorter_worker: logicBSP<long> {

    sorter_worker(
        size_t num_w,
        std::vector<long>::iterator begin, 
        std::vector<long>::iterator end): nw(num_w), elem(begin, end) {}

    void ss1(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {
        std::sort(elem.begin(), elem.end());
        auto range = elem.size() / (nw + 1);
        auto extra = elem.size() % (nw + 1);
        auto prev = elem.begin();
        for(size_t i = 0; i < extra; i++) {
            samples.push_back(*prev);
            prev += (range + 1);
        }
        for(size_t i = 0; i < nw - extra; i++) {
            samples.push_back(*prev);
            prev += range;
        }
        samples.push_back(*(elem.end() - 1));

        for(size_t i = 0; i <  next_queues.size(); i++) {
            if(i != worker_idx) {
                auto q = next_queues[i];
                q->push_multiple(samples.begin(), samples.end());
            }
        }
    }

    void ss2(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {
        std::optional<long> next;
        while((next = my_queue->try_pop()).has_value())
            samples.push_back(next.value());

        std::sort(samples.begin(), samples.end());
    }

    void ss3(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {

    }

    logicBSP::ss_function switcher(size_t idx) {
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

    size_t nw;
    std::vector<long> elem;
    std::vector<long> samples;

};

int main(int argc, char *argv[]) {

    if(argc != 3 && argc != 4) {
        std::cout << "Usage is ./sorter n nw (opt:seed)" << std::endl
            << "Where:" << std::endl 
            << "- n is the number of elements" << std::endl
            << "- nw is the number of workers" << std::endl
            << "- seed is the RNG seed (and it is optional)" << std::endl;
        return -1;
    }
    
    long n = atoi(argv[1]);
    size_t nw = atoi(argv[2]);
    int seed = (argc == 4)? atoi(argv[3]) : time(NULL);

    // Generate random unique vector (size n) with elements in range [0, 5*n)
    std::vector<long> in_tasks;
    std::vector<long> std_sort;
    std::vector<long> out_tasks(n);
    for(long i = 0; i < n*5; i++)
        in_tasks.push_back(i);
    std::shuffle(in_tasks.begin(), in_tasks.end(), std::default_random_engine(seed));
    in_tasks.erase(in_tasks.begin() + n, in_tasks.end());
    std_sort = in_tasks;

    {
        utimer timer("Posix version");

        std::vector<logicBSP<long>*> workers;
        auto range = in_tasks.size() / nw;
        auto extra = in_tasks.size() % nw;
        auto prev = in_tasks.begin();
        for(size_t i = 0; i < extra; i++) {
            workers.push_back(new sorter_worker(nw, prev, prev + range + 1));
            prev += (range + 1);
        }
        for(size_t i = 0; i < nw - extra; i++) {
            workers.push_back(new sorter_worker(nw, prev, prev + range));
            prev += range;
        }

        posixBSP<long> bsp(workers, nw, 3);
        bsp.start_and_wait();
    }

    {
        utimer timer("FastFlow version");
        
    }

    {
        utimer timer("std::sort");
        std::sort(std_sort.begin(), std_sort.end());
    }

    std::cout << "input" << std::endl;
    for(auto i : in_tasks)
        std::cout << i << " ";
    std::cout << std::endl;

    /*std::cout << "output" << endl;
    for(auto i : out_tasks)
        std::cout << i << " ";
    std::cout << endl;*/
    
    std::cout << "std::sort" << std::endl;
    for(auto i : std_sort)
        std::cout << i << " ";
    std::cout << std::endl;

    return 0;
}