#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#include <utimer.cpp>
#include <mapReduce.hpp>

using namespace std;

pair<int, int> my_map(int e) {
    return pair(1, e);
}

int my_reduce(int a, int b) {
    return a + b;
}
 

int main(int argc, char *argv[]) {

    if(argc != 2 && argc != 3) {
		cout << "Usage is: mapReduce n (opt:seed)" << endl;
		return(0);
	}

    int seed;
    if(argc == 3)
        seed = atoi(argv[2]);
    else
        seed = time(NULL);
    int n = atoi(argv[1]);
    vector<int> elem;
    vector<pair<int, int>> mapped;
    vector<pair<int, int>> reduced;
    srand(seed);

    for(int i = 0; i < n; i++)
        elem.push_back(rand() % 11 + 1);

    {    
        utimer timer("sequential version");
        mapReduce<int, int, int> mr(my_map, my_reduce);
    
        for(auto i : elem) {
            mapped.push_back(mr.map_function(i));
        }

        sort(   
            mapped.begin(), 
            mapped.end(), 
            [] (pair<int, int> p1, pair<int, int> p2) {
                    return p1.second < p2.second;
            }
        );

        auto prev = *(mapped.begin());
        for(auto ptr = mapped.begin() + 1; ptr < mapped.end(); ptr++) {
            pair<int, int> e = *ptr;
            if(e.second == prev.second)
                prev = pair(mr.reduce_function(prev.first, e.first), e.second);
            else {
                reduced.push_back(prev);
                prev = e;
            }
        }

        
        for(auto i : reduced)
            cout << "<" << i.first << ", " << i.second << "> ";
        cout << endl;
    }

}