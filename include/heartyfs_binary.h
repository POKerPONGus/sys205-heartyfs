/**
 * @file heartyfs_binary.h
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  A header for simple bitwise/binary operations for heartyfs.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#ifndef _HEARTYFS_BINARY_UTILS_H
#define _HEARTYFS_BINARY_UTILS_H

#include <stdint.h>

/**
 * @brief 
 *  Counts the number of set bits (1s) in the given 8-bit integer.
 * 
 * @note 
 *  The function uses a lookup table to count the set bits in both the upper and
 *  lower 4-bit sections of the byte.
 * 
 * @param[in] b   The 8-bit integer whose set bits are to be counted.
 * 
 * @return 
 *   The total number of set bits (1s) in the given 8-bit integer.
 */
int countSetBits(uint8_t b);

/**
 * @brief 
 *  Finds the position of the nth set bit (1) in an 8-bit integer.
 * 
 * @note 
 *  The function scans the bits from left to right, starting from the most
 *  significant bit, and returns the position of the nth set bit (1).
 * 
 * @param[in] b      The 8-bit integer to search for set bits.
 * @param[in] count  The number of the set bit to find (1-based index).
 * 
 * @return 
 *   The position of the nth set bit, or -1 if the nth set bit does not exist.
 */
int findFirstSetBit(uint8_t b, int count);

#endif