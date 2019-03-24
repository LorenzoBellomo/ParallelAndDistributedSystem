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

void func(vector<int>::iterator begin, vector<int>::iterator end, vector<int>::iterator output) {
    transform(begin, end, output, count_primes);
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
    
    out = vector<int> (num_tasks);
    int range = num_tasks / nw;

    {
        utimer timer("posix threads version");
        
        for(int i = 0; i < nw; i++) {
            threads.push_back(thread(
                func, 
                in.begin() + (i * range),
                in.begin() + (i + 1) * range,
                out.begin() + (i * range)
                ));
        }

        for(thread& t: threads)
            t.join();

    }

    return 0;
}