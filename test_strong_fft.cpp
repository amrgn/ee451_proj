#include "strong_fft.h"
#include <iostream>
#include <random>
#include <time.h>

using namespace std;

int main(int argc, char **argv)
{
    unsigned int n = 1 << 20;
    unsigned int p = 1;

    if (argc != 2)
    {
        printf("Error, expected two arguments.\n");
        return 1;
    }

    p = strtol(argv[1], NULL, 10);

    VEC x(n, 0);
    VEC X;

    // init x
    uniform_real_distribution<double> unif(0, 1);
    default_random_engine re;
    for (int idx = 0; idx < x.size(); ++idx)
    {
        x[idx].real(unif(re));
        x[idx].imag(unif(re));
    }


    // take fft and see how long it takes
    struct timespec start, stop; 
    double time;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1)
    {
        perror("clock gettime");
    }

    X = strong_radix2_fft(x, p);

    if (clock_gettime( CLOCK_REALTIME, &stop) == -1 )
    {
        perror("clock gettime");
    }		
    time = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    cout << "Execution time for input of size " << n << " with " << p << " processors: " << time << endl;

    return 0;
}