
/*
    filename: sorter.hpp
    author: Lorenzo Bellomo
    description: The file with the main function
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <optional> 

#include <utimer.hpp>
#include <logicBSP.hpp>
#include <posixBSP.hpp>
#include <sorterLogic.hpp>


int main(int argc, char *argv[]) {

    if(argc != 3 && argc != 4) {
        std::cerr << "Usage is ./sorter n nw (opt:seed)" << std::endl
            << "Where:" << std::endl 
            << "- n is the number of elements" << std::endl
            << "- nw is the number of workers" << std::endl
            << "- seed is the RNG seed (and it is optional)" << std::endl;
        return -1;
    }
    
    size_t n = atoi(argv[1]);
    size_t nw = atoi(argv[2]);
    int seed = (argc == 4)? atoi(argv[3]) : time(NULL);

    if(nw <= 0) {
        std::cerr << "need at least one worker" << std::endl;
        return -1;
    }
    if(n <= 0 || n < nw) {
        std::cerr << "invalid vector size" << std::endl;
        return -1;
    }

    // Generate random unique vector (size n) with elements in range [0, 5*n)
    // the time needed to generate the input vector is not considered for measures 
    std::vector<long> in_tasks;
    std::vector<long> out_tasks(n);

    std::cout << "Start generating input vector" << std::endl;

    {
        utimer timer("Generating input vector");

        for(size_t i = 0; i < n; i++)
            in_tasks.push_back(i);
        std::shuffle(in_tasks.begin(), in_tasks.end(), std::default_random_engine(seed));
    }

    std::vector<logicBSP<long>*> workers;

    {
        utimer timer("Posix version");

        // split input data between workers
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

        // generate BSP object and start the bsp
        posixBSP<long> bsp(workers, nw, 3);
        bsp.start_and_wait();
#ifdef TSEQ
        // just display the partial times
        bsp.dump_tseq();
#endif
    }

#ifndef BENCHMARK
    {
        utimer timer("std::sort");
        std::sort(in_tasks.begin(), in_tasks.end());
    }
#endif

#ifdef DEBUG
    std::vector<long> check_vec;
    for(auto l : workers) {
        sorter_worker *ptr = dynamic_cast<sorter_worker*>(l);
        ptr->push_to_output_vector(check_vec);
    }
    (check_vec == in_tasks)? 
        std::cout << "vectors are equal" << std::endl:
        std::cout << "vectors differ" << std::endl;

#endif

    return 0;
}