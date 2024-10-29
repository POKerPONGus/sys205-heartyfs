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

struct Array {
    void *val;
    int size;
    int len;
};

struct Interval {
    int start;
    int end;
};

/* Private Functions */

int _minInt(int, int);
int _maxInt(int, int);
bool _splitStr(char **, char, char **);
int _findStr(char *, struct Array *, bool(char *, void *));
int _countSetBits(uint8_t);
uint8_t _reverseBits(uint8_t);
int _findFirstSetBit(uint8_t, int);
void _findFreeBounds(uint8_t *, int, struct Interval *, struct Interval *);
int _findFreeDensestBlocks(uint8_t *, int, int (*)[2]);
int _getNodeID(union Block_HeartyFS *, char [], int );
void _initDirEntry(union Block_HeartyFS *, char *, int, int);
bool _isDirEntryMatch(char *, void *);

#endif