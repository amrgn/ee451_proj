#include "fft.h"
#include "util.h"

#include <pthread.h>
#include <iostream>

using namespace std;

// copy vector to/from array
struct copy_thread_arg_t
{
    unsigned int n; // number of elements to copy
    unsigned int offset; // starting point
    unsigned int stride; // stride (i.e. number of elements to skip between copies in the data structure being copied from)

    VEC& vec;
    NUM *arr;

    copy_thread_arg_t(unsigned int num_elem, unsigned int offset_idx, unsigned int idx_stride, VEC& v, NUM *a)
        : n(num_elem), offset(offset_idx), stride(idx_stride), vec(v), arr(a)
    {

    }
};

// copy from vector to array
static void *thread_cp_vta(void *arg)
{
    copy_thread_arg_t *thread_arg = (copy_thread_arg_t *)arg;

    unsigned int n = thread_arg->n;
    unsigned int offset = thread_arg->offset;
    const VEC& vec = thread_arg->vec;
    NUM *arr = thread_arg->arr;
    unsigned int stride = thread_arg->stride;

    for (unsigned int idx = 0; idx < n; ++idx)
    {
        arr[offset + idx] = vec[offset + idx * stride];
    }

    return NULL;
}

// copy from array to vector
static void *thread_cp_atv(void *arg)
{
    copy_thread_arg_t *thread_arg = (copy_thread_arg_t *)arg;

    unsigned int n = thread_arg->n;
    unsigned int offset = thread_arg->offset;
    VEC& vec = thread_arg->vec;
    NUM *arr = thread_arg->arr;
    unsigned int stride = thread_arg->stride;

    for (unsigned int idx = 0; idx < n; ++idx)
    {
        vec[offset + idx] = arr[offset + idx * stride];
    }

    return NULL;
}

/**
 * Copy elements from vector to array in parallel with p threads
 */
static void cp_vta(NUM *arr, VEC& vec, unsigned int n, unsigned int p, unsigned int stride = 1)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_t *threads = new pthread_t[p];
    copy_thread_arg_t **thread_args = new copy_thread_arg_t*[p];
    unsigned int blk_sz = n / p;

    for (int idx = 0; idx < p; ++idx)
    {
        thread_args[idx] = new copy_thread_arg_t(blk_sz, idx * blk_sz * stride, stride, vec, arr);
    }

    int rc;

    for (int idx = 0; idx < p; ++idx)
    {
        rc = pthread_create(&threads[idx], &attr, thread_cp_vta, (void *) thread_args[idx]);
        if (rc)
        {
            cerr << "ERROR: pthread_create error code is " << rc << endl;
            exit(-1);
        }
    }
    for (int idx = 0; idx < p; ++idx)
    {
        rc = pthread_join(threads[idx], NULL);
        if (rc)
        {
            cerr << "ERROR: pthread_join error code is " << rc << endl;
            exit(-1);
        }
    }

    pthread_attr_destroy(&attr);
    delete [] threads;

    for (int idx = 0; idx < p; ++idx)
    {
        delete thread_args[idx];
    }
    delete [] thread_args;
}

/**
 * Copy elements from array to vector in parallel with p threads
 */
static void cp_atv(NUM *arr, VEC& vec, unsigned int n, unsigned int p, unsigned int stride = 1)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_t *threads = new pthread_t[p];
    copy_thread_arg_t **thread_args = new copy_thread_arg_t*[p];
    unsigned int blk_sz = n / p;

    for (int idx = 0; idx < p; ++idx)
    {
        thread_args[idx] = new copy_thread_arg_t(blk_sz, idx * blk_sz * stride, stride, vec, arr);
    }

    int rc;

    for (int idx = 0; idx < p; ++idx)
    {
        rc = pthread_create(&threads[idx], &attr, thread_cp_atv, (void *) thread_args[idx]);
        if (rc)
        {
            cerr << "ERROR: pthread_create error code is " << rc << endl;
            exit(-1);
        }
    }
    for (int idx = 0; idx < p; ++idx)
    {
        rc = pthread_join(threads[idx], NULL);
        if (rc)
        {
            cerr << "ERROR: pthread_join error code is " << rc << endl;
            exit(-1);
        }
    }

    pthread_attr_destroy(&attr);
    delete [] threads;

    for (int idx = 0; idx < p; ++idx)
    {
        delete thread_args[idx];
    }
    delete [] thread_args;
}


struct thread_arg_t
{
    // get input to FFT from vector x
    NUM *x;
    unsigned int p;
    unsigned int n;

    // store the results of the FFT into array X
    NUM *X;
};

static void *thread_strong_radix2_fft(void *arg)
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

    
    // rearrange elements
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

        rc = pthread_create(&thread1, &attr, thread_strong_radix2_fft, (void *) &thread_arg1);
        if (rc)
        {
            cerr << "ERROR: pthread_create error code is " << rc << endl;
            exit(-1);
        }

        rc = pthread_create(&thread2, &attr, thread_strong_radix2_fft, (void *) &thread_arg2);
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
        thread_strong_radix2_fft(&thread_arg1);
        thread_strong_radix2_fft(&thread_arg2);
    }

    // combine results

    VEC X1(n / 2);
    VEC X2(n / 2);

    if ((p > 1) && (X1.size() >= 2))
    {
        cp_atv(thread_arg1.X, X1, X1.size(), 2, 1);
        cp_atv(thread_arg2.X, X2, X2.size(), 2, 1);
    }
    else
    {
        for (int idx = 0; idx < X1.size(); ++idx)
        {
            X1[idx] = thread_arg1.X[idx];
            X2[idx] = thread_arg2.X[idx];
        }
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

VEC strong_radix2_fft(VEC& x, unsigned int p)
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

    if (p > n)
    {
        throw invalid_argument("Invalid input, number of processors cannot exceed array size");
    }

    // init thread arg
    thread_arg_t thread_arg;

    thread_arg.n = n;
    thread_arg.p = p;
    thread_arg.x = new NUM[n];
    thread_arg.X = new NUM[n];

    // copy in x, implements
    // for (int idx = 0; idx < n; ++idx)
    // {
    //     thread_arg.x[idx] = x[idx];
    // }
    cp_vta(thread_arg.x, x, n, p);


    thread_strong_radix2_fft(&thread_arg);

    // get result
    VEC rval(n);

    // copy out X, implements
    // for (int idx = 0; idx < n; ++idx)
    // {
    //     rval[idx] = thread_arg.X[idx];
    // }
    cp_atv(thread_arg.X, rval, n, p);

    // destroy thread arg
    delete [] thread_arg.x;
    delete [] thread_arg.X;

    return rval;
}
