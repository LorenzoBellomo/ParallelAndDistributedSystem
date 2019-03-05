#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

#include <unistd.h>

using namespace std;

mutex l; 
int x = 0;                  // this is global

void body(int n) {
  l.lock();
  int y = x;
  y = y + n;
  sleep(1); 
  x = y;
  // x += n; 
  l.unlock();
  return;
}

int main(int argc, char * argv[]) {

  int n = atoi(argv[1]);    // segv if not present ...

  vector<thread> t;

  for(int i=0; i<n; i++)
    t.push_back(thread(body,i));

  for(thread& it: t) {
    it.join();
  }

  cout << "x = " << x << endl; 
  return(0); 
}
