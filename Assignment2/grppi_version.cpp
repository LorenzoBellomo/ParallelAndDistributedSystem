#include <iostream>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <optional>


#include "utimer.cpp"
#include <grppi.h>

using namespace std;
using namespace grppi;

vector<int> in;
vector<int> out;

void print_vector(vector<int> v) {
    for(int i : v) 
        cout << i << " ";
    cout << endl;
}

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

int func(int n) {
    out[i] = count_primes(in[i]);
    return i;
}

int main(int argc, char *argv[]){

    if (argc != 3) {
        cout << "ERROR: usage: ./omp_version num_tasks nw" << endl;
        return -1;
    }

    int num_tasks = atoi(argv[1]);
    int nw = atoi(argv[2]);
    vector<thread> threads;
    srand(time(NULL));

    for(int i = 0; i < num_tasks; i++) 
        in.push_back(rand() % 10000 + 1);

    out = vector<int> (num_tasks);

    auto generator = [num_tasks] () -> optional<int> {
        static int i = 0;
        if(i < num_tasks)
	        return(i++);
        else
	        return {};
    };
    auto merger = [] (int i) {};

    {
        utimer timer("grppi version");
        
        dynamic_execution thr = grppi::parallel_execution_native{};
        pipeline(thr, generator, farm(nw, func), merger);

    }

    print_vector(out)

    return 0;
}