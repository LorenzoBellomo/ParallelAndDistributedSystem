#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <thread>

#include <utimer.cpp>
#include <map_worker.hpp>
#include <reduce_worker.hpp>

using namespace std;

int nw_map, nw_reduce;
vector<thread> thd;
vector<int> elem;
vector<map_worker<int, int, int>> map_workers;
vector<reduce_worker<int, int>> reduce_workers;
thread emitter_thd;
thread collector_thd;
vector<unique_ptr<safe_queue<optional<pair<int, int>>>>> queues;
vector<pair<int,int>> result;

pair<int, int> my_map(int e) {
    return pair(1, e);
}

int my_reduce(int a, int b) {
    return a + b;
}

void emitter() {
    int i = 0;
    for(auto& e : elem) {
        map_workers[i].give_task(e);
        i = (i + 1) % nw_map;
    }
    for(auto& w : map_workers)
        w.end_stream();

}

void collector() {

    int eos = 0;
    while(eos < reduce_workers.size()) {
        for(auto& wk : reduce_workers) {
            auto e = wk.try_pull();
            if(e.first) {
                auto elem = e.second.value();
                if(elem.has_value()) 
                    result.push_back(elem.value());
                else
                    eos++;
            }
        }
    }
    
}


int main(int argc, char *argv[]) {

    if(argc != 4 && argc != 5) {
		cout << "Usage is: mapReduce n nwMap nwReduce (opt:seed)" << endl;
		return(0);
	}

    int seed;
    if(argc == 5)
        seed = atoi(argv[4]);
    else
        seed = time(NULL);
    int n = atoi(argv[1]);
    nw_map = atoi(argv[2]);
    nw_reduce = atoi(argv[3]);
    srand(seed);

    for(int i = 0; i < n; i++)
        elem.push_back(rand() % 11 + 1);

    {
        utimer timer("Posix parallel version");

        for(int i = 0; i < nw_reduce; i++) {
            queues.push_back(make_unique<safe_queue<optional<pair<int, int>>>>());
        }
        for(int i = 0; i < nw_map; i++) {
            map_worker<int, int, int> work(my_map, &queues);
            map_workers.push_back(work);
        }
        for(int i = 0; i < nw_reduce; i++) {
            reduce_worker<int, int> work(my_reduce, &queues, nw_map);
            reduce_workers.push_back(work);
        }
        
        thd.push_back(thread(emitter));
        for(auto& w : map_workers) 
            thd.push_back(thread(&map_worker<int, int, int>::execute_loop, &w));
        for(auto& w : reduce_workers) 
            thd.push_back(thread(&reduce_worker<int, int>::execute_loop, &w));
        thd.push_back(thread(collector));

        for(thread& t : thd)
            t.join();

    }

    for(auto e : result)
        cout << "<" << e.first << "," << e.second << "> ";
    cout << endl;
}