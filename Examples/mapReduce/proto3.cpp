#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <map>
#include <iostream>
#include <experimental/filesystem>
#include <thread>
#include <optional>

#include "util.cpp"

// #define DEBUG
// #define TIMEREP
///
/// g++ -std=c++17 proto.cpp -o proto  -lstdc++fs
/// 

using namespace std; 

template<typename Tin,     // type of the inputs to map function
	 typename Tkey,    // type of the keys
	 typename Tvalue>  // type of the values
class MapReduce {

private:
  function<vector<pair<Tkey,Tvalue>>(Tin)> f;   // map function
  function<Tvalue(Tvalue,Tvalue)> oplus;        // reduce function
  vector<Tin> inv;                              // the file (in memory) to process
  int nmap;                                     // the number of mapper threads
  int nred;                                     // the number of reducer threads
  int pardegree;                                // overall parallelism degree

public:
  map<Tkey,Tvalue> results;                     // final results

  MapReduce(vector<Tin> inv):                   // constructor: accepts the input vector
    inv(inv),nmap(0),nred(0),pardegree(0)
  {}
  
  MapReduce(vector<Tin> inv,                    // constructors: all needed parameters given
	    vector<pair<Tkey,Tvalue>> f(Tin),
	    Tvalue oplus(Tvalue,Tvalue)):
    inv(inv),f(f),oplus(oplus),
    nmap(0),nred(0),pardegree(0)
  {}
  
  MapReduce(vector<Tin> inv,                    // constructors: all needed parameters given
	    vector<pair<Tkey,Tvalue>> f(Tin),
	    Tvalue oplus(Tvalue,Tvalue),
	    int nmap, int nred):
    inv(inv),f(f),oplus(oplus),
    nmap(nmap),nred(nred),pardegree(nmap+nred)
  {}
  
  void setMapper(vector<pair<Tkey,Tvalue>> ff(Tin)) {     // sets the mapper function
    f = ff;
    return;
  }

  void setReducer(Tvalue foplus(Tvalue,Tvalue)) {         // sets the reducer function
    oplus = foplus;
    return;
  }

  void setPardegree(int m, int r) {                      // sets parallelism degree(s)
    nmap = m;
    nred = r;
    pardegree = m+r;
    return;
  }

  void compute() {                                       // starts sequential or parallel 
    if(pardegree == 0)                                   // computation depending on pardegree
      seqCompute();
    else
      parCompute();
    return;
  }
  
  void seqCompute() {                           // sequential computation
    vector<pair<Tkey, Tvalue>> mapresult;       // result vector
    for(auto &it : inv) {                       // map phase: scan the input
      auto v = f(it);                           // processing each item with the mapper function
      for(auto &it : v)                         // add pairs to the result vector
	mapresult.push_back(it);
    }
    for(auto &w : mapresult) {                  // reduce phase: process all pairs 
      results[w.first] = oplus(results[w.first],w.second);  // sum up with previous (0 if none)
    }
    return; 
  }

