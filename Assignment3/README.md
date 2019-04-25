## Third assignment 
# parallel implementation of Game Of Life (Conway)
We require to provide a parallel implementation (using primitive C++ mechanisms) of the Conway's Game of Life. Given an n x n matrix of cells with an initial assignment of alive cells, compute N iterations using nw threads. In case you want to display on the screen (or save to the disk) a graphic representation of the board, please consider using CImg library (single .h file to be compiled with the source. Declaring a color picture is a matter of declaring a Cimg im(x-width, y-width, z-width, colors); variable and then single pixels may be accessed with the overloaded () operator (coords and channel parameters) for reading an writing.

notes: 
In order to display onto the screen the status of your game of life board, you may consider using the CImg primitives as proposed in the file in attachment. Please take into account that:
a) the program does not show game of life evolution, just random moves ...
b) it needs to be linked to X11 (-lX11 on the command line
c) showing images on X11 requires (some) time. In case you choose to display evolution of the game of life board you have to account the display time in the serial fraction of each iteration ...
Enjoy.