#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

#include <utimer.cpp>

#include <CImg.h>
using namespace std;
using namespace cimg_library;

// notable constants and types 
using board_t = cimg_library::CImg<unsigned char>;
const unsigned int alive = 000;
const unsigned int dead  = 255;

board_t board;
int current_iteration = 0, n, iter_num, nw;

void thd_func(int id, int left_limit, int right_limit) {

	for(int i = left_limit; i < right_limit; i++){
		for(int j = 0; j < n; j++) {

		}
	}
	
}


int main(int argc, char * argv[]) {

	if(argc < 4 || argc > 5) {
		cout << "Usage is: life n iter-num nw (seed)" << endl;
		return(0);
	}
	// get matrix dimensions from the command line
	n = atoi(argv[1]);
	iter_num = atoi(argv[2]);
	nw = atoi(argv[3]);
	(argc == 5)? srand(atoi(argv[4])) : srand(time(NULL));

	cout << "Using " << n << "x" << n << " board " << endl;

	/*    
	Qualsiasi cella viva con meno di due celle vive adiacenti muore, come per effetto d'isolamento;
	Qualsiasi cella viva con due o tre celle vive adiacenti sopravvive alla generazione successiva;
	Qualsiasi cella viva con più di tre celle vive adiacenti muore, come per effetto di sovrappopolazione;
	Qualsiasi cella morta con esattamente tre celle vive adiacenti diventa una cella viva, come per effetto di riproduzione.
	*/

	// create an empty board
	board(n,n,1,1,0);
	vector<thread> threadPool;

	for(int i=0; i<n; i++)
		for(int j=0;  j<n; j++)
			board(i,j,0,0) = (rand() % 32 == 0 ? alive : dead);

	CImgDisplay main_displ(board,"Game of Life");
	sleep(2);  // show for two seconds

	int range = num_tasks / nw;
    int extra = num_tasks % nw;
	int prev = 0;

	for(int i = 0; i < extra; i++) {
		threads.push_back(thread(
			thd_func, 
			prev,
			prev + range + 1,
		));
		prev += (range + 1);
	}
        
	for(int i = 0; i < nw - extra; i++) {            
		threads.push_back(thread(
			thd_func, 
			prev,
			prev + range,
		));
		prev += range;
	}

	for(thread& t: threadPool)
        t.join();

	return(0);
  
}
