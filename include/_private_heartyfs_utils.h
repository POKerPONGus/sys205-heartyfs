#ifndef _PRIVATE_HEARTYFS_H
#define _PRIVATE_HEARTYFS_H

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

#define DISK_FILE_PATH "/tmp/heartyfs"
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

bool _splitStr(char **, char, char **);
int _findStr(char *, struct Array *, bool(char *, void *));
void _updateBitmapFree(union Block_HeartyFS *mem, struct Interval bounds);
void _updateBitmapUsed(union Block_HeartyFS *mem, struct Interval bounds);
int _findFreeDensestBlocks(uint8_t *, int, struct Interval *);
void _initDirEntry(union Block_HeartyFS *, char *, int, int);
bool _isDirEntryMatch(char *, void *);

#endif