#include <iostream>
#include <vector>
#include <map>

#include <ff/ff.hpp>
#include <ff/all2all.hpp>

#include <utimer.cpp>

using namespace std;
using namespace ff;

pair<int, int> my_map(int e) {
    return pair(1, e);
}

int my_reduce(int a, int b) {
    return a + b;
}

struct Task_Map {
    Task_Map(vector<int>::iterator begin, vector<int>::iterator end): b(begin), e(end) {}

    vector<int>::iterator b, e;
};

struct Map_Worker: ff_monode_t<Task_Map, pair<int,int>> {

    Map_Worker(Task_Map task): task(task) {}

    pair<int,int>* svc(Task_Map *) {

        const int nw_reduce = get_num_outchannels();
        
        for(auto ptr = task.b; ptr < task.e; ptr++) {
            size_t idx = my_hash(*ptr) % nw_reduce;
            pair<int, int> next_task = my_map(*ptr);
            this->ff_send_out_to(new pair<int, int>(next_task.first, next_task.second), idx);
        }
        return EOS;
    }
    Task_Map task;
    hash<int> my_hash;
};

struct Reduce_Worker: ff_minode_t<pair<int,int>, void> {

    Reduce_Worker(int nw_map): ended(0), nw_map(nw_map) {}

    void* svc(pair<int,int> *in) {
        auto x = (*in);
        auto prev = partial_results[x.second];
        partial_results.insert_or_assign(x.second, prev + x.first);
        if (ended != nw_map)
            return this->GO_ON;
        else
            return this->EOS;
    }

    void eosnotify(ssize_t) {
	    ended++;
    }

    void svc_end() {
        for (auto x : partial_results){
            printf("<%d,%d> \n", x.second, x.first);   
        }
    }
    map<int, int> partial_results;
    int ended, nw_map;
};

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
    int nw_map = atoi(argv[2]);
    int nw_reduce = atoi(argv[3]);
    srand(seed);

    vector<int> elem;

    for(int i = 0; i < n; i++)
        elem.push_back(rand() % 11 + 1);

    {
        utimer timer("Fastflow parallel version");
        std::vector<ff_node*> map_workers; 
        std::vector<ff_node*> reduce_workers; 

        int range = elem.size() / nw_map;
        int extra = elem.size() % nw_map;
        int prev = 0;
        for(int i = 0; i < extra; ++i) {
            Task_Map task(elem.begin() + prev, elem.begin() + prev + range + 1);
            map_workers.push_back(new Map_Worker(task));
            prev += (range + 1);
        }
        for(int i = 0; i < nw_map - extra; ++i) {
            Task_Map task(elem.begin() + prev, elem.begin() + prev + range);
            map_workers.push_back(new Map_Worker(task));
            prev += range;
        }
       
        for(int i = 0; i < nw_reduce; ++i){
            reduce_workers.push_back(new Reduce_Worker(nw_map));
        }

        ff_a2a a2a;
        a2a.add_firstset(map_workers);
        a2a.add_secondset(reduce_workers);
        if (a2a.run_and_wait_end()<0) {
            error("running a2a\n");
            return -1;
        }
    }

}