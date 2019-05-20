 
/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  As a special exception, you may use this file as part of a free software
 *  library without restriction.  Specifically, if other files instantiate
 *  templates or use macros or inline functions from this file, or you compile
 *  this file and link it with other files to produce an executable, this
 *  file does not by itself cause the resulting executable to be covered by
 *  the GNU General Public License.  This exception does not however
 *  invalidate any other reasons why the executable file might be covered by
 *  the GNU General Public License.
 *
 ****************************************************************************
 */
/* 
 * Author: Massimo Torquati <torquati@di.unipi.it> 
 * Date:   November 2014
 */

//
// Toy example to test FastFlow farm with feedback.
// It finds the prime numbers in the range (n1,n2) using a task-farm pattern plus
// the feedback modifier.
//
//                   |----> Worker --
//         Emitter --|----> Worker --|--
//           ^       |----> Worker --   |
//           |__________________________|
//
//  -   The Emitter generates all number between n1 and n2 and then waits 
//      to receive primes from Workers.
//  -   Each Worker checks if the number is prime (by using the is_prime function), 
//      if so then it sends the prime number back to the Emitter.
//

#include <cmath>
#include <vector>
#include <iostream>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
using namespace ff;

using ull = unsigned long long;

// see http://en.wikipedia.org/wiki/Primality_test
static inline bool is_prime(ull n) {
    if (n <= 3)  return n > 1; // 1 is not prime !
    
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (ull i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) 
            return false;
    }
    return true;
}

// generates the numbers
struct Emitter: ff_node_t<ull> {

    Emitter(ff_loadbalancer *const lb, 
            ull n1, ull n2)
        :lb(lb),n1(n1),n2(n2) {
	results.reserve( (size_t)(n2-n1)/log(n1) );
    }

    ull *svc(ull *x) {
        if (x == nullptr) { // first time
            ull n;
	    //
	    // NOTE: we assume here that sizeof(unsigned long long ) == sizeof(void*)
	    //
            while( (n=n1++) <= n2 ) ff_send_out( (void*)n);
	    
	    // broadcasting the End-Of-Stream to all workers
            lb->broadcast_task(EOS);
	    
	    // keep me alive 
            return GO_ON;
        }
	results.push_back((ull)x);
        return GO_ON;
    }
    ff_loadbalancer *const lb;
    ull n1,n2; // these are range's edge values
    std::vector<ull> results;
};
struct Worker: ff_node_t<ull> {
    ull *svc(ull *in) {
        if (is_prime(reinterpret_cast<ull>(in))) return in;
        return GO_ON;
    }
};

int main(int argc, char *argv[]) {    
    if (sizeof(unsigned long long) > sizeof(void*)) abort();

    if (argc<4) {
        std::cerr << "use: " << argv[0]  << " number1 number2 nworkers [print=off|on]\n";
        return -1;
    }
    ull n1          = std::stoll(argv[1]);
    ull n2          = std::stoll(argv[2]);
    const size_t nw = std::stol(argv[3]);
    bool print_primes = false;
    if (argc >= 5)  print_primes = (std::string(argv[4]) == "on");


    ffTime(START_TIME);
    ff_Farm<> farm( [&]() {
	    std::vector<std::unique_ptr<ff_node> > W;
	    for(size_t i=0;i<nw;++i)
		W.push_back(make_unique<Worker>());
	    return W;
	} () ); // by default it has both an emitter and a collector

    Emitter E(farm.getlb(), n1, n2);

    farm.add_emitter(E);      // replacing the default emitter
    farm.remove_collector();  // removing the default collector
    farm.wrap_around();       // adds feedback channel between worker and emitter

    if (farm.run_and_wait_end()<0) {
	error("running farm\n");
	return -1;
    }
	
    // printing obtained results
    const std::vector<ull> &results = E.results;
    const size_t n = results.size();
    std::cout << "Found " << n << " primes\n";
    if (print_primes) {
	for(size_t i=0;i<n;++i) 
	    std::cout << results[i] << " ";
	std::cout << "\n\n";
    }
    std::cout << "Time: " << ffTime(STOP_TIME) << " (ms)\n";
    std::cout << "farm Time: " << farm.ffTime() << " (ms)\n";
    return 0;
}
