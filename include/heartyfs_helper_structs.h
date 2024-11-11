/**d
 * @file heartyfs_helper_structs.h
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  A header for simple miscellaneous helper structs and associated operations
 *  for heartyfs.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#ifndef _HEARTYFS_HELPER_STRUCTS_UTILS_H
#define _HEARTYFS_HELPER_STRUCTS_UTILS_H

#include <limits.h>
#include <stdbool.h>

struct Interval {
    int start;
    int end;
};

static const struct Interval EMPTY_INTERVAL = {.start = INT_MAX, .end = INT_MIN};

/**
 * @brief 
 *  Calculates the interval that bounds an array of integers.
 *
 *  Given an array of integers, this function finds the smallest interval that
 *  contains all elements in the array. The start of the interval is the minimum
 *  element, and the end is one greater than the maximum.
 *
 * @param[in] arr Array of integers.
 * @param[in] len Length of the array.
 * 
 * @return 
 *  Interval that bounds all elements in the array.
 */
struct Interval intArrInterval(int *arr, int len);


/**
 * @brief 
 *  Checks if two intervals are equal.
 *
 *  Determines if two intervals have the same start and end values.
 *
 * @param[in] b1 Pointer to the first interval.
 * @param[in] b2 Pointer to the second interval.
 * @return 
 *  `true` if the intervals are equal, `false` otherwise.
 */
bool isEqInterval(const struct Interval *b1, const struct Interval *b2);

/**
 * @brief 
 *  Calculates the range of an interval.
 *
 *  If the interval is equal to `EMPTY_INTERVAL`, returns 0.
 *  Otherwise, returns the difference between the end and start values.
 *
 * @param[in] b Pointer to the interval.
 * 
 * @return 
 *  Range of the interval, or 0 if the interval is empty.
 */
int rangeOfInterval(const struct Interval *b);

/**
 * @brief 
 *  Spans two intervals into a single interval.
 *
 *  Creates an interval that fully encompasses both input intervals.
 *  The resulting interval's start is the minimum start, and the end
 *  is the maximum end of the two intervals.
 *
 * @param[in] b1 Pointer to the first interval.
 * @param[in] b2 Pointer to the second interval.
 * 
 * @return 
 *  Interval that spans both input intervals.
 */
struct Interval spanInterval(const struct Interval *b1, const struct Interval *b2);

#endif