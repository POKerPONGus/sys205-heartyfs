#include "_private_heartyfs_utils.h"

int _minInt(int a, int b) { return (a < b) ? a : b; }

int _maxInt(int a, int b) { return (a > b) ? a : b; }

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

int _findStr(char *target, struct Array *arr,
             bool is_match(char *,void *))
{
    uint8_t *buf = arr->val;
    for (int i = 0; i < arr->len; i++) {
        if (is_match(target, (void *)buf + arr->size * i)) {
            return i;
        }
    }
    return -1;
}

// Maps a 4 bit integer [0,15] to its count of set bits.
const uint8_t set_bit_count_lookup[16] = {0, 1, 1, 2, 1, 2, 2, 3,
                                          1, 2, 2, 3, 2, 3, 3, 4};

int _countSetBits(uint8_t b)
{
    return set_bit_count_lookup[b >> 4] +
           set_bit_count_lookup[b & 0x0F]; // Count lower and upper half
                                           // seperately
}

const uint8_t reverse_bit_lookup[16] = {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A,
                                        0x06, 0x0E, 0x01, 0x09, 0x05, 0x0D,
                                        0x03, 0x0B, 0x07, 0x0F};

uint8_t _reverseBits(uint8_t b)
{
    return (reverse_bit_lookup[b & 0x0F] << 4) | reverse_bit_lookup[b >> 4];
}

int _findFirstSetBit(uint8_t b, int count)
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

void _findFreeBounds(uint8_t *map, int count_to_find, struct Interval *idx,
                     struct Interval *block_bounds)
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
    int end_offset = _findFirstSetBit(map[idx->end - 1], remainder) + 1;

    block_bounds->start = CHAR_BIT * idx->start + start_offset;
    block_bounds->end = CHAR_BIT * (idx->end - 1) + end_offset;
}

int _findFreeDensestBlocks(uint8_t *map, int block_count, int (*min_bounds)[2])
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
        //         printf("range %d'%d, %d'%d\t", f(range), f(min_bit_range));
        //         printf("leftovers %d\t", leftovers);
        //         printf("idx %d, %d\t", idx.start, idx.end);
        //         printf("bounds %d'%d, %d'%d\n", f(bounds.start),
        //         f(bounds.end));

        if (range < min_bit_range) {
            min_bit_range = range;
            (*min_bounds)[0] = bounds.start;
            (*min_bounds)[1] = bounds.end;
            if (min_bit_range == block_count)
                break;
        }
        // reset
        uint8_t mask = (0xFF << CHAR_BIT - (bounds.end % CHAR_BIT));
        leftovers = _countSetBits(map[idx.start]) +
                    _countSetBits(map[idx.end - 1] & mask);
        idx.start++;
        idx.end--;
    }

    return min_bit_range;
}


int _getNodeID(union Block_HeartyFS *mem, char rel_path[], int start_id)
{
    if (rel_path[0] == '/') {
        errno = EINVAL;
        return -1;
    }
    int id = start_id;
    bool is_match = false;
    char *ptr = rel_path;
    char *substr = NULL;
    while (_splitStr(&substr, '/', &ptr)) {
        if (mem[id].dir.type != TYPE_DIR_HEARTY_FS)
            break;

        struct Array entries = {.val = mem[id].dir.entries,
                                .len = mem[id].dir.len,
                                .size = sizeof(struct DirEntry_HeartyFS)};
        int idx = _findStr(substr, &entries, _isDirEntryMatch);
        if (idx != -1) {
            id = mem[id].dir.entries[idx].block_id;
        } else {
            id = -1;
            break;
        }
    }
    return id;
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