# ee451_proj

Make generates two executables: test_fft and test_strong_fft

Both programs take a single command line parameter p, the number of threads to use. For this implementation, p must be a power of 2.

Example usage:
./test_fft 1
./test_fft 4
./test_strong_fft 16

Both programs test on an input size of 2 ^ 20 by default. This can be modified by changing n in test_fft.cpp and test_strong_fft.cpp and recompiling. The input size must be a power of 2 as well.

fft.cpp and strong_fft.cpp implement the fft algorithm using pthreads and a recursive approach. Strong fft uses multiple threads when rearranging data elements (e.g. copying values between vectors) to attempt to get strong scaling in the algorithm by dividing the work among threads equally. 
