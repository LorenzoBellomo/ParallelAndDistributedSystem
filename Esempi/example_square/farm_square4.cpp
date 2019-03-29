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

struct firstThirdStage: ff_node_t<float> {
    firstThirdStage(const size_t length):length(length) {}
    float* svc(float *task) {
        if (task == nullptr) {
            for(size_t i=0; i<length; ++i) {
                ff_send_out(new float(i));
            }
            return GO_ON;
        }        
        float &t = *task;
        std::cout<< "thirdStage received " << t << "\n";
        sum += t; 
        delete task;
        if (++ntasks == length) return EOS;
        return GO_ON;         
    }
    void svc_end() { std::cout << "sum = " << sum << "\n"; }

    const size_t length;
    size_t ntasks=0;
    float sum = 0.0;
};

struct secondStage: ff_node_t<float> {
    float* svc(float * task) { 
        float &t = *task; 
        std::cout<< "secondStage received " << t << "\n";
        t = t*t;
        return task; 
    }
};

int main(int argc, char *argv[]) {    
    if (argc<3) {
        std::cerr << "use: " << argv[0]  << " nworkers stream-length\n";
        return -1;
    }
    const size_t nworkers = std::stol(argv[1]);
    firstThirdStage  firstthird(std::stol(argv[2]));

    std::vector<std::unique_ptr<ff_node> > W;
    for(size_t i=0;i<nworkers;++i) W.push_back(make_unique<secondStage>());

    ff_Farm<float> farm(std::move(W), firstthird);


    farm.remove_collector(); // needed because the collector is present by default
    farm.wrap_around();
    //farm.set_scheduling_ondemand(); // optional


    ffTime(START_TIME);
    if (farm.run_and_wait_end()<0) {
        error("running farm");
        return -1;
    }
    ffTime(STOP_TIME);
    std::cout << "Time: " << ffTime(GET_TIME) << "\n";
    return 0;
}
