#include <iostream>
#include <vector>
#include <thread>
#include <atomic>

using namespace std;

#include <utimer.cpp>
#include <q.cpp>

#include <CImg.h>
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

  if(n % nw !=0) {
    cerr << "rows in image not multiple of the pardegree" << endl;
    return(-1);
  }
  
  cout << "Using " << n << "x" << m << " board " << endl;
  
  // create an empty board
  board_t board(n,m,1,1,0), board1(n,m,1,1,0);

  // initialize it // randomly
  srand(seed);
  for(int i=0; i<n; i++)
    for(int j=0;  j<m; j++)
      board(i,j,0,0) = dead; // (rand() % 2 == 0 ? dead : alive);


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
#ifndef PHI
  CImgDisplay main_displ(board,"Init");
#endif
  
  // here is the main loop, using two images swapped at each iteration
  img1 = &board;
  img2 = &board1;

  auto step = [&] (int start, int end) {
    {
#ifdef UTIMER
      utimer t("one chunk update");
#endif
      for(int i=start; i<end; i++) {    // first update the board in row chunks
	for(int j=0; j<m; j++) {
	  (*img2)(i,j,0,0) = next(i,j,n,m,(*img1));
	}
      }
    }
  };

  // create the ctask queue
  sample_queue<pair<int,int>> q;
  
  const int EOS = (-1);
  // barrier per iteration
  atomic<int> bar;

  const bool debug = false;
  
  auto body = [&] (sample_queue<pair<int,int>>& q, atomic<int>& bar) {
    while(true) {
      if(debug) cout << "Waiting for queue " << endl;
      auto task = q.pop();
      if(debug) cout << "Got task : " << task.first << "," << task.second << endl;
      if(task.first == EOS)
	break;
      step(task.first,task.second);
      if(debug) cout << "Computed " << endl; 
      bar--; // barrier update
      if(debug) cout << "Barrier updated = " << bar << endl;  // non corretto !!! 
    }
    if(debug) cout << "Exiting " << endl; 
    return;
  };

  vector<thread*> tid(nw);
  for(int t=0; t<nw; t++) {
    tid[t] = new thread(body,ref(q),ref(bar));
  }
  // all threads waiting ... 
    
  for(int i=0; i<iter; i++) {
    {
      int d = n / nw;          // amount of rows per thread

      if(debug) cout << "SETTING BARRIER" << endl; 
      bar = nw; 
      for(int t=0; t<nw; t++) {
	q.push(make_pair(t*d, (t+1)*d));
      }
      if(debug) cout << "WAITING BARRIER (it is " << bar << ")" << endl; 
      while(bar!=0);          // active wait
      
#ifndef PHI
      main_displ.display(*img2);  // then display results on screen
#endif
      auto temp = img1;           // eventually swap pointers 
      img1 = img2;
      img2 = temp;
      
    }
  }
  
  // when finished terminate threads : push nw EOS pairs
  for(int i=0; i<nw; i++)
    q.push(make_pair(EOS,EOS));

  for(int i=0; i<nw; i++)
    tid[i]->join();
  
  return(0);
}
