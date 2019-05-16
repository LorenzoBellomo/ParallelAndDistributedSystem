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
vector<worker> workers;
thread emitter_thd;
thread collector_thd;
vector<optional<safe_queue<pair<int, int>>>> queues;
vector<pair<int,int>> result;

pair<int, int> my_map(int e) {
    return pair(1, e);
}

int my_reduce(int a, int b) {
    return a + b;
}

void emitter() {
    int i = 0;
    for(auto ptr = elem.begin(); ptr < elem.end(); ptr++) {
        workers[i].give_task(*ptr);
        i = (i + 1) % nw_map;
    }
    for(int j = 0; j < nw; j++)
        workers[i].end_stream();
}

void collector() {

    int eos = 0;
    while(eos < queues.size()) {
        for(int i = nw_map; i < nw_reduce; i++) {
            optional<pair<int, int>> e = queues[i].try_pull();
            if(e != NULL) {
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

        collector_thd = thread(collector);
        emitter_thd = thread(emitter);

        for(int i = 0; i < nw_reduce; i++)
            queues.push_back(safe_queue());

        for(int i = 0; i < nw_map; i++) {
            map_worker<int, pair<int, int>> work(my_map, queues);
            thd.push_back(thread(&map_worker<int, pair<int, int>>::execute_loop, work));
            workers.push_back(work);
        }
        for(int i = 0; i < nw_reduce; i++) {
            reduce_worker<int, int> work(my_reduce, queues);
            thd.push_back(thread(&reduce_worker<int, int>::execute_loop, work));
            workers.push_back(work);
        }

        for(thread& t : thd)
            t.join();

        emitter_thd.join();
        collector_thd.join();

    }

    for(auto e : result)
        cout << "<" << e.first << "," << e.second << "> ";
    cout << endl;
}