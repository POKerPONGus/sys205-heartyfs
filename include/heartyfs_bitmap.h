/**
 * @file heartyfs_bitmap.h
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  A header for operations on the bitmap for heartyfs, used to track memory
 *  usage.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#ifndef _HEARTYFS_BITMAP_UTILS_H
#define _HEARTYFS_BITMAP_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#include "heartyfs_helper_structs.h"

/**
 * @brief 
 *  Sets a range of bits in the bitmap to free.
 *
 *  Marks the specified range in the bitmap as free, indicated by `bounds`.
 *
 * @param[out]  bitmap The bitmap to modify.
 * @param[in]   bounds Interval specifying the range to free.
 */
void setBitmapFree(uint8_t *bitmap, struct Interval *bounds);

/**
 * @brief 
 *  Sets a range of bits in the bitmap as used.
 *
 *  Marks the specified range in the bitmap as used, indicated by `bounds`.
 *
 * @param[out]  bitmap The bitmap to modify.
 * @param[in]   bounds Interval specifying the range to mark as used.
 */
void setBitmapUsed(uint8_t *bitmap, struct Interval *bounds);

/**
 * @brief 
 *  Finds the smallest interval of free blocks that encompasses the specified
 *  bounds.
 *
 *  Searches for a free interval that fits within `block_count` blocks and is as
 *  compact as possible. Updates `min_bounds` with the smallest found interval,
 *  including `existing_bounds`.
 *
 * @param[in]   map             Bitmap to search within.
 * @param[in]   block_count     Number of contiguous free blocks required.
 * @param[in]   existing_bounds Existing bounds to incorporate in the new
 *                              interval.
 * @param[out]  min_bounds      Smallest interval that encompasses both
 *                              `existing_bounds` and the free blocks.
 * 
 * @return 
 *  `true` if a suitable interval is found, otherwise `false`.
 */
bool findFreeDensestBlocks(uint8_t *map, int block_count,
                           const struct Interval *existing_bounds,
                           struct Interval *min_bounds);

/**
 * @brief 
 *  Finds the next free block in the bitmap starting from a given position.
 *
 * @param[in] map       Bitmap to search.
 * @param[in] start_id  Index to start searching from.
 * 
 * @return 
 *  Index of the next free block.
 */
int findNextFreeBlock(uint8_t *map, int start_id);
#endif