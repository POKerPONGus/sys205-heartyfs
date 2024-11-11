/**
 * @file heartyfs_helper_structs.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing the helper functions for the helper structs.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include "heartyfs_helper_structs.h"
#include "heartyfs_math.h"

struct Interval intArrInterval(int *arr, int len)
{
    struct Interval bounds = {.start = INT_MAX, .end = INT_MIN};
    for (int i = 0; i < len; i++) {
        bounds.start = minInt(bounds.start, arr[i]);
        bounds.end = maxInt(bounds.end, arr[i] + 1);
    }
    return bounds;
}

bool isEqInterval(const struct Interval *b1, const struct Interval *b2) 
{
    return (b1->start == b2->start && b1->end == b2->end)? true : false;
}

int rangeOfInterval(const struct Interval *b)
{
    if (isEqInterval(b, &EMPTY_INTERVAL))
        return 0;
    else
        return b->end - b->start;
}

struct Interval spanInterval(const struct Interval *b1, const struct Interval *b2)
{
    struct Interval new = {.start = minInt(b1->start, b2->start),
                           .end = maxInt(b1->end, b2->end)};
    return new;
}