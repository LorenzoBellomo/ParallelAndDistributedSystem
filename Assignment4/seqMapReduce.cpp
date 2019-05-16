#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

#include <utimer.cpp>
#include <mapReduce.hpp>

using namespace std;

pair<int, string> my_map(string s) {
    return pair(1, s);
}

int my_reduce(int a, int b) {
    return a + b;
}
 

int main(int argc, char *argv[]) {

    if(argc != 1 && argc != 2) {
		cout << "Usage is: mapReduce (opt:seed)" << endl;
		return(0);
	}

    {
        utimer timer("sequential version");

        int seed;
        if(argc == 2)
            seed = atoi(argv[1]);
        else
            seed = time(NULL);
        vector<string> elem;
        vector<pair<int, string>> mapped;
        srand(seed);
        mapReduce<string, int, string> mr(my_map, my_reduce);
    
        for(auto i : elem) {
            mapped.push_back(mr.map_function(i));
        }

        sort(   
            mapped.begin(), 
            mapped.end(), 
            [] (pair<int, string> p1, pair<int, string> p2) {
                    return p1.second < p2.second;
            }
        );
        for(auto i : mapped)
            cout << i.first << " " << i.second << ", ";
    }

}