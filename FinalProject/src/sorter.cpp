
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
#include <sorterLogic.hpp>


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
    
    return 0;
}