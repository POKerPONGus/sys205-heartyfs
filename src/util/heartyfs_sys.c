/**
 * @file heartyfs_sys.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing the main system helper functions.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "heartyfs.h"
#include "heartyfs_bitmap.h"
#include "heartyfs_helper_structs.h"
#include "heartyfs_math.h"
#include "heartyfs_string.h"

static int _compareInt(const void *n1, const void *n2);
static void _printBin(uint8_t byte);

void printBitmap(uint8_t *bitmap)
{
    const int col_n = 10;
    int max_digits = countDigits(BITMAP_LEN / col_n);
    for (int i = 0; i < BITMAP_LEN; i++) {
        if (i % col_n == 0) {
            int row_idx = i / 10 + 1;
            printf("%d", row_idx);
            for (int j = countDigits(row_idx); j < max_digits; j++)
                putchar(' ');
            printf(" | ");
        }
        _printBin(bitmap[i]);

        if ((i + 1) % col_n == 0)
            printf("\n");
        else
            printf(" ");
    }
    printf("\n");
}

int getNodeID(union Block *mem, char path[], int start_id)
{
    if (start_id == GETNODEID_USE_CWD) {
        start_id = getCWD();
        if (start_id == -1) {
            return -1;
        }
    }

    int id;
    char *buf = malloc(strlen(path) + 1);
    if (buf == NULL) {
        perror(__func__);
        return -1;
    }
    if (path[0] == '/') {
        strcpy(buf, path + 1);
        id = 0;
    } else {
        strcpy(buf, path);
        id = start_id;
    }
    char *ptr = buf;
    char *substr = NULL;
    while (splitStr(&substr, '/', &ptr)) {
        if (mem[id].dir.type != TYPE_DIR)
            break;

        struct DirNode *dir = &mem[id].dir;
        int idx = findStr(substr, dir->entries, dir->len,
                          sizeof(struct DirEntry), isDirEntryMatch);
        if (idx != -1) {
            id = mem[id].dir.entries[idx].block_id;
        } else {
            errno = ENOENT;
            perror(path);
            id = -1;
            break;
        }
    }
    free(buf);
    return id;
}

int getParentID(union Block *mem, char path[])
{
    int dir_len = strlen(path) + 1;
    char *dir = malloc(dir_len);
    if (dir == NULL) {
        perror(__func__);
        return -1;
    }

    parseDir(path, dir, dir_len);
    int id = getNodeID(mem, dir, GETNODEID_USE_CWD);
    if (id == -1) {
    } else if (mem[id].dir.type != TYPE_DIR) {
        errno = ENOTDIR;
        perror(dir);
    } else {
        free(dir);
        return id;
    }
    free(dir);
    return -1;
}

bool setCWD(int cwd_id)
{
    bool is_set = true;
    int fd = open(CWD_STORE_PATH, O_WRONLY | O_CREAT | O_TRUNC);
    char buf[STR_MAX_LEN];
    sprintf(buf, "%d", cwd_id);
    if (write(fd, buf, strlen(buf)) == -1) {
        perror(CWD_STORE_PATH);
        is_set = false;
    }
    close(fd);
    return is_set;
}

int getCWD()
{
    int cwd_id = -1;
    int fd = open(CWD_STORE_PATH, O_RDONLY);
    char buf[STR_MAX_LEN];
    if (read(fd, buf, STR_MAX_LEN) == -1) {
        perror(CWD_STORE_PATH);
    } else {
        sscanf(buf, "%d", &cwd_id);
    }
    close(fd);
    return cwd_id;
}

bool isDirEntryMatch(char *name, const void *entry)
{
    char *entry_name = ((struct DirEntry *)entry)->name;
    return (strncmp(entry_name, name, NAME_MAX_LEN) == 0) ? true : false;
}

void initDirEntry(union Block *mem, char *name, int id, int parent_id)
{
    struct DirNode *parent_dir = &mem[parent_id].dir;
    strncpy(parent_dir->entries[parent_dir->len].name, name, NAME_MAX_LEN);
    parent_dir->entries[parent_dir->len].block_id = id;
    parent_dir->len++;
}

void deleteParentDirEntry(struct DirNode *parent_dir, int id)
{
    int idx_to_delete;
    for (int i = 0; i < parent_dir->len; i++)
        if (parent_dir->entries[i].block_id == id)
            idx_to_delete = i;

    int i = idx_to_delete;
    for (; i < parent_dir->len - 1; i++)
        parent_dir->entries[i] = parent_dir->entries[i + 1];

    parent_dir->entries[i] = (struct DirEntry){0};
    parent_dir->len--;
}

int calcFileSize(union Block *mem, int id)
{
    if (mem[id].file.len == 0) {
        return 0;
    } else {
        int data_id = mem[id].file.blocks[mem[id].file.len - 1];
        int block_size = mem[data_id].data.size;
        return (mem[id].file.len - 1) * BLOCK_MAX_DATA + block_size;
    }
}

void deleteFileData(union Block *mem, int id)
{
    struct FileNode *file = &mem[id].file;
    if (file->len == 0)
        return;

    int blocks[FILE_MAX_BLOCKS];
    memcpy(blocks, file->blocks, file->len * sizeof(int));
    qsort(blocks, file->len, sizeof(int), _compareInt);

    struct Interval free_bounds = {.start = blocks[0], .end = blocks[0] + 1};
    for (int i = 1; i < file->len; i++) {
        if (blocks[i - 1] + 1 != blocks[i]) {
            setBitmapFree(mem[BITMAP_ID].bitmap, &free_bounds);
            free_bounds.start = blocks[i];
            free_bounds.end = blocks[i];
        }
        free_bounds.end++;
    }
    setBitmapFree(mem[BITMAP_ID].bitmap, &free_bounds);
    file->len = 0;
}

int writeDataBlock(struct DataBlock *d_block, int size_used, void *data,
                   int size)
{
    int write_size = minInt(size, BLOCK_MAX_DATA - size_used);
    memcpy(d_block->data + size_used, data, write_size);
    d_block->size = size_used + write_size;
    return write_size;
}

int readFileID(union Block *mem, int id, void *buf, int size, int *offset)
{
    struct FileNode *file = &mem[id].file;
    uint8_t *buf_ptr = buf;
    int total_read = 0;
    for (int i = *offset / BLOCK_MAX_DATA; i < file->len && size > 0; i++) {
        struct DataBlock *data_block = &mem[file->blocks[i]].data;
        int block_offset = *offset % BLOCK_MAX_DATA;

        uint8_t *block_ptr = data_block->data + block_offset;
        int size_read = minInt(size, data_block->size - block_offset);
        memcpy(buf_ptr, block_ptr, size_read);

        total_read += size_read;
        *offset += size_read;
        buf_ptr += size_read;
        size -= size_read;
    }
    return total_read;
}

bool readFilePath(union Block *mem, char *path, char **buf, int *size)
{
    int id = getNodeID(mem, path, GETNODEID_USE_CWD);
    if (id == -1) {
        return false;
    } else if (mem[id].file.type != TYPE_FILE) {
        errno = EISDIR;
        perror(path);
        return false;
    }

    *size = calcFileSize(mem, id);
    *buf = malloc(*size);
    int offset = 0;
    readFileID(mem, id, *buf, *size, &offset);
    return true;
}

/**
 * @brief 
 *  Prints the binary representation of a byte.
 * 
 * @param[in] byte  The byte to print in binary.
 */
static void _printBin(uint8_t byte)
{
    int i = CHAR_BIT;
    while (i--) {
        putchar('0' + ((byte >> i) & 1));
    }
}

/**
 * @brief 
 *  Compares two integers for sorting.
 * 
 * @param[in] n1  Pointer to the first integer.
 * @param[in] n2  Pointer to the second integer.
 * 
 * @return 
 *   Difference between the two integers.
 */
static int _compareInt(const void *n1, const void *n2)
{
    return (*(int *)n1) - (*(int *)n2);
}