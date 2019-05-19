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



void printBoard(int n, int m, board_t img) {
  cout << "---------------------- Board -----------------" << endl;
  for(int i=0; i<n; i++) {
    for(int j=0; j<m; j++) {
      if(img(i,j,0,0) == alive)
	cout << "O" ;
      else
	cout << " ";
    }
    cout << endl; 
  }
  cout << "---------------------- Board -----------------" << endl;
  return;
}

void osc1(int i, int j, board_t & board) {
  for(int k=0; k<3; k++)
    board(i+k, j, 0, 0) = alive;
  return;
}

void nave1(int i, int j, board_t & board) {
    board(i,j+1,0,0)   = alive;
    board(i+1,j+2,0,0) = alive;
    board(i+2,j,0,0)   = alive;
    board(i+2,j+1,0,0) = alive;
    board(i+2,j+2,0,0) = alive; 
    return;
  };

void spship1(int i, int j, board_t & board) {
  for(int k=0; k<6; k++)
    board(i+k, j, 0, 0) = alive;
  board(i,j-1,0,0) = alive;
  board(i,j-2,0,0) = alive;
  board(i+1,j-3,0,0) = alive;
  board(i+3,j-4,0,0) = alive;
  board(i+4,j-4,0,0) = alive;
  board(i+5,j-3,0,0) = alive; 
  board(i+5,j-1,0,0) = alive; 
  return; 
}
unsigned char next(int i, int j, int n, int m, board_t& img) {
  int neighbourhood =
    (img((i-1+n)%n, (j-1+m)%m, 0, 0) == alive ? 1 : 0) +
    (img(i,         (j-1+m)%m, 0, 0) == alive ? 1 : 0) +
    (img((i+1)%n,   (j-1+m)%m, 0, 0) == alive ? 1 : 0) + 
    (img((i-1+n)%n,  j,        0, 0) == alive ? 1 : 0) +
    (img((i+1)%n,    j,        0, 0) == alive ? 1 : 0) +
    (img((i-1+n)%n, (j+1)%m,   0, 0) == alive ? 1 : 0) +
    (img(i,         (j+1)%m,   0, 0) == alive ? 1 : 0) +
    (img((i+1)%n,    j+1,      0, 0) == alive ? 1 : 0);

  if(img(i,j,0,0) == alive && neighbourhood<2)
    return dead;
  if(img(i,j,0,0) == alive && (neighbourhood==2 || neighbourhood==3))
    return alive;
  if(img(i,j,0,0) == alive && neighbourhood>3)
    return dead;
  if(img(i,j,0,0) == dead && neighbourhood==3)
    return alive;

  return(img(i,j,0,0));
}

int main(int argc, char * argv[]) {

  if(argc == 1) {
    cout << "Usage is: life n m seed iter init-no nw" << endl;
    return(0);
  }
  // get matrix dimensions from the command line
  int n = atoi(argv[1]);
  int m = atoi(argv[2]);
  int seed = atoi(argv[3]);
  int iter = atoi(argv[4]);
  int init = atoi(argv[5]);
  int nw   = atoi(argv[6]);
  
  cout << "Using " << n << "x" << m << " board " << endl;
  
  // create an empty board
  board_t board(n,m,1,1,0), board1(n,m,1,1,0);

  // initialize it // randomly
  srand(seed);
  {
    utimer t("init empty");
    for(int i=0; i<n; i++)
      for(int j=0;  j<m; j++)
	board(i,j,0,0) = dead; // (rand() % 2 == 0 ? dead : alive);
  }
    
  // k oscillators
  for(int k=0; k<init/2; k++) {
    int i = rand()%n;
    int j = rand()%m;
    // cout << "Adding osc1 at " << i << " " << j << endl; 
    osc1(i,j, board);
  }

  // k spaceships
  for(int k=0; k<init/2; k++) {
    int i = rand()%n;
    int j = rand()%m;
    // cout << "Adding nave1 at " << i << " " << j << endl; 
    nave1(i,j, board);
  }
  
  // k spaceships
  for(int k=0; k<init/4; k++) {
    int i = rand()%n;
    int j = rand()%m;
    // cout << "Adding nave1 at " << i << " " << j << endl; 
    spship1(i,j, board);
  }

  board_t * img1, *img2;
  CImgDisplay main_displ(board,"Init");

  // here is the main loop, using two images swapped at each iteration
  img1 = &board;
  img2 = &board1;

  for(int i=0; i<iter; i++) {
    {
      // first compute the new status
      {
	utimer t("one step"); 
#pragma omp parallel for num_threads(nw)
	for(int i=0; i<n; i++) {
	  for(int j=0; j<m; j++) {
	    (*img2)(i,j,0,0) = next(i,j,n,m,(*img1));
	  }
	}
      }
      // then display it 
      // printBoard(n,m,*img2);

      //if(i%16 == 0)
      {
	utimer("display"); 
	main_displ.display(*img2);
      }

      // eventually swap the source and dest images
      {
	utimer ("swap");
	auto temp = img1;
	img1 = img2;
	img2 = temp;
      }
    }
  }
  return(0);
  
}
