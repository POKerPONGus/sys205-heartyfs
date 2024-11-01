#include "_heartyfs_main_utils.h"
#include "_heartyfs_binary_utils.h"
#include "_heartyfs_math_utils.h"

/* Private Functions */

static struct Range _spanRange(struct Range *, struct Range *);
static struct Range _returnRange2(struct Range *, struct Range *);
static bool _findFirstFreeRange(uint8_t *, int, struct Range *);
static void _findNextFreeRange(uint8_t *, struct Range *);

void _updateBitmapFree(uint8_t *bitmap, struct Range range)
{
    int range_end = range.start + range.len;
    int idx_start = range.start / CHAR_BIT;
    int idx_end = range_end / CHAR_BIT;
    int set_size = idx_end - idx_start - 1;
    if (set_size < 0)
        set_size = 0;
    memset(bitmap + idx_start + 1, FREE_HEARTY_FS, set_size);

    uint8_t start_mask = (0xFF >> (range.start % CHAR_BIT));
    uint8_t end_mask = (0xFF << (CHAR_BIT - (range_end % CHAR_BIT)));

    if (idx_start == idx_end) {
        bitmap[idx_end] = bitmap[idx_end] | (start_mask & end_mask);
    } else {
        bitmap[idx_start] = bitmap[idx_start] | start_mask;
        bitmap[idx_end] = bitmap[idx_end] | end_mask;
    }
}

void _updateBitmapUsed(uint8_t *bitmap, struct Range range)
{
    int range_end = range.start + range.len;
    int idx_start = range.start / CHAR_BIT;
    int idx_end = range_end / CHAR_BIT;
    int set_size = idx_end - idx_start - 1;
    if (set_size < 0)
        set_size = 0;
    memset(bitmap + idx_start + 1, USED_HEARTY_FS, set_size);

    uint8_t start_mask = (0xFF << (CHAR_BIT - (range.start % CHAR_BIT)));
    uint8_t end_mask = (0xFF >> (range_end % CHAR_BIT));

    if (idx_start == idx_end) {
        bitmap[idx_end] = bitmap[idx_end] & (start_mask | end_mask);
    } else {
        bitmap[idx_start] = bitmap[idx_start] & start_mask;
        bitmap[idx_end] = bitmap[idx_end] & end_mask;
    }
}

bool _findFreeDensestBlocks(uint8_t *map, int block_count,
                            struct Range *existing_range,
                            struct Range *min_range)
{
    int smallest_possible;
    struct Range (*span_range_func)(struct Range *, struct Range *);
    if (existing_range == NULL) {
        span_range_func = _returnRange2;
        smallest_possible = block_count;
    } else {
        span_range_func = _spanRange;
        smallest_possible = block_count + existing_range->len;
    }

    min_range->len = INT_MAX;
    int leftovers = block_count;
    struct Range range = {0};
    if (!_findFirstFreeRange(map, leftovers, &range))
        return false;

    while (range.start < BITMAP_LEN) {
        printf("leftovers %d\t", leftovers);
        printf("min %d, %d\t", min_range->start, min_range->len);
        printf("range %d, %d\n", range.start, range.len);

        struct Range merged_range = span_range_func(existing_range, &range);
        if (merged_range.len < min_range->len) {
            min_range->start = merged_range.start;
            min_range->len = merged_range.len;
            if (min_range->len == smallest_possible)
                break;
        }
        _findNextFreeRange(map, &range);
    }
    return true;
}

int _findNextFreeBlock(uint8_t *map, int start_id)
{
    int idx = start_id / CHAR_BIT;
    int offset;
    uint8_t mask = 0xFF >> (start_id % CHAR_BIT);
    if (_countSetBits(map[idx] & mask) == 0) {
        mask = 0xFF;
        idx++;
    }
    while (_countSetBits(map[idx]) == 0 && idx < BITMAP_ID) {
        idx++;
    }
    offset = _findFirstSetBit(map[idx] & mask, 1);
    return CHAR_BIT * idx + offset;
}

void _initDirEntry(union Block_HeartyFS *mem, char *name, int id, int parent_id)
{
    struct DirNode_HeartyFS *parent_dir = &mem[parent_id].dir;
    strncpy(parent_dir->entries[parent_dir->len].name, name, NAME_MAX_LEN);
    parent_dir->entries[parent_dir->len].block_id = id;
    parent_dir->len++;
}

bool _isDirEntryMatch(char *name, void *entry)
{
    char *entry_name = ((struct DirEntry_HeartyFS *)entry)->name;
    return (strcmp(entry_name, name) == 0) ? true : false;
}

static struct Range _returnRange2(struct Range *r1, struct Range *r2)
{
    (void)r1;
    return *r2;
}

static struct Range _spanRange(struct Range *r1, struct Range *r2)
{
    struct Range new;
    int start_diff = r1->start - r2->start;
    if (start_diff < 0) {
        new.start = r1->start;
        new.len = _maxInt(_absInt(start_diff) + r2->len, r1->len);
    } else {
        new.start = r2->start;
        new.len = _maxInt(_absInt(start_diff) + r1->len, r2->len);
    }
    return new;
}

static bool _findFirstFreeRange(uint8_t *map, int count_to_find,
                                struct Range *new_range)
{
    int start_id = _findNextFreeBlock(map, 0);

    int end_idx = start_id / CHAR_BIT;
    uint8_t mask = 0xFF >> (start_id % CHAR_BIT);
    count_to_find -= _countSetBits(map[end_idx] & mask);
    while (count_to_find > 0) {
        if (end_idx >= BITMAP_LEN)
            return false;

        end_idx++;
        count_to_find -= _countSetBits(map[end_idx]);
    }
    int remainder = count_to_find + _countSetBits(map[end_idx]);
    int end_offset = _findFirstSetBit(map[end_idx], remainder);

    new_range->start = start_id;
    new_range->len = CHAR_BIT * end_idx + (end_offset + 1) - start_id;
    return true;
}

static void _findNextFreeRange(uint8_t *map, struct Range *curr_range)
{
    int old_end = curr_range->start + curr_range->len;
    int start_id = _findNextFreeBlock(map, curr_range->start + 1);
    int end_id = _findNextFreeBlock(map, old_end) + 1;

    curr_range->start = start_id;
    curr_range->len = end_id - start_id;
}