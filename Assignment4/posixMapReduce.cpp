#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <thread>

#include <utimer.cpp>
#include <mapReduce.hpp>
#include <worker.hpp>

using namespace std;

int nw_map, nw_reduce;
vector<thread> thd;
thread coordinator_thd;

pair<int, int> my_map(int e) {
    return pair(1, e);
}

int my_reduce(int a, int b) {
    return a + b;
}

void emitter() {
    // TODO coordinator that acts as a emitter for map stage
}

void collector() {
    // TODO coordinator that acts as a collector for map stage, emitter of reduce stage
}

vector<pair<int, int>> reduce_worker_function(pair<iterator<int>, iterator<int>>) {
    // TODO function that sorts the array, and reduces on it
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
    vector<int> elem;
    srand(seed);

    for(int i = 0; i < n; i++)
        elem.push_back(rand() % 11 + 1);

    {
        utimer timer("Posix parallel version");

        coordinator_thd = thread(coordinator);

        for(int i = 0; i < nw_map; i++) {
            worker<int, pair<int, int>> work(my_map);
            thd.push_back(thread(&worker<int, pair<int, int>>::execute_loop, work));
        }
        for(int i = 0; i < nw_reduce; i++) {
            worker<pair<iterator<int>, iterator<int>>, vector<pair<int, int>> work(reduce_worker_function);
            thd.push_back(thread(
                &worker<pair<iterator<int>, iterator<int>>, vector<pair<int, int>>>::execute_loop,
                work
            ));
        }

        for(thread& t : thd)
            t.join();

        coordinator_thd.join();
    }
}