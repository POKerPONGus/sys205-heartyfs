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

struct Range {
    int start;
    int len;
};

void _updateBitmapFree(uint8_t *, struct Range);
void _updateBitmapUsed(uint8_t *, struct Range);
bool _findFreeDensestBlocks(uint8_t *, int, struct Range *, struct Range *);
int _findNextFreeBlock(uint8_t *, int);
void _initDirEntry(union Block_HeartyFS *, char *, int, int);
bool _isDirEntryMatch(char *, void *);
#endif