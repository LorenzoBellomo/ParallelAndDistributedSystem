#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <omp.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include <grppi.h>
#include <seq/sequential_execution.h>
#include <native/parallel_execution_native.h>
#include <omp/parallel_execution_omp.h>

// g++ Traduttore.cpp -lpthread -I/home/lorenzo/Desktop/Lorenzo/lib/grppi/include -fcilkplus

//#include <execution>

using namespace std;

typedef struct {
  int start;
  int end;
} RANGE;



int main(int argc, char * argv[]) {

  string filename = argv[1];
  int nw = atoi(argv[2]);

  // reading string from file
  string text = "";
  chrono::system_clock::time_point start = std::chrono::system_clock::now();
  std::ifstream t(argv[1]);
  string line;
  while(getline(t,line)){
    text.append(line);
  }
  t.close();
  chrono::system_clock::time_point end = std::chrono::system_clock::now();
  cout << "reading " <<
    chrono::duration_cast<std::chrono::microseconds>(end - start).count()  << " ";

  // keeping the original
  string original = text;
  // the traduttore function
  auto traduttore = [](char c){
    // this_thread::sleep_for(chrono::microseconds(1));
    return toupper(c);
  };

  // sequential : std::transform
  start = std::chrono::system_clock::now();
  transform(//execution::parallel_policy,
	    text.begin(), text.end(), text.begin(),
	    traduttore
	    );
  end = std::chrono::system_clock::now();
  cout << "transform " <<
    chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " " ;

  //this_thread::sleep_for(chrono::microseconds(8000000));

  // sequential : classical loop
  text = original;
  start = std::chrono::system_clock::now();
  for(unsigned int i=0; i<text.size() ; i++ ) {
    text[i] = traduttore(text[i]);
  }
  end = std::chrono::system_clock::now();
  cout << "for " <<
    chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " " ;

  // parallel : openmp
  text = original;
  start = std::chrono::system_clock::now();
// to be compiled with -fopenmp
#pragma omp parallel for num_threads(nw)
  for(unsigned int i=0; i<text.size() ; i++ ) {
    text[i] = traduttore(text[i]);
  }
  end = std::chrono::system_clock::now();
  cout << "omp  " <<
    chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " ";

  // parallel : c++ threads
  text = original;
  start = std::chrono::system_clock::now();
  vector<RANGE> ranges(nw);
  int m = text.size();
  int delta { m / nw };
  vector<thread> tids;

  for(int i=0; i<nw; i++) {
    ranges[i].start = i*delta;
    ranges[i].end   = (i != (nw-1) ? (i+1)*delta : m);

  }

  auto thread_body = [&](int a, int b, string t) {
    for(int i=a; i<b; i++)
      t[i] = traduttore(t[i]);
    return;
  };

  for(int i=0; i<nw; i++) {
    tids.push_back(thread(thread_body, ranges[i].start, ranges[i].end, text));
  }

  for(thread& t: tids) {
    t.join();
  }


  end = std::chrono::system_clock::now();
  cout << "c++ threads  " <<
    chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " " ;



  // cilk: parallel for  (you need -fcilkplus) 
  text = original;
  start = std::chrono::system_clock::now();

  __cilkrts_set_param("nworkers",argv[2]);
  {
    cilk_for(int i=0; i<text.size() ; i++ ) {
      text[i] = traduttore(text[i]);
    } 
  }
  end = std::chrono::system_clock::now();
  cout << "cilk_for " <<
    chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " " ;

  // grppi
  text = original;
  start = std::chrono::system_clock::now();
  auto thr = grppi::parallel_execution_native{nw};
  auto omp = grppi::parallel_execution_omp{};
  //omp.set_concurrency_degree(nw);
  auto seq = grppi::sequential_execution{};

  grppi::map(seq,
	     text.begin(), text.end(), text.begin(),
	     traduttore);
  
  end = std::chrono::system_clock::now();
  cout << "grppi map thr " <<
    chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " " ;
  
  cout << "nw " << nw << 	endl;
  // cout << text << endl;

  return 0;
}
