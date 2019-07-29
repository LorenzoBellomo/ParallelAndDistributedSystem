
#ifndef POSIX_BSP
#define POSIX_BSP

#include <iostream>
#include <vector>
#include <functional>

#include <logicBSP.hpp>
#include <barrier.hpp>
#include <queueMatrix.hpp>

template <typename T>
class posixBSP {

private:

    typedef std::shared_ptr<safe_queue<T>> ss_queue;
    typedef std::function<void(ss_queue, size_t, std::vector<ss_queue>)> ss_function;

    std::vector<logicBSP<T>*> logic;
    size_t nw, ss_count;
    std::vector<std::thread> thds;
    queue_matrix<T> matrix; // matrix[ss_num][nw]
    barrier barr;

    void worker(size_t worker_idx) {

        size_t current_ss = 0;
        bool end = false;
        logicBSP<T> *my_logic = logic[worker_idx];

        while(!end) {
            ss_queue my_queue = (current_ss < ss_count)?
                    matrix.get_queue(current_ss, worker_idx) : nullptr;
            std::vector<ss_queue> next_queues = (current_ss < ss_count - 1)? 
                    matrix.get_ss_queues(current_ss + 1) : std::vector<ss_queue>();
                    
            auto ss_funct = my_logic->switcher(current_ss);
            ss_funct(my_queue, worker_idx, next_queues);
            barr.barrier_wait();

            if(++current_ss == ss_count)
                end = true;
        }
        
    }
    
public:

    posixBSP(const std::vector<logicBSP<T>*>& _logic, size_t num_w, size_t num_ss): 
        logic(_logic), nw(num_w), ss_count(num_ss), matrix(num_ss, num_w), barr(num_w) {}
    

    void start_and_wait() {
        for(size_t i = 0; i < nw; i++)
            thds.push_back(std::thread(&posixBSP<T>::worker, this, i));

        for(auto& t : thds)
            t.join();
    }
};

#endif