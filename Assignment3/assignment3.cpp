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



int main(int argc, char * argv[]) {

	if(argc < 4 || argc > 5) {
		cout << "Usage is: life n iter-num nw (seed)" << endl;
		return(0);
	}
	// get matrix dimensions from the command line
	int n = atoi(argv[1]);
	int iter_num = atoi(argv[2]);
	int nw = atoi(argv[3]);
	(argc == 5)? srand(atoi(argv[4])) : srand(time(NULL));

	cout << "Using " << n << "x" << n << " board " << endl;

	/*    
	Qualsiasi cella viva con meno di due celle vive adiacenti muore, come per effetto d'isolamento;
	Qualsiasi cella viva con due o tre celle vive adiacenti sopravvive alla generazione successiva;
	Qualsiasi cella viva con più di tre celle vive adiacenti muore, come per effetto di sovrappopolazione;
	Qualsiasi cella morta con esattamente tre celle vive adiacenti diventa una cella viva, come per effetto di riproduzione.
	*/

	// create an empty board
	board_t board(n,n,1,1,0);

	for(int i=0; i<n; i++)
		for(int j=0;  j<n; j++)
			board(i,j,0,0) = (rand() % 32 == 0 ? alive : dead);

	CImgDisplay main_displ(board,"Init");
	sleep(2);  // show for two seconds

	for(int p = 0; p < iter_num; p++) {
		// change some individuals
		for(int k=0; k<n*n/100; k++) {
			int i = rand() % n;
			int j = rand() % n;
			if(board(i,j,0,0) == alive)
				board(i,j,0,0) = dead;
			else
				board(i,j,0,0) = alive;
		}
		// and redisplay
		main_displ.display(board);
		sleep(1);
	}
	return(0);
  
}
