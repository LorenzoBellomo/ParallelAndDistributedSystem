#include <iostream>
#include <optional>
#include <functional>

#include <safeQueue.hpp>

#ifndef REDUCE_WORKER
#define REDUCE_WORKER
using namespace std;

template <typename Tout, typename Tkey>
class reduce_worker {
private:
    function<Tout(pair<Tout, Tout>)> func;
    vector<safe_queue<optional<pair<Tout, Tkey>>>> input_queues;
    safe_queue<optional<pair<Tout, Tkey>>> *output_queue;
    map<Tkey, Tout> partial_results;

public:
    reduce_worker(Tout(pair<Tout, Tout>)> f, vector<safe_queue<optional<pair<Tout, Tkey>>>> qu): func(f), input_queues(qu), partial_results() {
        output_queue = new safe_queue<optional<pair<Tout, Tkey>>>();
    }

    ~reduce_worker() {
        delete(output_queue);
    }

    void execute_loop() {

        int eos = 0;
        while(eos < input_queues.size()) {
            for(auto q : input_queues) {
                optional<pair<Tout, Tkey>> elem = q.pop();
                if(elem.has_value()) {
                    auto x = elem.value();
                    auto ptr = partial_results.insert(x.second, x.first);
                    if(ptr.second == false) {
                        // already present
                        Tout prev = ptr.first;
                        partial_results.erase(ptr);
                        partial_results.insert(pair(x.first, func(x.second, prev)));
                    }
                } else {
                    eos++
                }
            }
        }
        for(auto elem : partial_results) {
            output_queue->push(elem);
        }
        output_queue->push(nullopt);
    }

    optional<pair<Tout, Tkey>> try_pull() {
        return output_queue.try_pull();
    }
    
};

#endif