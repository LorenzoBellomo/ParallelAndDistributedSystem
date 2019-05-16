#include <iostream>
#include <optional>
#include <functional>

#include <safeQueue.hpp>

#ifndef WORKER
#define WORKER
using namespace std;

template <typename Tin, typename Tout>
class map_worker {
private:
    function<Tout(Tin)> func;
    safe_queue<optional<Tin>> *input_queue;
    vector<safe_queue<optional<Tout>>> output_queues;

public:
    worker(function<Tout(Tin)> f, vector<safe_queue<optional<Tout>>> qu): func(f), output_queues(qu) {
        input_queue = new safe_queue<optional<Tin>>();
    }

    ~worker() {
        delete(input_queue);
    }

    void execute_loop() {

        bool eos = false;
        while(!eos) {
            optional<Tin> elem = input_queue->pop();
            if(elem.has_value()) {
                Tout res = func(elem.value());
                int idx = hash(res) / output_queues.size();
                output_queue[idx]s->push(res);
            } else {
                eos = true;
                for(auto q : output_queues)
                    q->push(nullopt);
            }
        }
    }

    void give_task(Tin task) {
        input_queue->push(task);
    }

    void end_stream() {
        input_queue->push(nullopt);
    }
};

#endif