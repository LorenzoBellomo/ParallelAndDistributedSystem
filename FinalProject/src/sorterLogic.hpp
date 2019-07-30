
#ifndef SORTER_LOGIC
#define SORTER_LOGIC

#include <iostream>
#include <vector>
#include <optional> 

#include <logicBSP.hpp>

class sorter_worker: public logicBSP<long> {

private:

    size_t nw;
    std::vector<long> elem;
    std::vector<long> samples;
    std::vector<long> output_v;

public:

    sorter_worker(
        size_t num_w,
        std::vector<long>::iterator begin, 
        std::vector<long>::iterator end): nw(num_w), elem(begin, end) {}

    void ss1(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {
        std::sort(elem.begin(), elem.end());

        auto range = elem.size() / (nw + 1);
        auto extra = elem.size() % (nw + 1);
        auto prev = elem.begin();
        for(size_t i = 0; i < extra; i++) {
            samples.push_back(*prev);
            prev += (range + 2);
        }
        for(size_t i = 0; i < nw - extra; i++) {
            samples.push_back(*prev);
            prev += range + 1;
        }
        samples.push_back(*(elem.end() - 1));

        for(size_t i = 0; i <  next_queues.size(); i++) {
            if(i != worker_idx) {
                auto q = next_queues[i];
                q->push_multiple(samples.begin(), samples.end());
            }
        }
    }

    void ss2(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {
        std::optional<long> next;
        while((next = my_queue->try_pop()).has_value())
            samples.push_back(next.value());

        std::sort(samples.begin(), samples.end());

        std::vector<long> separators;
        auto range = samples.size() / (nw + 1);
        auto extra = samples.size() % (nw + 1);
        auto prev = samples.begin();
        for(size_t i = 0; i < extra; i++) {
            separators.push_back(*prev);
            prev += (range + 2);
        }
        for(size_t i = 0; i < nw - extra; i++) {
            separators.push_back(*prev);
            prev += range + 1;
        }
        separators.push_back(*(samples.end() - 1));

        auto elem_iter = elem.begin();
        auto separator_iter = separators.begin(); 
        for(size_t i = 0; i < next_queues.size(); i++) {
            long next_separator = *(separator_iter+1);
            auto q = next_queues[i];
            std::vector<long> tmp;
            while(*elem_iter < next_separator && elem_iter != elem.end()) {
                if(worker_idx == i)
                    output_v.push_back(*elem_iter);
                else 
                    tmp.push_back(*elem_iter);

                elem_iter++;
            }
            if(worker_idx != i)
                q->push_multiple(tmp.begin(), tmp.end());
            separator_iter++;
        }
        if(worker_idx == nw-1)
            output_v.push_back(separators[nw]);
    }

    void ss3(
        logicBSP::ss_queue my_queue, 
        size_t worker_idx, 
        std::vector<logicBSP::ss_queue> next_queues)
    {
        std::optional<long> next;
        while((next = my_queue->try_pop()).has_value())
            output_v.push_back(next.value());

        std::sort(output_v.begin(), output_v.end());
    }

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

};

#endif