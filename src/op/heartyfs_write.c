/**
 * @file heartyfs_write.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's write command on the command line.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heartyfs.h"
#include "heartyfs_bitmap.h"
#include "heartyfs_helper_structs.h"
#include "heartyfs_math.h"
#include "heartyfs_string.h"

#define CMD_ARG_CNT 1

static bool _writeFile(union Block *mem, int id, void *data, int size);
static bool _readStdin(char **buf, size_t *offset);
static int _getWriteMode(char **cmd, int cmd_len, int *operand_start);

bool writeCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
{
    int operand_start;
    int mode = _getWriteMode(cmd, cmd_len, &operand_start);
    if (mode == -1) 
        return false;
        
    int operand_count = cmd_len - operand_start;
    if (operand_count < 1 || operand_count > 2) {
        printf("usage: %s %s [options] <dest-path> [src-path]\n", exe_path,
               cmd[0]);
        return false;
    }
    int id = getNodeID(mem, cmd[operand_start], GETNODEID_USE_CWD);
    if (id == -1) {
        return false;
    } else if (mem[id].file.type != TYPE_FILE) {
        errno = EISDIR;
        perror(cmd[operand_start]);
        return false;
    }

    char *input = NULL;
    int size;
    bool is_ok;
    switch (operand_count) {
    case 1: {
        size_t tmp_size;
        is_ok = _readStdin(&input, &tmp_size);
        size = (int)tmp_size;
        break;
    }
    case 2: {
        char *read_path = cmd[operand_start + 1];
        is_ok = readFilePath(mem, read_path, &input, &size);
        break;
    }
    }

    /* Check File size & Resize */
    if (!is_ok) {
    } else if (mode == WRONLY) {
        if (size > FILE_MAX_SIZE) {
            errno = ENOMEM;
            perror(cmd[operand_start]);
            is_ok = false;
        } else {
            deleteFileData(mem, id);
        }
    } else if (mode == APPEND) {
        if (size > FILE_MAX_SIZE - calcFileSize(mem, id)) {
            errno = ENOMEM;
            perror(cmd[operand_start]);
            is_ok = false;
        }
    }
    if (is_ok)
        is_ok = _writeFile(mem, id, input, size);
    free(input);

    if (is_ok)
        return true;
    else
        return false;
}

/**
 * @brief 
 *  Determines the write mode (overwrite or append) based on the command line
 *  options.
 *
 * @note 
 *  The function processes the command line options for the write command and
 *  sets the appropriate write mode. It handles options like `-w` (overwrite)
 *  and `-a` (append). If an invalid option is found, it returns `-1`.
 *
 * @param[in]  cmd             Array of command arguments.
 * @param[in]  cmd_len         The length of the command argument array.
 * @param[out] operand_start   The starting index of the operand arguments.
 * 
 * @return 
 *   WRONLY : Overwrite mode. @n
 *   APPEND : Append mode. @n
 *   -1     : Invalid options or error.
 */
static int _getWriteMode(char **cmd, int cmd_len, int *operand_start)
{
    int mode = WRONLY;
    int count = 0;
    char opt;
    char **params = cmd + CMD_ARG_CNT;
    int len = cmd_len - CMD_ARG_CNT;
    while ((opt = parseOpt(params, len, &count)) != '\0') {
        if (count > 1) {
            fprintf(stderr, "Too many options\n");
            return -1;
        }
        switch (opt) {
        case 'w':
            mode = WRONLY;
            break;
        case 'a':
            mode = APPEND;
            break;
        case '-':
            goto loop_end;
        default:
            errno = EINVAL;
            perror("Options");
            return -1;
        }
    }
loop_end:
    *operand_start = CMD_ARG_CNT + count;
    return mode;
}


/**
 * @brief 
 *  Writes the given data to a file in the file system.
 * 
 * @note 
 *  This function writes data to the file blocks, either overwriting existing 
 *  content or appending to it, depending on the provided write mode. If the 
 *  file needs more space, it finds free blocks in the bitmap and allocates 
 *  them for writing the data.
 * 
 * @param[in]  mem   Pointer to the memory block containing file system data.
 * @param[in]  id    The ID of the file to write to.
 * @param[in]  data  The data to write to the file.
 * @param[in]  size  The size of the data to write.
 * 
 * @return 
 *   true  : Data successfully written to the file. @n
 *   false : Failed to write data (e.g., insufficient space).
 */
static bool _writeFile(union Block *mem, int id, void *data, int size)
{
    struct FileNode *file = &mem[id].file;

    int file_size = calcFileSize(mem, id);
    int new_len = ceilDivInt(file_size + size, BLOCK_MAX_DATA);
    struct Interval curr_bounds = intArrInterval(file->blocks, file->len);
    struct Interval block_bounds;
    if (!findFreeDensestBlocks(mem[BITMAP_ID].bitmap, new_len - file->len,
                               &curr_bounds, &block_bounds)) {
        return false;
    }

    int curr_block;
    uint8_t *data_ptr = data;
    if (file_size > 0) {
        curr_block = file->blocks[file->len - 1];

        struct DataBlock *d_block = &mem[curr_block].data;
        int size_wrote = writeDataBlock(d_block, d_block->size, data_ptr, size);

        size -= size_wrote;
        data_ptr += size_wrote;
    }
    curr_block = findNextFreeBlock(mem[BITMAP_ID].bitmap, block_bounds.start);
    for (int i = file->len; size > 0 && curr_block < block_bounds.end; i++) {
        file->blocks[i] = curr_block;

        int size_wrote =
            writeDataBlock(&mem[curr_block].data, 0, data_ptr, size);

        size -= size_wrote;
        data_ptr += size_wrote;
        curr_block = findNextFreeBlock(mem[BITMAP_ID].bitmap, curr_block + 1);
    }
    setBitmapUsed(mem[BITMAP_ID].bitmap, &block_bounds);
    file->len = new_len;

    return true;
}

/**
 * @brief 
 *  Reads data from standard input into a buffer.
 * 
 * @note 
 *  This function allocates memory for the buffer and continuously reads from 
 *  standard input until all data is read. The buffer is reallocated if more 
 *  space is needed.
 * 
 * @param[out] buf     A pointer to the buffer where the input will be stored.
 * @param[out] offset  The number of bytes read so far.
 * 
 * @return 
 *   true  : Successfully read data from stdin. @n
 *   false : Failed to read data (e.g., memory allocation error).
 */
static bool _readStdin(char **buf, size_t *offset)
{
    if (*buf != NULL) {
        errno = EINVAL;
        fprintf(stderr, "Address in %p is not NULL. %s not allocating memory.",
                (void *)buf, __func__);
        return false;
    }
    *offset = 0;
    size_t size = STR_MAX_LEN;
    *buf = malloc(size);
    if (*buf == NULL) {
        perror(__func__);
        return false;
    }
    while (1) {
        int size_read = fread(*buf + *offset, sizeof(char), size - *offset, stdin);
        if (ferror(stdin)) {
            perror(__func__);
            return false;
        }
        *offset += size_read;
        if (*offset == size) {
            size *= 2;
            *buf = realloc(*buf, size);
            if (*buf == NULL) {
                perror(__func__);
                return false;
            }
        } else {
            break;
        }
    }
    return true;
}