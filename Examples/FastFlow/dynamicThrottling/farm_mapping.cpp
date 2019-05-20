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
#include <ff/farm.hpp>

using namespace ff;

#if defined(EXPLICIT_MAPPING)
static int emitter_core = 3;
const std::vector<int> worker_mapping{0,1,2};
#endif

struct Worker: ff_node_t<long> {
    int svc_init() {
#if defined(EXPLICIT_MAPPING)
        if (ff_mapThreadToCpu(worker_mapping[get_my_id() % worker_mapping.size()])<0)
            error("mapping Emitter");
#endif
        
        printf("I'm Worker%ld running on the core %ld\n", get_my_id(), ff_getMyCore());
        return 0;
    }
    long* svc(long * in) { 
        return in; 
    }
};

struct Emitter: ff_node_t<long> {
    int svc_init() {
#if defined(EXPLICIT_MAPPING)
        if (ff_mapThreadToCpu(emitter_core)<0)
            error("mapping Emitter");
#endif
        
        printf("I'm the Emitter running on the core %ld\n", ff_getMyCore());
        
        return 0;
    }
    long* svc(long* in) {
        if (in == nullptr) {
            for(size_t i=1;i<=100;++i)
                ff_send_out((long*)i);
            return EOS;
        }
        return GO_ON;
    }
}; 

int main(int argc, char *argv[]) {    
    if (argc<2) {
        std::cerr << "use: " << argv[0]  << " nworkers\n";
        return -1;
    }
    const size_t nworkers = std::stol(argv[1]);

    ff_farm farm;
    std::vector<ff_node*> W;
    for(size_t i=0;i<nworkers;++i)
        W.push_back(new Worker);
    farm.add_workers(W);
    Emitter E;   
    farm.add_emitter(&E);
    farm.wrap_around();
    farm.cleanup_workers();    

#if defined(EXPLICIT_MAPPING)    
    farm.no_mapping();
#else    
    const std::string worker_mapping{"3, 0, 1, 2"};
    threadMapper::instance()->setMappingList(worker_mapping.c_str());
#endif
    
    if (farm.run_and_wait_end()<0) {
        error("running farm");
        return -1;
    }

    return 0;
}
