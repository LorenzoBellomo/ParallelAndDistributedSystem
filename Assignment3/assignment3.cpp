#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <CImg.h>
using namespace std;
using namespace cimg_library;

// notable constants and types 
using board_t = cimg_library::CImg<unsigned char>;
const unsigned int alive = 000;
const unsigned int dead  = 255;

CImgDisplay main_displ;
board_t board;

vector<int> thd_status;
vector<mutex> status_mutex;
vector<condition_variable> status_condition;

int n, num_tasks, nw;

void thd_func(int id, int left_limit, int right_limit) {

	/*
	It works this way: 
	Thread 0 (the first one to be spawned) is the master, and acts as a kind of worker,
	emitter and collector. Every thread computes its portion of work using the board as
	read-only and writing the result to a local matrix. By ending, it sets a 
	variable status for himself and waits until all threads in the pool have ended.
	Thread 0 (the master) waits for all to end, then tells all the threads to proceed
	to copy the partial result of the local matrixes to the board, which acts now as write 
	only. Thread 0 then syncs with all the threads and makes them sleep until a second has 
	passed from the beginning of the current iteration (this is made to make the display clear)
	*/

	unsigned int my_board[right_limit - left_limit][n];

	for(int iter = 0; iter < num_tasks; iter++) {

		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		for(int i = left_limit; i < right_limit; i++) {
			for(int j = 0; j < n; j++) {
				my_board[i - left_limit][j] = dead;

				int tlx = (i == 0)? 0 : i-1;
				int tly = (j == 0)?0 : j-1;
				int brx = (i == n-1)? n-1 : i+1;
				int bry = (j == n-1)? n-1 : j+1; 
				int adj_count = 0;

				for(int p = tlx; p <= brx; p++) {
					for(int k = tly; k <= bry; k++) {
						if((p != i || k != j) && board(p,k,0,0) == alive)
							adj_count++;
					}
				}

				if(board(i,j,0,0) == dead) {
					if(adj_count == 3) 
						my_board[i - left_limit][j] = alive;
				} else {
					if(adj_count == 3 || adj_count == 2) 
						my_board[i - left_limit][j] = alive;
				}
			}
		}

		if(id != 0) {
			{
				unique_lock<mutex> lock(status_mutex[id]);
				thd_status[id] = 1;
			}
			status_condition[id].notify_one();
			{
				unique_lock<mutex> lock(status_mutex[id]);
				status_condition[id].wait(lock, [id]{ return thd_status[id] == 2; });
			}
		} else {
			for(int i = 1; i < nw; i++) {
				{
					unique_lock<mutex> lock(status_mutex[i]);
					status_condition[i].wait(lock, [i]{ return thd_status[i] == 1; });
					thd_status[i] = 2;
				}
				status_condition[i].notify_one();
			}
		}

		for(int i = left_limit; i < right_limit; i++) {
			for(int j = 0; j < n; j++) {
				board(i,j,0,0) = my_board[i - left_limit][j];
			}
		}

		if(id != 0) {
			{
				unique_lock<mutex> lock(status_mutex[id]);
				thd_status[id] = 3;
			}
			status_condition[id].notify_one();
		}

		if(id == 0) {
			for(int i = 1; i < nw; i++) {
				unique_lock<mutex> lock(status_mutex[i]);
				status_condition[i].wait(lock, [i]{ return thd_status[i] == 3; });
			}
			std::chrono::system_clock::time_point stop = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed = stop - start;
    		auto musec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
			cout << "Computed " << (iter+1) << "th iteration in " << musec << " microseconds" << endl;
			main_displ.display(board);
			this_thread::sleep_for(std::chrono::microseconds(1000000 - musec));
			for(int i = 1; i < nw; i++){
				{
					unique_lock<mutex> lock(status_mutex[i]);
					thd_status[i] = 0;
				}
				status_condition[i].notify_one();
			}
		} else {
			unique_lock<mutex> lock(status_mutex[id]);
			status_condition[id].wait(lock, [id]{ return thd_status[id] == 0; });
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
	num_tasks = atoi(argv[2]);
	nw = atoi(argv[3]);
	(argc == 5)? srand(atoi(argv[4])) : srand(time(NULL));

	cout << "Using " << n << "x" << n << " board " << endl;

	// create an empty board
	board = board_t(n,n,1,1,0);
	vector<thread> thread_pool;
	thd_status = vector<int>(nw);
	status_mutex = vector<mutex>(nw);
	status_condition = vector<condition_variable>(nw);

	for(int i = 0; i < nw; i++) {
		thd_status[i] = 0;
		//status_mutex[i] = unique_ptr<mutex>(new mutex);
		//status_condition[i] = unique_ptr<condition_variable>(new condition_variable);
	}

	for(int i=0; i<n; i++)
		for(int j=0;  j<n; j++)
			board(i,j,0,0) = (rand() % 32 == 0 ? alive : dead);

	main_displ = CImgDisplay(board,"Game of Life");
	sleep(2);  // show for two seconds

	int range = n / nw;
    int extra = n % nw;
	int prev = 0;

	for(int i = 0; i < extra; i++) {
		thread_pool.push_back(thread(
			thd_func,
			i,
			prev,
			prev + range + 1
		));
		prev += (range + 1);
	}
        
	for(int i = 0; i < nw - extra; i++) {            
		thread_pool.push_back(thread(
			thd_func,
			i + extra,
			prev,
			prev + range
		));
		prev += range;
	}

	for(thread& t: thread_pool)
        t.join();

	return(0);
  
}
