/**
 * @file heartyfs_math.h
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  A header for simple math operations for heartyfs.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#ifndef _HEARTYFS_MATH_UTILS_H
#define _HEARTYFS_MATH_UTILS_H

/**
 * @brief 
 *  Returns the maximum of two integers.
 *
 * @param[in] a First integer.
 * @param[in] b Second integer.
 * 
 * @return 
 *  The larger of the two integers.
 */
int maxInt(int a, int b);

/**
 * @brief 
 *  Returns the minimum of two integers.
 *
 * @param[in] a First integer.
 * @param[in] b Second integer.
 * 
 * @return 
 *  The smaller of the two integers.
 */
int minInt(int a, int b);

/**
 * @brief 
 *  Returns the ceiling of the division of two integers.
 *
 *  Computes the integer ceiling of the division `n / d`.
 *
 * @param[in] n Numerator.
 * @param[in] d Denominator.
 * 
 * @return 
 *  Ceiling of the integer division `n / d`.
 */
int ceilDivInt(int n, int d);

/**
 * @brief 
 *  Returns the absolute value of an integer.
 *
 * @param[in] x Input integer.
 * 
 * @return 
 *  Absolute value of the input integer.
 */
int absInt(int x);

/**
 * @brief 
 *  Counts the number of digits in an integer.
 *
 *  Handles both positive and negative integers. For `x = 0`, returns 1.
 *
 * @param[in] x Input integer.
 * 
 * @return 
 *  Number of digits in the integer.
 */
int countDigits(int x);
#endif