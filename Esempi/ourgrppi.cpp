#include <iostream>
#include <chrono>
#include <vector>

#include <experimental/optional>

#include "CImg.h"
 
using namespace cimg_library;
using namespace std;

#include <grppi.h>
using namespace grppi;

#include "utimer.cpp"

int main(int argc, char * argv[]) {

  // a.out nw stamp image*
  int  nw     = atoi(argv[1]);

  {
    utimer tall("seq time all");
    // read stamp
    CImg<unsigned char> stampo(argv[2]);
    int sw = stampo.width();
    int sh = stampo.height();
    // lettura e processamento immagini
    // for(int i=3; i<argc; i++) {

    // read image to stamp
    auto do_img = [&] (int i) {
      CImg<unsigned char> current(argv[i]);
      // check dimensions
      if((current.height() != sh) || (current.width()!= sw)) {
	cerr << "Error while processing image " << argv[i]
	     << " not same dimensions of stamp " << argv[2]<< endl;
	return(0);
      }
      // stamp it
      for(int i=0; i<sw; i++) {
	for(int j=0; j<sh; j++) {
	  if(stampo(i,j,0,0)==0 &&
	     stampo(i,j,0,1)==0 &&
	     stampo(i,j,0,2)==0) {
	    
	    current(i,j,0,0) = stampo(i,j,0,0);
	    current(i,j,0,1) = stampo(i,j,0,1);
	    current(i,j,0,2) = stampo(i,j,0,2);
	    
	  }
	}
	}
      
	// write image back
      string dest = string("result/")+string(argv[i]);
      current.save(dest.c_str());
      return(i);
    };
    

    auto genstream = [&] () -> experimental::optional<int> {
      static int i = 3;
      if(i<argc)
	return(i++);
      else
	return {};
    };

    auto fakedrain = [&] (int i) {
      cout << "Image " << argv[i] <<  " has been processed" << endl;
    };
    
    // grppi pipeline with second stage (farm)
    dynamic_execution thr = grppi::parallel_execution_native{};
    pipeline(thr, genstream, farm(nw, do_img), fakedrain);
  }
  // do some computation on the pixels (R,G,B values)
  return(0);
  }

  
