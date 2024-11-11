/**
 * @file heartyfs_bitmap.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing the bitmap helper functions.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "heartyfs.h"
#include "heartyfs_binary.h"
#include "heartyfs_bitmap.h"
#include "heartyfs_helper_structs.h"
#include "heartyfs_math.h"
#include "heartyfs_string.h"

/* Private Functions */

static bool _findFirstFreeInterval(uint8_t *, int, struct Interval *);

void setBitmapFree(uint8_t *bitmap, struct Interval *bounds)
{
    if (bounds == NULL)
        return;
    int idx_start = bounds->start / CHAR_BIT;
    int idx_end = bounds->end / CHAR_BIT;
    int set_size = idx_end - idx_start - 1;
    if (set_size < 0)
        set_size = 0;
    memset(bitmap + idx_start + 1, 0xFF, set_size);
    uint8_t start_mask = (0xFF >> (bounds->start % CHAR_BIT));
    uint8_t end_mask = (0xFF << (CHAR_BIT - (bounds->end % CHAR_BIT)));

    if (idx_start == idx_end) {
        bitmap[idx_end] = bitmap[idx_end] | (start_mask & end_mask);
    } else {
        bitmap[idx_start] = bitmap[idx_start] | start_mask;
        bitmap[idx_end] = bitmap[idx_end] | end_mask;
    }
}

void setBitmapUsed(uint8_t *bitmap, struct Interval *bounds)
{
    if (bounds == NULL)
        return;
    int idx_start = bounds->start / CHAR_BIT;
    int idx_end = bounds->end / CHAR_BIT;
    int set_size = idx_end - idx_start - 1;
    if (set_size < 0)
        set_size = 0;
    memset(bitmap + idx_start + 1, 0x00, set_size);

    uint8_t start_mask = (0xFF << (CHAR_BIT - (bounds->start % CHAR_BIT)));
    uint8_t end_mask = (0xFF >> (bounds->end % CHAR_BIT));

    if (idx_start == idx_end) {
        bitmap[idx_end] = bitmap[idx_end] & (start_mask | end_mask);
    } else {
        bitmap[idx_start] = bitmap[idx_start] & start_mask;
        bitmap[idx_end] = bitmap[idx_end] & end_mask;
    }
}

bool findFreeDensestBlocks(uint8_t *map, int block_count,
                           const struct Interval *existing_bounds,
                           struct Interval *min_bounds)
{
    int smallest_possible = block_count + rangeOfInterval(existing_bounds);
    int min_range = INT_MAX;
    struct Interval bounds = {0};
    if (!_findFirstFreeInterval(map, block_count, &bounds)) {
        errno = ENOSPC;
        perror("Disk: " DISK_FILE_PATH);
        return false;
    }

    while (1) {
        struct Interval merged_bounds =
            spanInterval(existing_bounds, &bounds);
        int new_range = rangeOfInterval(&merged_bounds);
        if (new_range < min_range) {
            min_range = new_range;
            min_bounds->start = merged_bounds.start;
            min_bounds->end = merged_bounds.end;
            if (min_range == smallest_possible)
                break;
        }
        if (bounds.end >= BITMAP_LEN)
            break;
        
        bounds.start = findNextFreeBlock(map, bounds.start + 1);
        bounds.end = findNextFreeBlock(map, bounds.end) + 1;
    }
    return true;
}

int findNextFreeBlock(uint8_t *map, int start_id)
{
    int idx = start_id / CHAR_BIT;
    int offset;
    uint8_t mask = 0xFF >> (start_id % CHAR_BIT);
    if (countSetBits(map[idx] & mask) == 0) {
        mask = 0xFF;
        idx++;
    }
    while (countSetBits(map[idx]) == 0 && idx < BITMAP_LEN) {
        idx++;
    }
    offset = findFirstSetBit(map[idx] & mask, 1);
    return CHAR_BIT * idx + offset;
}

/**
 * @brief 
 *  Finds the first free interval in the bitmap that can fit the specified
 *  number of blocks.
 *
 *  This function searches the bitmap for a contiguous interval of free blocks
 *  that meets the required count. If found, the interval's bounds are set in
 *  `new_bounds`.
 *
 * @param[in]   map             Bitmap to search within.
 * @param[in]   count_to_find   Number of free blocks needed.
 * @param[out]  new_bounds      Interval where free blocks are located if found.
 * 
 * @return 
 *  `true` if a free interval is found, otherwise `false`.
 */
static bool _findFirstFreeInterval(uint8_t *map, int count_to_find,
                                   struct Interval *new_bounds)
{
    int start_id = findNextFreeBlock(map, 0);

    int end_idx = start_id / CHAR_BIT;
    uint8_t mask = 0xFF >> (start_id % CHAR_BIT);
    count_to_find -= countSetBits(map[end_idx] & mask);
    while (count_to_find > 0) {
        if (end_idx >= BITMAP_LEN)
            return false;

        end_idx++;
        count_to_find -= countSetBits(map[end_idx]);
    }
    int remainder = count_to_find + countSetBits(map[end_idx]);
    int end_offset = findFirstSetBit(map[end_idx], remainder);

    new_bounds->start = start_id;
    new_bounds->end = CHAR_BIT * end_idx + (end_offset + 1);
    return true;
}