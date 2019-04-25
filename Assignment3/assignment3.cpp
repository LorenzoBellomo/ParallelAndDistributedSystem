#include <iostream>
#include <vector>

#include <utimer.cpp>

#include <CImg.h>
using namespace std;
using namespace cimg_library;

// notable constants and types 
using board_t = cimg_library::CImg<unsigned char>;
const unsigned int alive = 000;
const unsigned int dead  = 255;



int main(int argc, char * argv[]) {

  if(argc == 1) {
    cout << "Usage is: life n m seed iter init-no nw" << endl;
    return(0);
  }
  // get matrix dimensions from the command line
  int n = atoi(argv[1]);
  int m = atoi(argv[2]);
  int seed = atoi(argv[3]);

  cout << "Using " << n << "x" << m << " board " << endl;
  
  // create an empty board
  board_t board(n,m,1,1,0);

  // initialize it // randomly
  srand(seed);
  for(int i=0; i<n; i++)
    for(int j=0;  j<m; j++)
      board(i,j,0,0) = (rand() % 32 == 0 ? alive : dead);
  
  CImgDisplay main_displ(board,"Init");
  sleep(2);  // show for two seconds

  while(true) {
    // change some individuals
    for(int k=0; k<n*m/100; k++) {
      int i = rand() % n;
      int j = rand() % m;
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
