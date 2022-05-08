#ifndef STRONG_FFT_H
#define STRONG_FFT_H

#include "typedefs.h"
#include <stdexcept>

/**
 * Takes radix2 FFT of input vector x using p processors. BOTH x.size() and p must be a power of 2!
 * Strong scaling approach
 */
VEC strong_radix2_fft(VEC& x, unsigned int p);

#endif // STRONG_FFT_H