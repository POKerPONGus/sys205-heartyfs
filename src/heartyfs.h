#ifndef HEARTYFS_H
#define HEARTYFS_H

#include <fcntl.h>
#include <stdint.h>

#define BLOCK_SIZE (1 << 9)
#define DISK_SIZE (1 << 20)
#define BLOCK_COUNT (DISK_SIZE / BLOCK_SIZE)

#define BITMAP_LEN (BLOCK_COUNT >> 3)
#define FILE_MAX_BLOCKS 119
#define DIR_MAX_ENTRIES 14
#define NAME_MAX_LEN 28
#define BLOCK_MAX_DATA BLOCK_SIZE - sizeof(int)

enum InodeTypes_HeartyFS { TYPE_FILE_HEARTY_FS = 0, TYPE_DIR_HEARTY_FS = 1 };
enum AccessModes_HeartyFS {
    RDONLY_HEARTY_FS = O_RDONLY,
    WRONLY_HEARTY_FS = O_WRONLY,
    RDWR_HEARTY_FS = O_RDWR,
    APPEND_HEARTY_FS = O_RDWR + 1
};

struct DirEntry_HeartyFS {
    char name[NAME_MAX_LEN];
    int block_id;
};

struct DirNode_HeartyFS {
    char name[NAME_MAX_LEN];
    uint8_t type;
    int len;
    struct DirEntry_HeartyFS entries[DIR_MAX_ENTRIES];
};

struct FileNode_HeartyFS {
    char name[NAME_MAX_LEN];
    uint8_t type;
    int len;
    int blocks[FILE_MAX_BLOCKS];
};

struct DataBlock_HeartyFS {
    int size;
    uint8_t data[BLOCK_MAX_DATA];
};

union Block_HeartyFS {
    struct FileNode_HeartyFS file;
    struct DirNode_HeartyFS dir;
    struct DataBlock_HeartyFS data;
    uint8_t bitmap[BITMAP_LEN];
};

union Block_HeartyFS *mapDisk_HeartyFS(int mode);

#endif