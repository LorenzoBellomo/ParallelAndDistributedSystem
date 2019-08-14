
#ifndef LOGIC_BSP
#define LOGIC_BSP

/*
    filename: logicBSP.hpp
    author: Lorenzo Bellomo
    description: interface for a generic worker, to be subclassed providing the business logic code
*/

#include <iostream>
#include <functional>

#ifdef TSEQ
#include <map>
#endif

#include <safeQueue.hpp>

template <typename T>
class logicBSP {

protected: 
    // typedefs are available to subclasses
    typedef std::shared_ptr<safe_queue<T>> ss_queue;
    typedef std::function<void(ss_queue, size_t, std::vector<ss_queue>)> ss_function;
    
public: 
    // switcher is the function that, given ss_index as the super step identifier (0 for ss 0...),
    // returns the correct super step function for the runtime to call
    virtual ss_function switcher(size_t ss_index) = 0;

#ifdef TSEQ
    // if TSEQ is defined then the owner wants to collect some partial times. If the application
    // writer also wants to collect some times and display them to the stdout, then this function 
    // makes it possible, by default it does nothing
    virtual void dump_tseq() = 0;
#endif

};

#endif