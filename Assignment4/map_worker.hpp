#include <iostream>
#include <optional>
#include <functional>

#include <safeQueue.hpp>

#ifndef MAP_WORKER
#define MAP_WORKER
using namespace std;

template <typename Tin, typename Tout, typename Tkey>
class map_worker {
private:
    function<pair<Tout, Tkey>(Tin)> func;
    safe_queue<optional<Tin>> *input_queue;
    vector<unique_ptr<safe_queue<optional<pair<Tout, Tkey>>>>> *output_queues;
    hash<Tkey> my_hash;

public:
    map_worker(function<pair<Tout, Tkey>(Tin)> f, vector<unique_ptr<safe_queue<optional<pair<Tout, Tkey>>>>> *qu): func(f), output_queues(qu) {
        input_queue = new safe_queue<optional<Tin>>();
    }

    void execute_loop() {

        bool eos = false;
        while(!eos) {
            optional<Tin> elem = input_queue->pop();
            if(elem.has_value()) {
                pair<Tout, Tkey> res = func(elem.value());
                int idx = my_hash(res.second) % (*output_queues).size();
                (*output_queues)[idx]->push(res);
            } else {
                eos = true;
                for(auto& q : *output_queues)
                    q->push(nullopt);
            }
        }
        cout << "map end" << endl;
    }

    void give_task(Tin task) {
        input_queue->push(task);
    }

    void end_stream() {
        input_queue->push(nullopt);
    }
};

#endif