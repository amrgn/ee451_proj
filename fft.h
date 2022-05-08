#ifndef FFT_H
#define FFT_H

#include "typedefs.h"
#include <stdexcept>

/**
 * Takes radix2 FFT of input vector x using p processors. BOTH x.size() and p must be a power of 2!
 */
VEC radix2_fft(const VEC& x, unsigned int p);

#endif // FFT_H