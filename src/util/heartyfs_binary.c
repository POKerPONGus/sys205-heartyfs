/**
 * @file heartyfs_binary.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing the binary helper functions.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <limits.h>
#include "heartyfs_binary.h"

// Maps a 4 bit integer [0,15] to its count of set bits.
const uint8_t set_bit_count_lookup[16] = {0, 1, 1, 2, 1, 2, 2, 3,
                                          1, 2, 2, 3, 2, 3, 3, 4};

int countSetBits(uint8_t b)
{
    return set_bit_count_lookup[b >> 4] +
           set_bit_count_lookup[b & 0x0F]; // Count lower and upper half
                                           // seperately
}

int findFirstSetBit(uint8_t b, int count)
{
    int found = 0;
    uint8_t mask = 0x80;
    int i;
    for (i = 0; i < CHAR_BIT && found < count; i++) {
        if ((uint8_t)(b & mask) > 0)
            found++;
        mask = mask >> 1;
    }
    return i - 1;
}