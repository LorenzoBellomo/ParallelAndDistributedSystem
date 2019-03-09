#include <iostream>
#include <vector>
#include <thread>
#include <optional>
#include <chrono>
#include "safeQueue.hpp"


using namespace std;

void stage1(int m, safe_queue<optional<int>> *next_stage) {
    for(int i = 0; i < m; i++) 
        next_stage->push(i);
    next_stage->push(nullopt);
}

void stage2(safe_queue<optional<int>> *prev_stage, safe_queue<optional<int>> *next_stage) {
    bool eos = false;
    
    while(!eos) {
        optional<int> elem = prev_stage->pop();
        if(elem.has_value()) {
            int a = elem.value();
            this_thread::sleep_for(std::chrono::milliseconds(10));
            next_stage->push(++a);
        } else {
            eos = true;
            next_stage->push(nullopt);
        }
    }
    
}

void stage3(safe_queue<optional<int>> *prev_stage, safe_queue<optional<int>> *next_stage) {
    bool eos = false;
    
    while(!eos) {
        optional<int> elem = prev_stage->pop();
        if(elem.has_value()) {
            int a = elem.value();
            this_thread::sleep_for(std::chrono::milliseconds(10));
            next_stage->push(a*a);
        } else {
            eos = true;
            next_stage->push(nullopt);
        }
    }
    
}

void stage4(safe_queue<optional<int>> *prev_stage, safe_queue<optional<int>> *next_stage) {
    bool eos = false;
    
    while(!eos) {
        optional<int> elem = prev_stage->pop();
        if(elem.has_value()) {
            int a = elem.value();
            this_thread::sleep_for(std::chrono::milliseconds(10));
            next_stage->push(--a);
        } else {
            eos = true;
            next_stage->push(nullopt);
        }
    }
    
}

void stage5(safe_queue<optional<int>> *prev_stage) {
    bool eos = false;
    
    while(!eos) {
        optional<int> elem = prev_stage->pop();
        if(elem.has_value()) {
            int a = elem.value();
            cout << a << endl;
        } else 
            eos = true;
    }
    
}

int main(int argc, char* argv[]) {

    chrono::system_clock::time_point start = std::chrono::system_clock::now();

    if (argc != 2) {
        cout << "ERROR: usage: ./a.out m";
        return -1;
    }
    int m = atoi(argv[1]);

    vector<thread> threads;
    vector<safe_queue<optional<int>>> queues(4);

    threads.push_back(thread(stage1, m, &(queues.at(0))));
    threads.push_back(thread(stage2, &(queues.at(0)), &(queues.at(1))));
    threads.push_back(thread(stage3, &(queues.at(1)), &(queues.at(2))));
    threads.push_back(thread(stage4, &(queues.at(2)), &(queues.at(3))));
    threads.push_back(thread(stage5, &(queues.at(3))));

    for(thread& t: threads)
        t.join();

    chrono::system_clock::time_point end = std::chrono::system_clock::now();
    int elapsed = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    int best_time = m * 10;
    cout << "elapsed time: " << elapsed << endl;
    cout << "best possible time is " << best_time << endl;
    cout << "difference is " << elapsed - best_time << endl;

    return 0;
}
