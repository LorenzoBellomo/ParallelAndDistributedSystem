#include <iostream>
#include <optional>
#include <functional>
#include <map>

#include <safeQueue.hpp>

#ifndef REDUCE_WORKER
#define REDUCE_WORKER
using namespace std;

template <typename Tout, typename Tkey>
class reduce_worker {
private:
    function<Tout(Tout, Tout)> func;
    vector<unique_ptr<safe_queue<optional<pair<Tout, Tkey>>>>> *input_queues;
    safe_queue<optional<pair<Tout, Tkey>>> *output_queue;
    map<Tkey, Tout> partial_results;

public:
    reduce_worker(function<Tout(Tout, Tout)> f, vector<unique_ptr<safe_queue<optional<pair<Tout, Tkey>>>>> *qu): func(f), input_queues(qu), partial_results() {
        output_queue = new safe_queue<optional<pair<Tout, Tkey>>>();
    }

    void execute_loop() {

        int eos = 0;
        while(eos < (*input_queues).size()) {
            for(auto& q : *input_queues) {
                auto elem = q->try_pop();
                if(elem.has_value()) {
                    auto e = elem.value();
                    if(e.has_value()) {
                        auto x = e.value();
                        auto prev = partial_results[x.second];
                        partial_results.insert_or_assign(x.second, prev + x.first);
                    }
                } else {
                    eos++;
                }
            }
        }
        for(auto& elem : partial_results) {
            output_queue->push(elem);
        }
        output_queue->push(nullopt);
    }

    optional<optional<pair<Tout, Tkey>>> try_pull() {
        return output_queue->try_pop();
    }
    
};

#endif