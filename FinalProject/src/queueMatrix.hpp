
#ifndef QUEUE_MATRIX
#define QUEUE_MATRIX

#include <iostream>
#include <vector>

#include <safeQueue.hpp>

using namespace std;

template <typename T>
class queue_matrix {
private:
    typedef shared_ptr<safe_queue<T>> ss_queue;

    int nw, ss_num;
    vector<vector<ss_queue>> matrix; // matrix[#super_steps][#workers] 
public:
    queue_matrix(int ss, int num_w): nw(num_w), ss_num(ss), 
        matrix(ss, vector<ss_queue>(nw, ss_queue(new safe_queue<T>()))) {}

    ss_queue get_queue(int ss, int wkr_idx) {
        return matrix[ss][wkr_idx];
    }

    vector<ss_queue> get_ss_queues(int ss) {
        return matrix[ss];
    }

};

#endif