#ifndef UTIL_H
#define UTIL_H

#include "typedefs.h"

/**
 * True if x is a power of 2, false if not. 0 is not a power of 2.
 */
bool is_power_of_two(unsigned int x);

/**
 * Returns e ^ (-i * 2 * PI * k / N)
 */
NUM w(unsigned int N, unsigned int k);

#endif // UTIL_H