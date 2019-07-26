
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

    logicBSP<T> *logic;
    int nw, ss_count, current_ss;
    vector<thread> thds;
    queue_matrix<T> matrix; // matrix[ss_num][nw]
    barrier barr;

    void worker(int worker_idx) {
        
    }
    
public:

    posixBSP(logicBSP<T> *_logic, int num_w): logic(_logic), nw(num_w), 
        ss_count(_logic->ss_count()), current_ss(0), 
        matrix(_logic->ss_count(), nw) 
    {
        auto q = matrix.get_queue(0, 0);
        q->push(1);
        cout << q->try_pop().value() << endl;;
    }

    void start_and_wait() {

    }
};

#endif