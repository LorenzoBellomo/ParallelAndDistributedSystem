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


struct Emitter: ff_monode_t<float> {
    int svc_init() {
        active = get_num_outchannels();
        return 0;
    }
    float* svc(float* in) {
#if defined(THROTTLING)
        const unsigned nw = get_num_outchannels();

        if (nw == 1) return in;
        switch(cnt) {
        case 100:  {
            std::cout << "REMOVING WORKER " << nw-1 << "\n";
            ff_send_out_to(GO_OUT, nw-1);
            --active;
        }  break;
        case 1000: {
            if (nw>2) {
                std::cout << "REMOVING WORKER " << nw-2 << "\n";
                ff_send_out_to(GO_OUT, nw-2);
                --active;
            }
        } break;
        case 5000: {
            if (nw > 2) {
                // to be sure the worker nw-2 went to sleep
                ff_monode::getlb()->wait_freezing(nw-2);
                std::cout << "RE-ADDING WORKER " << nw-2 << "\n";
                ff_monode::getlb()->thaw(nw-2, true);
                active++;
            }
        } break;
        case 10000: {
            // to be sure the worker nw-1 went to sleep
            ff_monode::getlb()->wait_freezing(nw-1);
            std::cout << "RE-ADDING WORKER " << nw-1 << "\n";
            ff_monode::getlb()->thaw(nw-1, true);
            active++;
        } break;
        default:;
        }
#endif
        ff_send_out_to(in, cnt % active);
        ++cnt;
        return GO_ON;
    }

    size_t cnt=0;
    unsigned active=0;
}; 

int main(int argc, char *argv[]) {    
    if (argc<3) {
        std::cerr << "use: " << argv[0]  << " nworkers stream-length\n";
        return -1;
    }
    const size_t nworkers = std::stol(argv[1]);

    firstStage  first(std::stol(argv[2]));
    thirdStage  third;

    ff_farm farm;
    std::vector<ff_node*> W;
    for(size_t i=0;i<nworkers;++i)
        W.push_back(new secondStage);
    farm.add_workers(W);
    Emitter E;   
    farm.add_emitter(&E);
    farm.add_collector(nullptr);
    farm.cleanup_workers();
    
    ff_pipeline pipe;
    pipe.add_stage(first);
    pipe.add_stage(farm);
    pipe.add_stage(third);

    ffTime(START_TIME);
    if (pipe.run_then_freeze()<0) {
        error("running pipe");
        return -1;
    }
    pipe.wait();
    
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << "\n";
#if defined(TRACE_FASTFLOW)
    pipe.ffStats(std::cout);
#endif    
    return 0;
}
