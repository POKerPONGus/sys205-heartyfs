#ifndef _HEARTYFS_MAIN_UTILS_H
#define _HEARTYFS_MAIN_UTILS_H

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <unistd.h>

#include "heartyfs.h"

#define ROOT_ID 0
#define BITMAP_ID 1

enum BitmapState_HeartyFS { USED_HEARTY_FS, FREE_HEARTY_FS };

struct Array {
    void *val;
    int size;
    int len;
};

struct Interval {
    int start;
    int end;
};

void _updateBitmapFree(uint8_t *, struct Interval *);
void _updateBitmapUsed(uint8_t *, struct Interval *);
bool _findFreeDensestBlocks(uint8_t *, int, struct Interval *,
                            struct Interval *);
int _findNextFreeBlock(uint8_t *, int);
void _initDirEntry(union Block_HeartyFS *, char *, int, int);
bool _isDirEntryMatch(char *, void *);
struct Interval _intArrInterval(int *, int);
int _calcFileSize(union Block_HeartyFS *, int);
void _deleteFileData(union Block_HeartyFS *, int);
int _writeDataBlock(struct DataBlock_HeartyFS *, void *, int,
                    enum AccessModes_HeartyFS);
#endif