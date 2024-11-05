#include "_heartyfs_main_utils.h"
#include "_heartyfs_binary_utils.h"
#include "_heartyfs_math_utils.h"

/* Private Functions */

static struct Interval _spanInterval(struct Interval *, struct Interval *);
static struct Interval _returnInterval2(struct Interval *, struct Interval *);
static int _rangeOfInterval(struct Interval *);
static bool _findFirstFreeInterval(uint8_t *, int, struct Interval *);
static void _findNextFreeInterval(uint8_t *, struct Interval *);

void _updateBitmapFree(uint8_t *bitmap, struct Interval *bounds)
{
    if (bounds == NULL)
        return;
    int idx_start = bounds->start / CHAR_BIT;
    int idx_end = bounds->end / CHAR_BIT;
    int set_size = idx_end - idx_start - 1;
    if (set_size < 0)
        set_size = 0;
    memset(bitmap + idx_start + 1, FREE_HEARTY_FS, set_size);

    uint8_t start_mask = (0xFF >> (bounds->start % CHAR_BIT));
    uint8_t end_mask = (0xFF << (CHAR_BIT - (bounds->end % CHAR_BIT)));

    if (idx_start == idx_end) {
        bitmap[idx_end] = bitmap[idx_end] | (start_mask & end_mask);
    } else {
        bitmap[idx_start] = bitmap[idx_start] | start_mask;
        bitmap[idx_end] = bitmap[idx_end] | end_mask;
    }
}

void _updateBitmapUsed(uint8_t *bitmap, struct Interval *bounds)
{
    if (bounds == NULL)
        return;
    int idx_start = bounds->start / CHAR_BIT;
    int idx_end = bounds->end / CHAR_BIT;
    int set_size = idx_end - idx_start - 1;
    if (set_size < 0)
        set_size = 0;
    memset(bitmap + idx_start + 1, USED_HEARTY_FS, set_size);

    uint8_t start_mask = (0xFF << (CHAR_BIT - (bounds->start % CHAR_BIT)));
    uint8_t end_mask = (0xFF >> (bounds->end % CHAR_BIT));

    if (idx_start == idx_end) {
        bitmap[idx_end] = bitmap[idx_end] & (start_mask | end_mask);
    } else {
        bitmap[idx_start] = bitmap[idx_start] & start_mask;
        bitmap[idx_end] = bitmap[idx_end] & end_mask;
    }
}

bool _findFreeDensestBlocks(uint8_t *map, int block_count,
                            struct Interval *existing_bounds,
                            struct Interval *min_bounds)
{
    int smallest_possible;
    struct Interval (*span_bounds_func)(struct Interval *, struct Interval *);
    if (existing_bounds == NULL) {
        span_bounds_func = _returnInterval2;
        smallest_possible = block_count;
    } else {
        span_bounds_func = _spanInterval;
        smallest_possible = block_count + _rangeOfInterval(existing_bounds);
    }
    int min_range = INT_MAX;
    struct Interval bounds = {0};
    if (!_findFirstFreeInterval(map, block_count, &bounds))
        return false;

    while (bounds.start < BITMAP_LEN) {
        struct Interval merged_bounds =
            span_bounds_func(existing_bounds, &bounds);
        int new_range = _rangeOfInterval(&merged_bounds);

        // printf("min %d, %d\t", min_bounds->start, min_bounds->end);
        // printf("bounds %d, %d\t", bounds.start, bounds.end);
        // printf("range %d\n", new_range);
        if (new_range < min_range) {
            min_range = new_range;
            min_bounds->start = merged_bounds.start;
            min_bounds->end = merged_bounds.end;
            if (min_range == smallest_possible)
                break;
        }
        _findNextFreeInterval(map, &bounds);
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

struct Interval _intArrInterval(int *arr, int len)
{
    struct Interval bounds = {.start = INT_MAX, .end = INT_MIN};
    for (int i = 0; i < len; i++) {
        bounds.start = _minInt(bounds.start, arr[i]);
        bounds.end = _maxInt(bounds.end, arr[i]);
    }
    return bounds;
}


int _calcFileSize(union Block_HeartyFS *mem, int id)
{
    int data_id = mem[id].file.blocks[mem[id].file.len - 1];
    return (mem[id].file.len - 1) * BLOCK_MAX_DATA + mem[data_id].data.size;
}

void _deleteFileData(union Block_HeartyFS *mem, int id)
{
    struct FileNode_HeartyFS *file = &mem[id].file;
    if (file->len == 0)
        return;
    struct Interval bounds = _intArrInterval(file->blocks, file->len);
    _updateBitmapFree(mem[BITMAP_ID].bitmap, &bounds);
    file->len = 0;
}

int _writeDataBlock(struct DataBlock_HeartyFS *d_block, void *data, int size,
                    enum AccessModes_HeartyFS mode)
{
    int write_size = 0;
    switch (mode) {
    case WRONLY_HEARTY_FS:
        write_size = _minInt(size, BLOCK_MAX_DATA);
        memcpy(d_block->data, data, write_size);
        d_block->size = write_size;
        break;
    case APPEND_HEARTY_FS:
        write_size = _minInt(size, BLOCK_MAX_DATA - d_block->size);
        memcpy(d_block->data + d_block->size - 1, data, write_size);
        d_block->size += write_size;
        break;
    }
    return write_size;
}

static int _rangeOfInterval(struct Interval *b) { return b->end - b->start; }

static struct Interval _returnInterval2(struct Interval *r1,
                                        struct Interval *r2)
{
    (void)r1;
    return *r2;
}

static struct Interval _spanInterval(struct Interval *b1, struct Interval *b2)
{
    struct Interval new = {.start = _minInt(b1->start, b2->start),
                           .end = _maxInt(b1->end, b2->end)};
    return new;
}

static bool _findFirstFreeInterval(uint8_t *map, int count_to_find,
                                   struct Interval *new_bounds)
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

    new_bounds->start = start_id;
    new_bounds->end = CHAR_BIT * end_idx + (end_offset + 1);
    return true;
}

static void _findNextFreeInterval(uint8_t *map, struct Interval *curr_bounds)
{
    curr_bounds->start = _findNextFreeBlock(map, curr_bounds->start + 1);
    curr_bounds->end = _findNextFreeBlock(map, curr_bounds->end) + 1;
}