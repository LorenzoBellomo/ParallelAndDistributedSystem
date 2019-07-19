#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include <utimer.cpp>
//#include <BSP>
//#include <seqBSP>
//#include <parBSP>

using namespace std;

int main(int argc, char *argv[]) {

    if(argc != 3 && argc != 4) {
        cout << "Usage is ./sorter n nw (opt:seed)" << endl
            << "Where:" << endl 
            << "- n is the number of elements" << endl
            << "- nw is the number of workers" << endl
            << "- seed is the RNG seed (and it is optional)" << endl;
        return -1;
    }
    
    long n = atoi(argv[1]);
    int nw = atoi(argv[2]);
    int seed = (argc == 4)? atoi(argv[3]) : time(NULL);

    // Generate random unique vector (size n) with elements in range [0, 5*n)
    vector<long> input;
    for(long i = 0; i < n*5; i++)
        input.push_back(i);
    shuffle(input.begin(), input.end(), default_random_engine(seed));
    input.erase(input.begin() + n, input.end());

    {
        utimer timer("parallel version");
    }

    {
        utimer timer("std::sort");
        sort(input.begin(), input.end());
    }

    return 0;
}