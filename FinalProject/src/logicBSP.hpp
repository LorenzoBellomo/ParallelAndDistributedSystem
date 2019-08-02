
#ifndef LOGIC_BSP
#define LOGIC_BSP

#include <iostream>
#include <functional>

#ifdef TSEQ
#include <map>
#endif

#include <safeQueue.hpp>

template <typename T>
class logicBSP {

protected: 
    typedef std::shared_ptr<safe_queue<T>> ss_queue;
    typedef std::function<void(ss_queue, size_t, std::vector<ss_queue>)> ss_function;
    
public: 
    virtual ss_function switcher(size_t ss_index) = 0;

#ifdef TSEQ
    virtual void dump_tseq() = 0;
#endif

};

#endif