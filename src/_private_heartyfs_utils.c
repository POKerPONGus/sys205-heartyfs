#include "_private_heartyfs_utils.h"

/* Private Functions */

static int _minInt(int, int);
static int _maxInt(int, int);
static int _countSetBits(uint8_t);
static int _findFirstSetBit(uint8_t, int);
static void _findFreeBounds(uint8_t *, int, struct Interval *,
                            struct Interval *);

bool _splitStr(char **substr, char delim, char **curr_ptr)
{
    if (**curr_ptr == '\0')
        return false;
    *substr = *curr_ptr;
    while (**curr_ptr != '\0' && **curr_ptr != delim) {
        (*curr_ptr)++;
    }
    if (**curr_ptr == delim) {
        **curr_ptr = '\0';
        (*curr_ptr)++;
    }
    return true;
}

int _findStr(char *target, struct Array *arr, bool is_match(char *, void *))
{
    uint8_t *buf = arr->val;
    for (int i = 0; i < arr->len; i++) {
        if (is_match(target, (void *)(buf + arr->size * i))) {
            return i;
        }
    }
    return -1;
}

void _updateBitmapFree(union Block_HeartyFS *mem, struct Interval bounds)
{
    uint8_t *bitmap = mem[BITMAP_ID].bitmap;
    struct Interval idx_bounds = {.start = bounds.start / CHAR_BIT,
                                  .end = bounds.end / CHAR_BIT};
    int range = idx_bounds.end - idx_bounds.start;
    int set_size = range - 2;
    set_size = (set_size < 0) ? 0 : set_size;
    memset(bitmap + idx_bounds.start + 1, FREE_HEARTY_FS, set_size);

    uint8_t start_mask = (0xFF >> (bounds.start % CHAR_BIT));
    uint8_t end_mask = (0xFF << (CHAR_BIT - (bounds.end % CHAR_BIT)));

    if (idx_bounds.start == idx_bounds.end) {
        bitmap[idx_bounds.end] =
            bitmap[idx_bounds.end] | (start_mask & end_mask);
    } else {
        bitmap[idx_bounds.start] = bitmap[idx_bounds.start] | start_mask;
        bitmap[idx_bounds.end] = bitmap[idx_bounds.end] | end_mask;
    }
}

void _updateBitmapUsed(union Block_HeartyFS *mem, struct Interval bounds)
{ 
    uint8_t *bitmap = mem[BITMAP_ID].bitmap;
    struct Interval idx_bounds = {.start = bounds.start / CHAR_BIT,
                                  .end = bounds.end / CHAR_BIT};
    int range = idx_bounds.end - idx_bounds.start;
    int set_size = range - 2;
    set_size = (set_size < 0) ? 0 : set_size;
    memset(bitmap + idx_bounds.start + 1, USED_HEARTY_FS, set_size);

    uint8_t start_mask = (0xFF << (CHAR_BIT - (bounds.start % CHAR_BIT)));
    uint8_t end_mask = (0xFF >> (bounds.end % CHAR_BIT));

    if (idx_bounds.start == idx_bounds.end) {
        bitmap[idx_bounds.end] =
            bitmap[idx_bounds.end] & (start_mask | end_mask);
    } else {
        bitmap[idx_bounds.start] = bitmap[idx_bounds.start] & start_mask;
        bitmap[idx_bounds.end] = bitmap[idx_bounds.end] & end_mask;
    }
}

int _findFreeDensestBlocks(uint8_t *map, int block_count,
                           struct Interval *min_bounds)
{
    struct Interval existing_bounds = {.start = INT_MAX, .end = INT_MIN};
    int min_bit_range = INT_MAX;
    int leftovers = block_count;
    struct Interval bounds = {0};
    struct Interval idx = {0};
    while (idx.start < BITMAP_LEN) {
        _findFreeBounds(map, leftovers, &idx, &bounds);
        bounds.start = _minInt(existing_bounds.start, bounds.start);
        bounds.end = _maxInt(existing_bounds.end, bounds.end);
        int range = bounds.end - bounds.start;

// #define f(x) (x) / CHAR_BIT, (x) % CHAR_BIT
//         printf("range %d, %d\t", range, min_bit_range);
//         printf("leftovers %d\t", leftovers);
//         printf("idx %d, %d\t", idx.start, idx.end);
//         printf("bounds %d, %d\n", bounds.start, bounds.end);

        if (range < min_bit_range) {
            min_bit_range = range;
            min_bounds->start = bounds.start;
            min_bounds->end = bounds.end;
            if (min_bit_range == block_count)
                break;
        }
        // reset
        uint8_t mask = (0xFF << (CHAR_BIT - (bounds.end % CHAR_BIT)));
        int end_offset = _countSetBits(map[idx.end - 1] & mask);
        leftovers = _countSetBits(map[idx.start]) + end_offset;
        idx.start++;
        idx.end -= (end_offset > 0) ? 1 : 0;
    }

    return min_bit_range;
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

static int _minInt(int a, int b) { return (a < b) ? a : b; }

static int _maxInt(int a, int b) { return (a > b) ? a : b; }

// Maps a 4 bit integer [0,15] to its count of set bits.
const uint8_t set_bit_count_lookup[16] = {0, 1, 1, 2, 1, 2, 2, 3,
                                          1, 2, 2, 3, 2, 3, 3, 4};

static int _countSetBits(uint8_t b)
{
    return set_bit_count_lookup[b >> 4] +
           set_bit_count_lookup[b & 0x0F]; // Count lower and upper half
                                           // seperately
}

static int _findFirstSetBit(uint8_t b, int count)
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

static void _findFreeBounds(uint8_t *map, int count_to_find,
                            struct Interval *idx, struct Interval *block_bounds)
{
    while (_countSetBits(map[idx->start]) == 0 && idx->start < BITMAP_LEN) {
        idx->start++;
    }
    while (count_to_find > 0 && idx->start < BITMAP_LEN) {
        count_to_find -= _countSetBits(map[idx->end]);
        idx->end++;
    }
    int remainder = count_to_find + _countSetBits(map[idx->end - 1]);
    int start_offset = _findFirstSetBit(map[idx->start], 1);
    int end_offset = _findFirstSetBit(map[idx->end - 1], remainder);
    
    block_bounds->start = CHAR_BIT * idx->start + start_offset;
    block_bounds->end = CHAR_BIT * (idx->end - 1) + end_offset + 1;
}