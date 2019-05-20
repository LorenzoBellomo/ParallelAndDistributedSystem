 
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
 * Date:   November 2019
 */

//
// Toy example to test FastFlow farm with feedback.
// It finds the prime numbers in the range (n1,n2) using a task-ffarm pattern plus
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
#include <string>
#include <vector>
#include <iostream>
#include <ff/ff.hpp>
#include <ff/all2all.hpp>
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

struct Task_t {
    Task_t(ull n1, ull n2):n1(n1),n2(n2) {}
    const ull n1, n2;
};

// generates the numbers
struct L_Worker: ff_monode_t<Task_t> {

    L_Worker(ull n1, ull n2)
        : n1(n1),n2(n2) {}

    Task_t *svc(Task_t *) {

	const int nw = 	get_num_outchannels();
	const size_t  size = (n2 - n1) / nw;
	ssize_t more = (n2-n1) % nw;
	ull start = n1, stop = n1;
	
	for(int i=0; i<nw; ++i) {
	    start = stop;
	    stop  = start + size + (more>0 ? 1:0);
	    --more;
	    
	    Task_t *task = new Task_t(start, stop);
	    ff_send_out_to(task, i);
	}
	return EOS;
    }
    ull n1,n2; 
};
struct R_Worker: ff_minode_t<Task_t> {
    Task_t *svc(Task_t *in) {
	ull   n1 = in->n1, n2 = in->n2;
	ull  prime;
	while( (prime=n1++) < n2 )  if (is_prime(prime)) results.push_back(prime);
	//std::cout << "Worker" << get_my_id() << " found " << V.size() << " primes\n";
        return GO_ON;
    }
    void svc_end() {
	std::sort(results.begin(), results.end());

#if 0	
	printf("Worker%d:\n", get_my_id());
	for(size_t i=0;i<results.size();++i)
	    printf("%lld " , results[i]);
	printf("\n");
#endif
    }
    std::vector<ull> results;
};

int main(int argc, char *argv[]) {    
    if (argc<5) {
        std::cerr << "use: " << argv[0]  << " number1 number2 Lworkers RWorkers [print=off|on]\n";
        return -1;
    }
    ull n1          = std::stoll(argv[1]);
    ull n2          = std::stoll(argv[2]);
    const size_t Lw = std::stol(argv[3]);
    const size_t Rw = std::stol(argv[4]);
    bool print_primes = false;
    if (argc >= 6)  print_primes = (std::string(argv[5]) == "on");

    ffTime(START_TIME);

    const size_t  size = (n2 - n1) / Lw;
    ssize_t more = (n2-n1) % Lw;
    ull start = n1, stop = n1;

    std::vector<ff_node*> LW;
    std::vector<ff_node*> RW;
    for(size_t i=0; i<Lw; ++i) {
	start = stop;
	stop  = start + size + (more>0 ? 1:0);
	--more;
	LW.push_back(new L_Worker(start, stop));
    }
    for(size_t i=0;i<Rw;++i)
	RW.push_back(new R_Worker);

    ff_a2a a2a;
    a2a.add_firstset(LW);
    a2a.add_secondset(RW);
    
    if (a2a.run_and_wait_end()<0) {
	error("running a2a\n");
	return -1;
    }

    std::vector<ull> results;
    results.reserve( (size_t)(n2-n1)/log(n1) );
    for(size_t i=0;i<Rw;++i) {
	R_Worker* r = reinterpret_cast<R_Worker*>(RW[i]);
	if (r->results.size())  
            results.insert(std::upper_bound(results.begin(), results.end(), r->results[0]),
			   r->results.begin(), r->results.end());
    }
    ffTime(STOP_TIME);
    
    // printing obtained results
    const size_t n = results.size();
    std::cout << "Found " << n << " primes\n";
    if (print_primes) {
	for(size_t i=0;i<n;++i) 
	    std::cout << results[i] << " ";
	std::cout << "\n\n";
    }
    std::cout << "Time: " << ffTime(GET_TIME) << " (ms)\n";
    std::cout << "A2A Time: " << a2a.ffTime() << " (ms)\n";

    return 0;
}
