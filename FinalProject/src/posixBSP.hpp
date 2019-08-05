
#ifndef POSIX_BSP
#define POSIX_BSP

/*
    filename: posixBSP.hpp
    author: Lorenzo Bellomo
    description: The BSP runtime class
*/

#include <iostream>
#include <vector>
#include <functional>
#include <chrono>

#include <logicBSP.hpp>
#include <barrier.hpp>
#include <queueMatrix.hpp>
#include <utimer.hpp>

#ifdef TSEQ
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#endif

template <typename T>
class posixBSP {

private:

    typedef std::shared_ptr<safe_queue<T>> ss_queue;
    typedef std::function<void(ss_queue, size_t, std::vector<ss_queue>)> ss_function;

    std::vector<logicBSP<T>*> logic; // vector of workers, passed by user
    // global_ss is only updated at the end of all the super steps
    size_t nw, ss_count, global_ss, global_end_next_ss;
    std::vector<std::thread> thds;
    queue_matrix<T> matrix; // matrix[ss_num][nw] of safe_queues (check queueMatrix.hpp)
    barrier barr, end_barrier;  // barrier is the one shared between workers
                                // end_barrier is used to make the global end check
    bool global_end;    // used to syncronize workers and tell them if the BSP is over or not
    std::function<bool(std::vector<logicBSP<T>*>)> termination_condition;

#ifdef TSEQ
    // collection of partial times
    std::map<std::string, long> time_map;
    std::mutex time_map_mutex;
#endif


    // force a given std::thread to be executed on one of the thread contexts available
    // in case of failure, just exit without sticking the thread
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

    // worker loop, it takes a worker id (in range [0, nw)) and a boolean which is 
    // true if the program has to try and stick one worker per core, and false otherwise
    void worker(size_t worker_idx, bool stick) {

        size_t current_ss = 0; // local view of current super step
        bool end = false; // end is true if the global condition is true
        logicBSP<T> *my_logic = logic[worker_idx]; // handler to the worker of index worker_idx

#ifdef TSEQ
        std::stringstream ss;
#endif

#ifndef PROFILE
        // for some reason thread sticking doesn't work with gprof
        if(stick)
            try_stick_current_thread(&thds[worker_idx], worker_idx);
#endif

        while(!end) {
#ifdef TSEQ
            std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
#endif
            ss_queue my_queue = matrix.get_queue(current_ss, worker_idx);
            std::vector<ss_queue> next_queues = (current_ss < ss_count - 1)? 
                matrix.get_ss_queues(current_ss + 1) : 
                matrix.get_ss_queues(global_end_next_ss); // if I'm at the end then I want the next queue
                                                            // from the super step the BSP loop restarts to
                    
            auto ss_funct = my_logic->switcher(current_ss);
            ss_funct(my_queue, worker_idx, next_queues); // super step function call

#ifdef TSEQ
            ss.str(""); // save time of super step (both communication and computation)
            ss.clear();
            ss << "super step " << current_ss << " worker " << worker_idx;
            {
                std::unique_lock<std::mutex> lock(time_map_mutex);
                time_map.insert({ss.str(), utimer::elapsed(start)});
            }
            start = std::chrono::system_clock::now();
#endif

            barr.barrier_wait(); // syncronization

#ifdef TSEQ
            ss.str("");
            ss.clear();
            ss << "barrier " << current_ss << " worker " << worker_idx;
            {
                std::unique_lock<std::mutex> lock(time_map_mutex);
                time_map.insert({ss.str(), utimer::elapsed(start)});
            }
            start = std::chrono::system_clock::now();
#endif

            if(++current_ss == ss_count) {
                // first end barrier wait is to signal the main thread that 
                // the end of the super steps was reached
                end_barrier.barrier_wait();
                // the second one is to ensure that the main thread could
                // sequentially compute the end condition
                end_barrier.barrier_wait();
                (global_end)? end = true : current_ss = global_ss;
#ifdef TSEQ
                ss.str("");
                ss.clear();
                ss << "end worker " << worker_idx;
                {
                    std::unique_lock<std::mutex> lock(time_map_mutex);
                    time_map.insert({ss.str(), utimer::elapsed(start)});
                }
                start = std::chrono::system_clock::now();
#endif
            }
        }
        
    }
    
public:

    // constructor (vector of workers, parallelism degree, number of super steps)
    posixBSP(const std::vector<logicBSP<T>*>& _logic, size_t num_w, size_t num_ss): 
        logic(_logic), nw(num_w), ss_count(num_ss), global_end_next_ss(0),
        matrix(num_ss, num_w), barr(num_w), end_barrier(nw+1), global_end(false)
    // end barrier waits nw+1 threads because the main too (the one that calls start_and_wait)
    // waits there, and computes at the end  the global continuation condition
    {
        termination_condition = [](std::vector<logicBSP<T>*> workers) {
            return true;
        };
    }


    void start_and_wait() {
        // check if the
        unsigned concurrentThreadsSupported = std::thread::hardware_concurrency();
        bool stick = ((concurrentThreadsSupported >= nw) || (concurrentThreadsSupported == 0));

        for(size_t i = 0; i < nw; i++) 
            thds.push_back(std::thread(&posixBSP<T>::worker, this, i, stick));

        while(!global_end) {
            // I wait for all super steps to end
            end_barrier.barrier_wait();
            // at this point I compute the global end condition
            if(termination_condition(logic))
                global_end = true;
            else 
                global_ss = global_end_next_ss;
            end_barrier.barrier_wait();
            // and I notify to all the workers now
        }

        for(auto& t : thds)
            t.join();
    }

    // to change the global end condition function
    void set_termination_function(std::function<bool(std::vector<logicBSP<T>*>)> term, size_t next_ss) {
        termination_condition = term;
        global_end_next_ss = next_ss;
    }

    // display the partial times computed
    void dump_tseq() {

#ifdef TSEQ
        std::map<std::string, long>::iterator begin, end;
        for(auto l : logic) 
            l->dump_tseq();
        
        for(auto e : time_map)
            std::cout << e.first << " -> " << e.second << " usec" << std::endl;
#else
        std::cout << "compile with \"make tseq\" in order to "
                        "obtain super step times" << std::endl;
#endif

    }
};

#endif