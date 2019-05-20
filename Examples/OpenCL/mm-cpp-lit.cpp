#include <iostream>
#include <string>
#include <fstream>

#include <utimer.cpp>

using namespace std;

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#define DEVICE CL_DEVICE_TYPE_DEFAULT

int main(int argc, char * argv[]) {

  int N = atoi(argv[1]);
  long sz = N * N;


  std::vector<float> h_A(sz);
  std::vector<float> h_B(sz);
  std::vector<float> h_C(sz);
  std::vector<float> h_Cc(sz,0.0);

  cl::Buffer d_A, d_B, d_C;
  
  for (int i = 0; i < sz; i++) {
    h_A[i] = ((float) (i%N));
    h_B[i] = ((float) (i%N));
  }

  auto print_mat = [] (string nm, int N, vector<float> m) {
    cout << "----> matrix " << nm << endl;
    for(int i=0; i<N; i++) {
      for(int j=0; j<N; j++) 
	cout << m[i*N+j] << " ";
      cout << endl;
    }
    cout<< "-----> end of matrix " << nm << endl; 
  };

  // print_mat("A",N,h_A);
  // print_mat("B",N,h_B);

  // usage of C++11 string literals to introduce code
  const char * KernelCode = R"CODE(
  __kernel void mmul(
		   const int N,
		   __global float *A,
		   __global float *B,
		   __global float *C)
  {
    int k;
    int i = get_global_id(0);
    int j = get_global_id(1);
    float tmp = 0.0f;
    for (k = 0; k < N; k++)
      tmp += A[i*N+k]*B[k*N+j];
    C[i*N+j] = tmp;
  }
  )CODE";

  cl::Context context(DEVICE);
  cl::Program program(context,
		      KernelCode,
		      true);

  cl::CommandQueue queue(context);
  cl::make_kernel <int, cl::Buffer, cl::Buffer, cl::Buffer> mmul(program, "mmul");
  d_A = cl::Buffer(context, h_A.begin(), h_A.end(), true);
  d_B = cl::Buffer(context, h_B.begin(), h_B.end(), true);
  d_C = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * sz);

  START(mm1);
  mmul(cl::EnqueueArgs(queue, cl::NDRange(N,N)),N, d_A, d_B, d_C );
  cl::copy(queue, d_C, h_C.begin(), h_C.end());
  STOP(mm1,mmt);

  START(smm1);
  for(int i=0; i<N; i++)
    for(int j=0; j<N; j++) 
      for(int k=0; k<N; k++)
	h_Cc[i*N+j] += h_A[i*N+k] * h_B[k*N+j];
  STOP(smm1,smmt);
  
  //  print_mat("C",N,h_C);

  cout << "Spent " << mmt << " usecs to compute " << N*N*N << " FLOP on GPU" << endl << "FLOPS = " << ((float) (N*N*N)) / (((float) mmt)/1000000.0) << endl;
  cout << "Spend " << smmt << " usecs on CPU " << endl
       << "CPU FLOPS " << ((float) (N*N*N)) / (((float) smmt)/1000000.0) << endl;
  return(0);
}
