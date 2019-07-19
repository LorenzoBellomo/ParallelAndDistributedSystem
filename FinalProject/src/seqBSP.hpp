#include <iostream>
#include <vector>

#include <BSP.hpp>

#ifndef SEQ_BSP
#define SEQ_BSP

using namespace std;

template <typename T>
class seqBSP : public BSP {

private:

public:
    seqBSP(
        vector<T>::iterator begin,
        vector<T>::iterator end,
        vector<super_step<T>> super_steps
    ) {}

};

#endif