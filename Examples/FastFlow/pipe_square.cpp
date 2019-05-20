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
 * Date:   November 2016
 */

#include <math.h>
#include <iostream>
#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
using namespace ff;

static void doWork(float& t) {
    for(volatile int i=0;i<1000; ++i) t+=sin(t)/(2*cos(t));
}

struct firstStage: ff_node_t<float> {
    firstStage(const size_t length):length(length) {}
    float* svc(float *) {
        for(size_t i=0; i<length; ++i) {
            ff_send_out(new float(i));
        }
        return EOS;
    }
    const size_t length;
};
struct secondStage: ff_node_t<float> {
    float* svc(float * task) { 
        auto &t = *task; 

        //doWork(t);        
        t = t*t;
        
        return task; 
    }
};
struct thirdStage: ff_node_t<float> {   
    float* svc(float * task) { 
        auto &t = *task;
        sum += t; 
        delete task;
        return GO_ON; 
    }
    void svc_end() { std::cout << "sum = " << sum << "\n"; }
    float sum = 0.0;
};

int main(int argc, char *argv[]) {    
    if (argc<2) {
        std::cerr << "use: " << argv[0]  << " stream-length\n";
        return -1;
    }
    
    firstStage  first(std::stol(argv[1]));
    secondStage second;
    thirdStage  third;
    ff_Pipe<float> pipe(first, second, third);
    if (pipe.run_and_wait_end()<0) {
        error("running pipe");
        return -1;
    }
    std::cout << "Time: " << pipe.ffTime() << "\n";
    pipe.ffStats(std::cout);
    return 0;
}
