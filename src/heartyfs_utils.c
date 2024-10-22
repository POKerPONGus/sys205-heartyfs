#include "heartyfs.h"

#define BITMAP_LEN (BLOCK_COUNT >> 3)

struct BytesAndBits {
    unsigned int bytes;
    unsigned char bits;
};

void *writeBlock_HeartyFS(void *mem, void *data, int block_id)
{
    if (block_id < 0 || block_id > BLOCK_COUNT) {
        errno = EFAULT;
        return NULL;
    }
    return memcpy(mem + BLOCK_SIZE * block_id, data, BLOCK_SIZE);
}

// Maps a 4 bit integer [0,15] to its count of unset bits.
static const Byte unset_bit_count_lookup[16] = {4, 3, 3, 2, 3, 2, 2, 1,
                                                3, 2, 2, 1, 2, 1, 1, 0};

static int countUnsetBits(Byte b)
{
    return unset_bit_count_lookup[b >> 4] +
           unset_bit_count_lookup[b & 0x0F]; // Count lower and upper half
                                             // seperately
}

static const Byte reverse_bit_lookup[16] = {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A,
                                            0x06, 0x0E, 0x01, 0x09, 0x05, 0x0D,
                                            0x03, 0x0B, 0x07, 0x0F};

static Byte reverseBits(Byte b)
{
    return (reverse_bit_lookup[b & 0x0F] << 4) | reverse_bit_lookup[b >> 4];
}

static int findFirstUnsetBit(Byte b, int count)
{
    int found = 0;
    Byte mask = 0x80;
    int i;
    for (i = 0; i < __CHAR_BIT__ && found < count; i++) {
        if ((Byte)(~b & mask) > 0)
            found++;
        mask = mask >> 1;
    }
    return i - 1;
}

static int findFirstFree(Byte *map, int start_idx, struct BytesAndBits *start)
{
    int idx = start_idx;
    while (countUnsetBits(map[idx]) == 0 && idx < BITMAP_LEN) {
        idx++;
    }
    start->bytes = idx;
    start->bits = findFirstUnsetBit(map[idx], 1);
    return idx;
}

static void findFreeRange(Byte *map, int start_idx, int leftover,
                         int start_range, struct BytesAndBits *range)
{
    int idx = start_idx;
    range->bytes = start_range;
    while (leftover > 0 && idx < BITMAP_LEN) {
        leftover -= countUnsetBits(map[idx + range->bytes]);
        range->bytes++;
    }
    range->bits = 1 + findFirstUnsetBit(
        map[idx + range->bytes - 1],
        leftover + countUnsetBits(map[idx + range->bytes - 1]));
}

int findFreeDensestBlocks_HeartyFS(Byte *map, int block_count,
                                struct BytesAndBits *start_of_min)
{
    int min_bit_range = BLOCK_COUNT;
    struct BytesAndBits range = {0};
    int leftovers = block_count;
    int idx = 0;
    while (idx < BITMAP_LEN) {
        struct BytesAndBits start;
        idx = findFirstFree(map, idx, &start);
        // printf("%d, %d, %d\n", idx, leftovers, range.bytes);
        findFreeRange(map, idx, leftovers, range.bytes, &range);
        int bit_count = __CHAR_BIT__ * (range.bytes - 1) - start.bits + range.bits;

        // printf("8*(%d - 1) - %d + %d = %d\n", range.bytes, start.bits, range.bits,
        //     bit_count);

        if (bit_count < min_bit_range) {
            min_bit_range = bit_count;
            *start_of_min = start;
            if (min_bit_range == block_count)
                break;
        }
        // reset
        leftovers = countUnsetBits(map[idx]) + range.bits;
        range.bytes -= 2;
        printf("%d\n", range.bytes);
        idx++;
    }
    return min_bit_range;
}

int main()
{
    Byte map[BITMAP_LEN] = {0};
    int i;
    for (i = 0; i < BITMAP_LEN / 4; i++) {
        map[i] = 0xFF;
    }
    map[i] = 0xCC;
    struct BytesAndBits start = {0};
    unsigned int min = findFreeBestBlocks_HeartyFS(map, 18, &start);
    printf("%u, %d, %d\n", min, start.bytes, start.bits);
    return 0;
}
// [1010 1010]
// [0101 0101]