  void parCompute() {
    cout << "ParCompute called" << endl; 
    std::vector<std::pair<int,int>> chunks;     // compute the chunck sizes
    int size = (inv.size() / nmap);
    for(int i=0; i<nmap; i++) {
      if(i != (nmap-1)) {
  	chunks.push_back(make_pair(i*size, (i+1)*size-1));   // regular chunck
      } else {                                               // last one is longer 
  	chunks.push_back(make_pair(i*size, inv.size()));     // sospetto fine riga -1
      }
    }
    std::vector<queue<pair<Tkey, Tvalue>>> redq(nred);       // create queues to reducers

    auto mapworker = [&] (int c) {             // this is the map worker code
      map<Tkey,Tvalue> mapresult;              // declare the map for the result
      
      for(int i=chunks[c].first; i<chunks[c].second; i++) {  // compute the map
  	auto v = f(inv[i]);                    // apply map
  	for(auto &it : v)                      // insert pairs into the result
  	  mapresult[it.first] = oplus(mapresult[it.first],it.second);
      }
      for(auto &it : mapresult) {              // once completed, deliver pairs to proper 
  	int destreducer = hash<Tkey>()(it.first) % nred; // reducer: use hash to pick up index
  	redq[destreducer].push(it);            // then send it the pair 
      }
      return;                                  // done, everything to reducers
    };

    vector<map<Tkey,Tvalue>> redresults(nred);
    
    auto redworker = [&](int r) {              // the reduce worker 
      bool cont = true;                        // controls the otherwise infinite loop
      while(cont) {
  	auto po = (redq[r]).try_pop();         // pop a pair from the input queu
	
  	if(!po.has_value()) {                  // if there is nothing, means mappers closed
  	  cont = false;                        // and no more items in the queue
  	  break;                               // then stop
  	} 
  	auto p = po.value();                   // otherwise, get value and process it
  	(redresults[r])[p.first] = oplus((redresults[r])[p.first],p.second);
      }
      return;                                  // queue empty, then done!
    };

    vector<thread> mtids;                      // thread ids of the mappers
    vector<thread> rtids;                      // thread ids of the reducers

    for(int i=0; i<nred; i++)                  // create reducers: will block on empty queues
      rtids.push_back(thread(redworker,i));
    for(int i=0; i<nmap; i++)                  // create mappers: start filling queues
      mtids.push_back(thread(mapworker,i));

    for(int i=0; i<nmap; i++)                  // await mapper termination
      mtids[i].join();
    for(int i=0; i<nred; i++) {                // tell reducer input queues all producers ended
      redq[i].nomorewriters();                 // this possibily unblocks awaiting reducers
    }
    for(int i=0; i<nred; i++)                  // now await reducer termination
      rtids[i].join();

    auto power2 = [] (int n) {                 // compute if it's a power of two (and which one)
      int pow  = 0;
      int pow2 = 1;
      while(pow2 < n) {
	pow++;
	pow2 *= 2;
      }
      if(pow2 == n)
	return(make_pair(true,pow));
      else
	return(make_pair(false,0));
    };

    auto duealla = [] (int n) { return pow(2.0, ((float) n)); };

    auto lp = power2(nred);
    if(lp.first) {                            // n is a power of 2
      cout << "Parallel merge" << endl;
      // parallel merge of the maps computed by the reducers (binary tree)
      for(int l=0; l<lp.second; l++) {
	// execute step l of the binary tree
	  for(int j=0; j<nred; j++) {
	    if(j % ((int) duealla(l+1)) == 0) {
	      int dueallal = duealla(l);
	      cout << "Merging " << j+dueallal << " with " << j << endl;
	      // QUESTO NON SI PUÒ FARE COSÌ : NON FA IL MERGE DELLE CHIAVI UGUALI, OVVIAMENTE
	      for(auto &it : redresults[j+dueallal]) {
		(redresults[j])[it.first] += ((redresults[j+dueallal])[it.first]);
	      }
	    }
	  }
      }
      for(auto &it : redresults[0])
	cout << it.first << "," << it.second << endl; 
      results = redresults[0];
    } else {
      cout << "Sequential merge" << endl; 
      // sequential merge of the maps computed by the reducers, should be implemented in parallel
      for(int i=0; i<nred; i++)
	results.insert(redresults[i].begin(), redresults[i].end());
    }
    return;
    
  }
};
 

int main(int argc, char * argv[]){

  // parameters from command line
  string fnin  = argv[1];    // input file 
  char   mode  = argv[2][0]; // mode S P 
  int    nmap;
  int    nred;

  switch(mode) {
  case 'S': { nmap = 0; nred = 0; break; }
  case 'P': { nmap = atoi(argv[3]); nred = atoi(argv[4]); break; }
  default : { }
  }
  
  // read file in memory
  std::vector<string> lines;
  ifstream fdin(fnin);
  string line;
  while(getline(fdin, line)) {
    lines.push_back(line);
  }
  

  auto f = [] (string s) {
    vector<pair<string,int>> *v = new vector<pair<string,int>>();
    stringstream ss(s);
    string word; 
    while(ss >> word) {
      pair<string,int> p(word,1);
      v->push_back(p);
    } 
    return (*v);
  };

  auto oplus = [] (int i, int j) { return (i+j); };

  switch (mode) {
  case 'S': {
    MapReduce<string,string,int> mr(lines,f,oplus);
    auto start   = std::chrono::high_resolution_clock::now();
    mr.compute();
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    std::cerr << "Sequential time is " << msec << " msecs " << std::endl;
    // write results to disk
    string fnout = "seq.res";
    ofstream fdout(fnout);
    for(auto &it : mr.results) {
      fdout << it.first << " = " << it.second << endl; 
    }
    fdout.close();
    break;
  }
  case 'P': {
    MapReduce<string,string,int> mr(lines,f,oplus,nmap,nred);
    auto start   = std::chrono::high_resolution_clock::now();
    mr.compute();
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    std::cerr << "Parallel time is " << msec << " msecs with " << nmap << " mappers and "
	      << nred << " reducers (pardegree " << nmap + nred << " )" << std::endl;
    // write results to disk
    string fnout = "par.res";
    ofstream fdout(fnout);
    for(auto &it : mr.results) {
      fdout << it.first << " = " << it.second << endl; 
    }
    fdout.close();
    break;
  }
  default: {}
  }
  return(0);
}
