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
    int nw_map, id;

public:
    reduce_worker(function<Tout(Tout, Tout)> f, vector<unique_ptr<safe_queue<optional<pair<Tout, Tkey>>>>> *qu, int n_map, int id): func(f), input_queues(qu), partial_results(), nw_map(n_map), id(id) {
        output_queue = new safe_queue<optional<pair<Tout, Tkey>>>();
    }

    void execute_loop() {

        int eos = 0;
        while(eos < nw_map) {
            safe_queue<optional<pair<Tout, Tkey>>> *q = (*input_queues).at(id).get();
            auto elem = q->try_pop();
            if(elem.first) {
                auto e = elem.second.value();
                if(e.has_value()) {
                    auto x = e.value();
                    auto prev = partial_results[x.second];
                    partial_results.insert_or_assign(x.second, prev + x.first);
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

    pair<bool, optional<optional<pair<Tout, Tkey>>>> try_pull() {
        return output_queue->try_pop();
    }
    
};

#endif