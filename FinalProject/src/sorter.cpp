
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <deque>

#include <utimer.hpp>
#include <posixBSP.hpp>
#include <superStep.hpp>

using namespace std;

struct ss1: super_step<long> {

    void ss_func(
        typename deque<long>::reverse_iterator begin, 
        typename deque<long>::reverse_iterator end, 
        int my_idx, 
        shared_ptr<super_step<T>> next_ss)
    {
        elements.insert(elements.begin(), input_begin, input_end);

        sort(elements.begin(), elements.end());
        vector<long> samples;
        long range = elements.size() / (get_nw() + 1);
        long extra = elements.size() % (get_nw() + 1);
        long prev = 0;
        for(int i = 0; i < extra; ++i) {
            samples.push_back(*(elements.begin() + prev + 1));
            prev += (range + 1);
        }
        for(int i = 0; i < nw - extra; ++i) {
            samples.push_back(*(elements.begin() + prev));
            prev += range;
        }
        for(int i = 0; i < get_nw(); i++)
            next_ss->offer_tasks(samples.begin(), samples.end(), i)
    }

    void give_iterators(vector<long>::iterator begin, vector<long>::iterator end) {
        input_begin = begin;
        input_end = end;
    }


    vector<long> elements;
    vector<long>::iterator input_begin, input_end;
};

struct ss2: super_step<long> {

    void ss_func(
        typename deque<long>::reverse_iterator begin, 
        typename deque<long>::reverse_iterator end, 
        int my_idx, 
        shared_ptr<super_step<T>> next_ss)
    {
        vector<long> elements(input_begin, input_end);
        for(long i = begin; i < end; i++) {
            tmp.push_back((*i)+1);
        }
        next_ss->offer_tasks(tmp.begin(), tmp.end(), 0);
    }

};

struct ss3: super_step<long> {
    void ss_func(
        typename deque<long>::reverse_iterator begin, 
        typename deque<long>::reverse_iterator end, 
        int my_idx, 
        shared_ptr<super_step<T>> next_ss)
    {
        for(long i = begin; i < end; i++) {
            cout << *i << " ";
        }
        cout << endl;
    }
};

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
    vector<long> in_tasks;
    vector<long> out_tasks(n);
    for(long i = 0; i < n*5; i++)
        in_tasks.push_back(i);
    shuffle(in_tasks.begin(), in_tasks.end(), default_random_engine(seed));
    in_tasks.erase(in_tasks.begin() + n, in_tasks.end());

    {
        utimer timer("parallel version");

        vector<shared_ptr<super_step<long>>> ss;
        ss.push_back(shared_ptr<ss1>(new ss1()));
        ss.push_back(shared_ptr<ss2>(new ss2()));
        ss.push_back(shared_ptr<ss3>(new ss3()));

        auto init_logic = [] () {
            long range = in_tasks / nw;
            long extra = in_tasks % nw;
            long prev = 0;
            for(int i = 0; i < extra; ++i) {
                ss[0]->give_iterators
                    (in_tasks.begin() + prev, in_tasks.begin() + prev + range + 1);
                prev += (range + 1);
            }
            for(int i = 0; i < nw - extra; ++i) {
                ss[0]->give_iterators
                    (in_tasks.begin() + prev, in_tasks.begin() + prev + range);
                prev += range;
            }
            
        };

        posixBSP<long> p(ss, 1, init_logic);
        p.start_and_wait();
    }

    {
        utimer timer("std::sort");
        sort(input.begin(), input.end());
    }

    return 0;
}