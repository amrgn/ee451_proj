#include "fft.h"
#include "util.h"

#include <pthread.h>
#include <iostream>

using namespace std;

struct thread_arg_t
{
    // get input to FFT from vector x
    NUM *x;
    unsigned int p;
    unsigned int n;

    // store the results of the FFT into array X
    NUM *X;
};

static void *thread_radix2_fft(void *arg)
{
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    
    NUM *x = thread_arg->x;
    unsigned int p = thread_arg->p;
    unsigned int n = thread_arg->n;
    NUM *X = thread_arg->X;
    
    // base case
    if (n == 1)
    {
        X[0] = x[0];
        return NULL;
    }

    // setup input for recursive call

    VEC x1(n / 2);
    VEC x2(n / 2);

    for (int idx = 0; idx < x1.size(); ++idx)
    {
        x1[idx] = x[2 * idx];
        x2[idx] = x[2 * idx + 1];
    }

    for (int idx = 0; idx < x1.size(); ++idx)
    {
        x[idx] = x1[idx];
        x[idx + (n / 2)] = x2[idx];
    }

    // generate thread args for recursive calls
    thread_arg_t thread_arg1, thread_arg2;

    thread_arg1.n = n / 2;
    thread_arg2.n = n / 2;

    thread_arg1.x = x;
    thread_arg2.x = x + (n / 2);

    thread_arg1.X = X;
    thread_arg2.X = X + (n / 2);
    
    if (p > 1)
    {
        thread_arg1.p = p / 2;
        thread_arg2.p = p / 2;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        pthread_t thread1, thread2;

        int rc;

        rc = pthread_create(&thread1, &attr, thread_radix2_fft, (void *) &thread_arg1);
        if (rc)
        {
            cerr << "ERROR: pthread_create error code is " << rc << endl;
            exit(-1);
        }

        rc = pthread_create(&thread2, &attr, thread_radix2_fft, (void *) &thread_arg2);
        if (rc)
        {
            cerr << "ERROR: pthread_create error code is " << rc << endl;
            exit(-1);
        }

        rc = pthread_join(thread1, NULL);
        if (rc)
        {
            cerr << "ERROR: pthread_join error code is " << rc << endl;
            exit(-1);
        }

        rc = pthread_join(thread2, NULL);
        if (rc)
        {
            cerr << "ERROR: pthread_join error code is " << rc << endl;
            exit(-1);
        }

        pthread_attr_destroy(&attr);
    }
    else
    {
        thread_arg1.p = 1;
        thread_arg2.p = 1;
        thread_radix2_fft(&thread_arg1);
        thread_radix2_fft(&thread_arg2);
    }

    // combine results

    VEC X1(n / 2);
    VEC X2(n / 2);

    for (int idx = 0; idx < X1.size(); ++idx)
    {
        X1[idx] = thread_arg1.X[idx];
        X2[idx] = thread_arg2.X[idx];
    }

    NUM curr_phase(1, 0);
    NUM unity_root = w(n, 1);

    for (int idx = 0; idx < X1.size(); ++idx)
    {
        X[idx] = X1[idx] + curr_phase * X2[idx];
        X[idx + (n / 2)] = X1[idx] - curr_phase * X2[idx];
        curr_phase *= unity_root;
    }
    
    return NULL;
}

VEC radix2_fft(const VEC& x, unsigned int p)
{
    unsigned int n = x.size();

    if (!is_power_of_two(n))
    {
        throw invalid_argument("Invalid input array length (must be a power of 2).");
    }

    if (!is_power_of_two(p))
    {
        throw invalid_argument("Invalid number of processors (must be a power of 2).");
    }

    // init thread arg
    thread_arg_t thread_arg;

    thread_arg.n = n;
    thread_arg.p = p;
    thread_arg.x = new NUM[n];
    thread_arg.X = new NUM[n];

    // copy in x

    for (int idx = 0; idx < n; ++idx)
    {
        thread_arg.x[idx] = x[idx];
    }

    thread_radix2_fft(&thread_arg);

    // get result
    VEC rval(n);
    for (int idx = 0; idx < n; ++idx)
    {
        rval[idx] = thread_arg.X[idx];
    }

    // destroy thread arg
    delete [] thread_arg.x;
    delete [] thread_arg.X;

    return rval;
}
