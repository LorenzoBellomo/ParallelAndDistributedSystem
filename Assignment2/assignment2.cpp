#include <iostream>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <algorithm>
#include <thread>

#include "utimer.cpp"

using namespace std;

vector<int> in;
vector<int> out;

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

void func(int i, int range) {
    transform(in.begin() + (range * i), in.end() + (i + 1) * range -1, out.begin(), count_primes);
}

int main(int argc, char *argv[]){

    if (argc != 3) {
        cout << "ERROR: usage: ./assignment2 num_tasks nw" << endl;
        return -1;
    }

    int num_tasks = atoi(argv[1]);
    int nw = atoi(argv[2]);
    vector<thread> threads;
    srand(time(NULL));

    for(int i = 0; i < num_tasks; i++) 
        in.push_back(rand() % 10000 + 1);

    int range = num_tasks / nw;
    
    for(int i = 0; i < nw; i++) {
        threads.push_back(thread(func, i, range));
    }

    for(thread& t: threads)
        t.join();

    return 0;
     
}