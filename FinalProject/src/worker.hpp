#include <iostream> 
#include <vector>
 
#ifndef BSP_WORKER
#define BSP_WORKER

using namespace std;

template <typename T>
class worker {
private:
    safe_queue<T> input_queue;

public:
    worker()

}

#endif