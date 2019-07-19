#include <iostream> 
#include <vector>
 
#ifndef BSP
#define BSP

using namespace std;

template <typename T>
class BSP {
private:
    int currentSS;
    
    
public:

    virtual void BSP::send_to(
        vector<T>::iterator begin, 
        vector<T>::iterator end,
        int worker_index
    ) = 0;

};

#endif