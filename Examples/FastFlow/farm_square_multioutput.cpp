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

#include <map>
#include <iostream>
#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
using namespace ff;

using mypair_t = std::pair<size_t,float*>;

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

// multi-output node
struct secondStage: ff_monode_t<mypair_t> {
    mypair_t* svc(mypair_t * task) {
        float &t = *(task->second);
        t = t*t;
        ssize_t myid = get_my_id();
        switch(myid) {
        case 0:
        case 1:
            for(volatile long k=0; k<t;++k);
        default:;
        };
        ff_send_out_to(task, 0);  // to the Emitter
        ff_send_out_to(task, 1);   // to the next stage
        return GO_ON;
    }
};

// multi-input node
struct thirdStage: ff_minode_t<mypair_t, float > {
    thirdStage(const size_t nworkers):nworkers(nworkers) {}
        
    float* svc(mypair_t* in) {
        if (counter == in->first) { // it's the next to send out
            counter++;
            float &t = *(in->second);            
            delete in;

            sum += t; 
            
            std::map<size_t,mypair_t*>::iterator b = M.begin(), e = M.end();
            while(b != e) {
                if (b->first==counter) {

                    sum += *(b->second->second);
                    delete b->second;
                    counter++;
                    b = M.erase(b);
                } else break;
            }
            return GO_ON;
        }
        M[in->first] = in;
        return GO_ON;
    }
    
    void svc_end() {
        // check if something is still in the internal state!
        std::map<size_t,mypair_t*>::iterator b = M.begin(), e = M.end();
        while(b != e) {
            if (b->first==counter) {
                sum += *(b->second->second);
                delete b->second;
                counter++;
                b = M.erase(b);
            } else abort(); // something went wrong!
        }
        std::cout << "sum = " << sum << "\n";
    }


    size_t  nworkers;
    size_t  neos = 0;
    size_t counter = 0;
    std::map<size_t, mypair_t*> M;  // internal state used for keeping the ordering of data

    float sum = 0.0;
};



struct Emitter: ff_node_t<float, mypair_t> {
    Emitter(ff_loadbalancer *lb):lb(lb) {}

    int svc_init() {
        last = lb->getNWorkers();
        ready.resize(lb->getNWorkers());
        for(size_t i=0; i<ready.size(); ++i) ready[i] = true;
        nready=ready.size();

        return 0;
    }
    
    mypair_t* svc(float *in) {
        int wid = lb->get_channel_id();
        if (wid == -1) { // task coming from the first stage, if wid>=0 then it comes from a worker 
            
            mypair_t* p = new mypair_t;
            p->first = counter;
            counter++;
            p->second = in;

            int victim = selectReadyWorker(); // get a ready worker
            if (victim<0) data.push_back(p);
            else {
                lb->ff_send_out_to(p, victim);
                ready[victim]=false;
                --nready;
            }
            return GO_ON;            
        }
        assert(ready[wid] == false);
        ready[wid] = true;
        ++nready;
        if (data.size()>0) {
            lb->ff_send_out_to(data.back(), wid);
            data.pop_back();
            ready[wid]=false;
            --nready;
        } else
            if (eos_received && nready == ready.size()) return EOS;
        return GO_ON;
    }
    void svc_end() {
        // just for debugging
        assert(data.size()==0);
    }
    void eosnotify(ssize_t id) {
        if (id == -1) { // we have to receive all EOS from the previous stage
            // EOS is coming from the input channel
            
            eos_received=true; 
            if (eos_received              && 
                nready == ready.size()    &&
                data.size() == 0) {
                lb->broadcast_task(EOS);
            }
        }
    }
    
    int selectReadyWorker() {
        for (unsigned i=last+1;i<ready.size();++i) {
            if (ready[i]) {
                last = i;
                return i;
            }
        }
        for (unsigned i=0;i<=last;++i) {
            if (ready[i]) {
                last = i;
                return i;
            }
        }
        return -1;
    }

    bool  eos_received = false;
    size_t counter=0;    
    unsigned last;
    unsigned nready;
    ff_loadbalancer *lb;
    std::vector<bool> ready;
    std::vector<mypair_t*> data;
};


int main(int argc, char *argv[]) {    
    if (argc<3) {
        std::cerr << "use: " << argv[0]  << " nworkers stream-length\n";
        return -1;
    }
    const size_t nworkers = std::stol(argv[1]);

    firstStage  first(std::stol(argv[2]));
    thirdStage  third(nworkers);     // this is a multi-input node
    std::vector<std::unique_ptr<ff_node> > W;
    // workers are multi-output nodes
    for(size_t i=0;i<nworkers;++i) W.push_back(make_unique<secondStage>());

    ff_Farm<float> farm(std::move(W));
    // the Emitter needs the internal load-balancer
    Emitter e(farm.getlb());  
    farm.add_emitter(e);
    farm.remove_collector();  // removing the default collector
    farm.wrap_around();       // creating a feedback channel between workers and emitter

    ff_Pipe<> pipe(first, farm, third);  // build the pipeline

    ffTime(START_TIME);
    if (pipe.run_and_wait_end()<0) {
        error("running pipe");
        return -1;
    }
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << "\n";

    return 0;
}
