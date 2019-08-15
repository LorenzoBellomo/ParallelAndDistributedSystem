
#ifndef SORTER_LOGIC
#define SORTER_LOGIC

/*
    filename: sorterLogic.hpp
    author: Lorenzo Bellomo
    description: The logicBSP subclass that implements the Tiskin sorting algorithm
*/

#include <iostream>
#include <vector>
#include <optional> 
#include <ctime>

#ifdef TSEQ
#include <map>
#include <string>
#include <sstream>
#endif

#include <logicBSP.hpp>

class sorter_worker: public logicBSP<long> {

private:

    size_t nw;
    std::vector<long> elem;
    std::vector<long> samples;
    std::vector<long> output_v;

#ifdef TSEQ
    std::map<std::string, long> tseq_map;
    std::stringstream ss;
#endif

public:

    sorter_worker(
        size_t num_w,
        std::vector<long>::iterator begin, 
        std::vector<long>::iterator end): nw(num_w), elem(begin, end) {}

    // split in_vector in p ranges by computing p+1 samples, and return 
    // the p+1 samples equally spaced
    static std::vector<long> split_with_samples(std::vector<long> in_vector, size_t p) {
        
        std::vector<long> tmp;

        auto range = in_vector.size() / (p + 1);
        auto extra = in_vector.size() % (p + 1);
        auto prev = in_vector.begin();
        for(size_t i = 0; i < extra; i++) {
            tmp.push_back(*prev);
            prev += (range + 1);
        }
        for(size_t i = 0; i < p - extra; i++) {
            tmp.push_back(*prev);
            prev += range;
        }
        tmp.push_back(*(in_vector.end() - 1));

        return tmp;
    }

    void ss1(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {
#ifdef TSEQ
        auto start = std::chrono::system_clock::now();
#endif
        std::sort(elem.begin(), elem.end());

        samples = split_with_samples(elem, nw);
        
#ifdef TSEQ
        ss.str("");
        ss.clear();
        ss << "computation ss1 worker " << worker_idx;
        tseq_map.insert({ss.str(), utimer::elapsed(start)});
        start = std::chrono::system_clock::now();
#endif

        for(size_t i = 0; i <  next_queues.size(); i++) {
            if(i != worker_idx) {
                auto q = next_queues[i];
                q->push_multiple(samples.begin(), samples.end());
            }
        }
#ifdef TSEQ
        ss.str("");
        ss.clear();
        ss << "communication ss1 worker " << worker_idx;
        tseq_map.insert({ss.str(), utimer::elapsed(start)});
        start = std::chrono::system_clock::now();
#endif
    }

    void ss2(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {
#ifdef TSEQ
        auto start = std::chrono::system_clock::now();
#endif
        std::optional<long> next;
        while((next = my_queue->try_pop()).has_value())
            samples.push_back(next.value());

        std::sort(samples.begin(), samples.end());

        std::vector<long> separators = split_with_samples(samples, nw);
    
        // below is just a linear algorithm (O(size(elem))) to split the elements of elem
        // between the output queues according to the separators
        std::vector<std::vector<long>> tmp(next_queues.size());
        auto elem_iter = elem.begin();
        auto separator_iter = separators.begin(); 
        for(size_t i = 0; i < next_queues.size(); i++) {
            long next_separator = *(separator_iter+1);
            auto q = next_queues[i];
            while(*elem_iter < next_separator && elem_iter != elem.end()) {
                if(worker_idx == i) // I do not put them in the queue
                    output_v.push_back(*elem_iter);
                else 
                    tmp[i].push_back(*elem_iter); 
                // I store in a tmp vector and send it all later to lock the queue only once
                elem_iter++;
            }
            separator_iter++;
        }
        if(worker_idx == nw-1) // the last worker misses the last separator with the provided algorithm
            output_v.push_back(separators[nw]);

#ifdef TSEQ
        ss.str("");
        ss.clear();
        ss << "computation ss2 worker " << worker_idx;
        tseq_map.insert({ss.str(), utimer::elapsed(start)});
        start = std::chrono::system_clock::now();
#endif

        // now communication phase, in tmp[i] I have the elements to send to queue i
        for(size_t i = 0; i < next_queues.size(); i++) {
            auto q = next_queues[i];
            if(worker_idx != i) 
                q->push_multiple(tmp[i].begin(), tmp[i].end());
        }

#ifdef TSEQ
        ss.str("");
        ss.clear();
        ss << "communication ss2 worker " << worker_idx;
        tseq_map.insert({ss.str(), utimer::elapsed(start)});
        start = std::chrono::system_clock::now();
#endif
    }

    void ss3(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {
#ifdef TSEQ
        auto start = std::chrono::system_clock::now();
#endif
        std::optional<long> next;
        while((next = my_queue->try_pop()).has_value())
            output_v.push_back(next.value());

        std::sort(output_v.begin(), output_v.end());
#ifdef TSEQ
        ss.str("");
        ss.clear();
        ss << "computation ss3 worker " << worker_idx;
        tseq_map.insert({ss.str(), utimer::elapsed(start)});
        start = std::chrono::system_clock::now();
#endif
    }

    // switcher function, maps ss1 to super step indexed 0, ss2 to 1 and ss3 to 2
    logicBSP::ss_function switcher(size_t idx) {
        switch(idx) {
            using namespace std::placeholders;
            case 0: 
                {
                    auto f = bind(&sorter_worker::ss1, this, _1, _2, _3);
                    return f;
                }
            case 1: 
                {
                    auto f = bind(&sorter_worker::ss2, this, _1, _2, _3);
                    return f;
                }
            case 2: 
                {
                    auto f = bind(&sorter_worker::ss3, this, _1, _2, _3);
                    return f;
                }
        }
        return nullptr;
    }

#ifdef TSEQ
    void dump_tseq() {
        for(auto e : tseq_map)
            std::cout << e.first << " -> " << e.second << " usec" << std::endl;
    }
#endif

    void push_to_output_vector(std::vector<long>& out_v) {
        for(auto e : output_v)
            out_v.push_back(e);
    }

};

#endif