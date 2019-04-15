#include <iostream>
#include <vector>

#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include <ff/parallel_for.hpp>

#include "utimer.cpp"

using namespace std;
using namespace ff;

int count_primes(int n) {
    int sieve[n+1];
    for(int i = 0; i <= n; i++)
	    sieve[i] = 1;      
    for(int i = 2; i <= n; i++) {
	    if( i*i > n) 
	        break;   
	    if (sieve[i] == 1) 
	        for(int j = i; i*j <= n; j++) 
	            sieve[i*j] = 0; 
    }
    int count = 0;
    for(int i = 0; i <= n; i++)
	    if (sieve[i] == 1)
	        count++;
    return count;
}

struct couple {
    couple(int a, int b):a(a), b(b) {}
    int a, b;
};

vector<couple> output;

struct emitter: ff_node_t<couple> {

    emitter(const int num_tasks):num_tasks(num_tasks) {}

    couple* svc(couple *) {
        for(int i=0; i<num_tasks; ++i) {
            ff_send_out(new couple(rand() % 10000 + 1, 0));
        }
        return EOS;
    }

    const int num_tasks;
};

struct farm_stage: ff_node_t<couple> {

    couple* svc(couple *task) {
        couple &t = *task;
        t.b = count_primes(t.a);
        return task;
    }

};

struct collector: ff_node_t<couple> {

    couple* svc(couple *task) {
        couple &t = *task;
        output.push_back(t);
        delete task;
        return GO_ON;
    }

};

int main(int argc, char *argv[]) {

	if (argc != 4) {
        cout << "ERROR: usage: ./assignment2 num_tasks nw seed" << endl;
        return -1;
    }

	int num_tasks = atoi(argv[1]);
    int nw = atoi(argv[2]);
    int seed = atoi(argv[3]);
    srand(seed);

    emitter emit(num_tasks);
    collector collect;

    ff_Farm<float> farm(
        [nw]() {
            vector<std::unique_ptr<ff_node> > w;
            for(size_t i=0;i<nw;++i)
                w.push_back(make_unique<farm_stage>());
            return w;
        } ()
    );

    ff_Pipe<> pipe(emit, farm, collect);

    {
        utimer timer("ff farm version");

        if (pipe.run_and_wait_end()<0) {
            error("running pipe");
            return -1;
        }
    }

    cout << "--------------------------------------------------" << endl;

    int a[num_tasks];
    int b[num_tasks];

    {
        utimer timer("sequential version");

        for(int i = 0; i < num_tasks; i++) {
            a[i] = count_primes(output.at(i).a);
        }
    }

    cout << "--------------------------------------------------" << endl;

    {

        utimer timer("parallel for version");

        ParallelFor pf(nw);
        pf.parallel_for(0L,num_tasks,
            [&b](const long i) {
                b[i] = count_primes(output.at(i).a);
            }
        );

        bool mistake = false;
        for(int i = 0; i < num_tasks; i++) {
            if(b[i] != a[i] || a[i] != output.at(i).b)
                mistake = true;
        }

        if(mistake)
            cout << "computation result were different" << endl;
    }

}