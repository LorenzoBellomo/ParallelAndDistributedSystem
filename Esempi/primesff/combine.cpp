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
 *
 *
 */
/* Author: Massimo Torquati
 *
 */
// basic composition of sequential stages

#include <iostream>
#include <ff/ff.hpp>
using namespace ff;

long ntasks = 100000;
long worktime = 1*2400; 

struct firstStage: ff_node_t<long> {
    long *svc(long*) {
        for(long i=1;i<=ntasks;++i) {
            ff_send_out((long*)i);
        }
        return EOS; // End-Of-Stream
    }
};
struct secondStage: ff_node_t<long> { // 2nd stage
    long *svc(long *t) {
        ticks_wait(worktime);
        return t;
    }
};
struct secondStage2: ff_node_t<long> { // 2nd stage
    long *svc(long *t) {
        ticks_wait(worktime);
        return t;
    }
};
struct secondStage3: ff_node_t<long> { // 2nd stage
    long *svc(long *t) {
        ticks_wait(worktime);
        return t;
    }
};
struct thirdStage: ff_node_t<long> {  // 3rd stage
    long *svc(long *task) {
        ticks_wait(worktime);
        return GO_ON; 
    }
}; 
int main() {
    firstStage    _1;
    secondStage   _2;
    secondStage2  _3; 
    secondStage3  _4;
    thirdStage    _5;
    
    unsigned long start=getusec();
    long *r;
    for(long i=1;i<=ntasks;++i) {
        r = _2.svc((long*)i);
        r = _3.svc(r);
        r = _4.svc(r);
        r = _5.svc(r);
    }
    unsigned long stop=getusec();
    std::cout << "TEST  FOR  Time = " << (stop-start) / 1000.0 << " ms\n";

    ff_pipeline pipe;  
    pipe.add_stage(_1);
    pipe.add_stage(_2);
    pipe.add_stage(_3);
    pipe.add_stage(_4);
    pipe.add_stage(_5);
    pipe.run_and_wait_end();
    std::cout << "TEST  PIPE Time = " << pipe.ffwTime() << " ms\n";

    ff_pipeline pipe2;
    auto comb = combine_nodes(_2, combine_nodes(_3, _4));
    pipe2.add_stage(_1);
    pipe2.add_stage(comb);
    pipe2.add_stage(_5);
    pipe2.run_and_wait_end();
    std::cout << "TEST  PIPE2 Time = " << pipe2.ffwTime() << " ms\n";


    ff_pipeline pipe3;
    pipe3.add_stage(_1);
    pipe3.add_stage(new ff_comb(new secondStage, new secondStage2, true, true), true);
    pipe3.add_stage(new ff_comb(new secondStage3, new thirdStage, true, true), true);
    pipe3.run_and_wait_end();
    std::cout << "TEST  PIPE3 Time = " << pipe3.ffwTime() << " ms\n";

    return 0;
}
