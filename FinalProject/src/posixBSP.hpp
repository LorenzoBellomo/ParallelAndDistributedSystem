
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
    size_t nw, ss_count, global_ss;
    std::vector<std::thread> thds;
    queue_matrix<T> matrix; // matrix[ss_num][nw]
    barrier barr, end_barrier;
    bool global_end;

    // force a given std::thread to be executed on one of the thread contexts available
    void try_stick_current_thread(std::thread *tid, int coreno) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreno, &cpuset);  // without % those in excess are free to move
    if(pthread_setaffinity_np(tid->native_handle(),sizeof(cpu_set_t), &cpuset) != 0) {
        auto reason = errno; 
        std::cerr << "Error while sticking current thread to core: "
            << (reason == EINVAL ? "EINVAL" : (reason == EFAULT ? "EFAULT" : 
            (reason == ESRCH ? "ESRCH" : "UNKNOWN"))) << std::endl;
        }
    }

    void worker(size_t worker_idx) {

        size_t current_ss = 0;
        bool end = false;
        logicBSP<T> *my_logic = logic[worker_idx];

        try_stick_current_thread(&thds[worker_idx], worker_idx);

        while(!end) {
            ss_queue my_queue = matrix.get_queue(current_ss, worker_idx);
            std::vector<ss_queue> next_queues = (current_ss < ss_count - 1)? 
                matrix.get_ss_queues(current_ss + 1) : 
                matrix.get_ss_queues(global_end_next_ss());
                    
            auto ss_funct = my_logic->switcher(current_ss);
            ss_funct(my_queue, worker_idx, next_queues);
            barr.barrier_wait();

            if(++current_ss == ss_count) {
                // first barrier is to signal the main thread that 
                // the end of the super steps was reached
                end_barrier.barrier_wait();
                // the second one is to ensure that the main thread could
                // sequentially compute the end condition
                end_barrier.barrier_wait();
                (global_end)? end = true : current_ss = global_ss;
            }
        }
        
    }
    
public:

    posixBSP(const std::vector<logicBSP<T>*>& _logic, size_t num_w, size_t num_ss): 
        logic(_logic), nw(num_w), ss_count(num_ss), matrix(num_ss, num_w), 
        barr(num_w), end_barrier(nw+1), global_end(false) {}
    

    void start_and_wait() {
        for(size_t i = 0; i < nw; i++)
            thds.push_back(std::thread(&posixBSP<T>::worker, this, i));

        while(!global_end) {
            end_barrier.barrier_wait();
            if(global_end_condition(logic))
                global_end = true;
            else 
                global_ss = global_end_next_ss();
            end_barrier.barrier_wait();

        }

        for(auto& t : thds)
            t.join();
    }

    bool global_end_condition(std::vector<logicBSP<T>*> logic) {
        return true;
    }

    int global_end_next_ss() {
        return 0;
    }
};

#endif