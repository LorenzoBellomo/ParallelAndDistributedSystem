
#ifndef QUEUE_MATRIX
#define QUEUE_MATRIX

#include <iostream>
#include <vector>

#include <safeQueue.hpp>

template <typename T>
class queue_matrix {
private:
    typedef std::shared_ptr<safe_queue<T>> ss_queue;

    int nw, ss_num;
    std::vector<std::vector<ss_queue>> matrix; // matrix[#super_steps][#workers] 
public:
    queue_matrix(int ss, int num_w): nw(num_w), ss_num(ss) {
        for(int i = 0; i < ss; i++) {
            matrix.push_back(std::vector<ss_queue>());
            for(int j = 0; j < nw; j++) {
                matrix[i].push_back(ss_queue(new safe_queue<T>()));
            }
        }
    }

    ss_queue get_queue(int ss, int wkr_idx) {
        return matrix[ss][wkr_idx];
    }

    std::vector<ss_queue> get_ss_queues(int ss) {
        return matrix[ss];
    }

};

#endif