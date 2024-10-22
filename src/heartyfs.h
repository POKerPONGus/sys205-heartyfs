#ifndef H_HeartyFS
#define H_HeartyFS

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define DISK_FILE_PATH "/tmp/heartyfs"
#define BLOCK_SIZE (1 << 9)
#define DISK_SIZE (1 << 20)
#define BLOCK_NUM (DISK_SIZE / BLOCK_SIZE)

#define FILE_MAX_BLOCK_NUM 119
#define DIR_MAX_ENTRY_NUM 14
#define NAME_MAX_LEN 28

typedef unsigned char Byte;

enum IOnodeTypes_HeartyFS { TYPE_FILE_HeartyFS = 0, TYPE_DIR_HeartyFS = 1 };
enum BitmapState_HeartyFS {
    BITMAP_FREE_HeartyFS = 0,
    BITMAP_USED_HeartyFS = 1
};

struct DirEntry_HeartyFS {
    char name[NAME_MAX_LEN];
    int block_id;
};

struct IONode_HeartyFS {
    char name[NAME_MAX_LEN];
    Byte type;
    int size;
    union {
        int blocks[FILE_MAX_BLOCK_NUM];
        struct DirEntry_HeartyFS entries[DIR_MAX_ENTRY_NUM];
    } data;
};

void *writeBlock_HeartyFS(void *mem, void *data, int block_id);
#endif