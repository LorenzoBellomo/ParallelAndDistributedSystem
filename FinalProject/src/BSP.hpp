 
#ifndef BSP_INTERFACE
#define BSP_INTERFACE

#include <iostream> 
#include <vector>

using namespace std;

template <typename T>
class BSP {
private:
public:

    virtual void send_to(
        typename vector<T>::iterator _begin, 
        typename vector<T>::iterator _end,
        int ss_lvl_index,
        int worker_index) = 0;
    
};

#endif