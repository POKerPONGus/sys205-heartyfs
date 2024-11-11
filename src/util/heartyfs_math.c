/**
 * @file heartyfs_math.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing the math helper functions.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include "heartyfs_math.h"

int maxInt(int a, int b) { return (a > b) ? a : b; }

int minInt(int a, int b) { return (a < b) ? a : b; }

int ceilDivInt(int n, int d) { return (n % d == 0) ? n / d : n / d + 1; }

int absInt(int x) { return (x < 0) ? -x : x; }

int countDigits(int x)
{
    if (x == 0)
        return 1;
    else if (x < 0)
        x *= -1;

    int i = 0;
    while (x > 0) {
        x /= 10;
        i++;
    }
    return i;
}