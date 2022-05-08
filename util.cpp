#include "util.h"
#include <math.h>

/**
 * Taken from https://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2
 */
bool is_power_of_two(unsigned int x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

NUM w(unsigned int N, unsigned int k)
{
    return NUM(cos(2 * M_PI * k / N), -sin(2 * M_PI * k / N));
}