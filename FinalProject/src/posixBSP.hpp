
#ifndef POSIX_BSP
#define POSIX_BSP

#include <iostream>
#include <vector>
#include <functional>

#include <logicBSP.hpp>
#include <barrier.hpp>
#include <queueMatrix.hpp>

using namespace std;

template <typename T>
class posixBSP {

private:

    typedef shared_ptr<safe_queue<T>> ss_queue;
    typedef function<void(ss_queue, int, vector<ss_queue>)> ss_function;

    logicBSP<T> *logic;
    int nw, ss_count;
    vector<thread> thds;
    vector<ss_function> ss_funct;
    queue_matrix<T> matrix; // matrix[ss_num][nw]
    barrier barr;

    void worker(int worker_idx) {

        int current_ss = 0;
        bool end = false;

        while(!end) {
            ss_queue my_queue = matrix.get_queue(current_ss, worker_idx);
            vector<ss_queue> next_queues = (current_ss == ss_count-1)? 
                    matrix.get_ss_queues(current_ss) : vector<ss_queue>();
                    
            ss_funct[current_ss](my_queue, worker_idx, next_queues);
            barr.barrier_wait();

            if(++current_ss == ss_count)
                end = true;
        }
        
    }
    
public:

    posixBSP(logicBSP<T> *_logic, int num_w): logic(_logic), nw(num_w), 
        ss_count(_logic->ss_count()), matrix(_logic->ss_count(), nw), barr(nw) 
    {
        for(int i = 0; i < ss_count; i++)
            ss_funct.push_back(logic->switcher(i));
    }

    void start_and_wait() {
        for(int i = 0; i < nw; i++)
            thds.push_back(thread(&posixBSP<T>::worker, this, i));

        for(auto& t : thds)
            t.join();
    }
};

#endif