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
 *
 */

#include <iostream>
#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>

using namespace ff;

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
        float &t = *task; 
        t = t*t;

        ssize_t myid = get_my_id();
        switch(myid) {
        case 0:
        case 1:
            for(volatile long k=0; k<t;++k);
        default:;
        };

        return task; 
    }
};
struct thirdStage: ff_node_t<float> {
    float* svc(float * task) { 
        float &t = *task;
        //std::cout<< "thirdStage received " << t << "\n";
        sum += t; 
        delete task;
        return GO_ON; 
    }
    void svc_end() { std::cout << "sum = " << sum << "\n"; }
    float sum = 0.0;
};

int main(int argc, char *argv[]) {    
    if (argc<3) {
        std::cerr << "use: " << argv[0]  << " nworkers stream-length\n";
        return -1;
    }
    const size_t nworkers = std::stol(argv[1]);

    firstStage  first(std::stol(argv[2]));
    thirdStage  third;
    std::vector<std::unique_ptr<ff_node> > W;
    for(size_t i=0;i<nworkers;++i) W.push_back(make_unique<secondStage>());

    ff_OFarm<float> farm(std::move(W));  

#if 0
    farm.set_scheduling_ondemand();  
#endif
    
    ff_Pipe<> pipe(first, farm, third);

    ffTime(START_TIME);
    if (pipe.run_and_wait_end()<0) {
        error("running pipe");
        return -1;
    }
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << "\n";
    pipe.ffStats(std::cout);

    return 0;
}
