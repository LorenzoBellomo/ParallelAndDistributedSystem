#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
using namespace ff;

struct Stage_0: ff_node_t<long>{
    long* svc(long*) {
	for(long i=1;i<=100;++i)
	    ff_send_out((long*)i);

	//sleep(100);
	
	return EOS;
    }
};

struct Stage_A: ff_minode_t<long> {   // multi-input node
    long* svc(long* in) {
	if (fromInput()) {
	    ff_send_out(in);
	    ++onthefly;
	} else
	    if ((--onthefly<=0) && eosreceived) return EOS;
	return GO_ON;
    }
    void eosnotify(ssize_t) {
	eosreceived=true;
	
	if (onthefly==0) ff_send_out(EOS);
	
    }
    
    bool eosreceived=false;
    size_t onthefly=0;
};
struct Stage_B: ff_monode_t<long> {
    long* svc(long* in) {
	//usleep((long)in);
	ff_send_out_to(in, 0);
	ff_send_out_to(in, 1);
	return GO_ON;
    }
};
struct Stage_1: ff_node_t<long>{
    long* svc(long* in) {
	printf("S_1 received %ld\n", (long)in);
	return GO_ON;
    }
};

int main() {
    Stage_0 S_0;
    Stage_1 S_1;
    Stage_A S_A;
    Stage_B S_B;
    ff_Pipe<> pipeIn(S_A, S_B);
    pipeIn.wrap_around();    // feedback 
    ff_Pipe<> pipeOut(S_0, pipeIn, S_1);
    if (pipeOut.run_and_wait_end()<0)
	error("running pipeOut\n");
    return 0;
}


	
