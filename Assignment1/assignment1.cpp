#include <iostream>
#include <vector>
#include <thread>
#include "safeQueue.hpp"


using namespace std;

void stage1(int m, safe_queue<int> next_stage) {
    for(int i = 0; i < m; i++) 
        to_next_stage.push(i);
    //push EOF

}

void stage2(safe_queue<int> prev_stage, safe_queue<int> next_stage) {
    while(true) {
        int elem = prev_stage.pop();
        //ACTIVE DELAY
        next_stage.push(++elem)
    }
}

void stage3(safe_queue<int> prev_stage, safe_queue<int> next_stage) {
    while(true) {
        int elem = prev_stage.pop();
        //ACTIVE DELAY
        next_stage.push(elem*elem)
    }
}

void stage4(safe_queue<int> prev_stage, safe_queue<int> next_stage) {
    while(true) {
        int elem = prev_stage.pop();
        //ACTIVE DELAY
        next_stage.push(--elem)
    }
}

void stage5(safe_queue<int> prev_stage) {
    while(true) {
        int elem = prev_stage.pop();
        //ACTIVE DELAY
        cout << elem << endl;
    }
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "ERROR: usage: ./a.out m";
        return -1;
    }
    int m = atoi(argv[1]);

    vector<thread> threads(5);
    vector<safe_queue<int>> queues(4);

    threads.push_back(stage1, m, queues.at(0));
    threads.push_back(stage2, queues.at(0), queues.at(1));
    threads.push_back(stage3, queues.at(1), queues.at(2));
    threads.push_back(stage4, queues.at(2), queues.at(3));
    threads.push_back(stage5, queues.at(3));

    for(thread& t: threads)
        t.join();

    return 0;
}
