/**
 * @file heartyfs_rm.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's rm command on the command line.
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
#include "heartyfs_string.h"

static void _deleteFile(union Block *mem, int id, int parent_id);

bool rmCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
{
    if (cmd_len != 2) {
        printf("usage: %s %s <file-path>\n", exe_path, cmd[0]);
        return false;
    }

    int parent_id = getParentID(mem, cmd[1]);
    if (parent_id == -1)
        return false;

    char name[NAME_MAX_LEN];
    parseBasename(cmd[1], name, NAME_MAX_LEN);
    int id = getNodeID(mem, name, parent_id);

    if (id == -1) {
    } else if (mem[id].file.type != TYPE_FILE) {
        errno = EISDIR;
        perror(cmd[1]);
    } else {
        _deleteFile(mem, id, parent_id);
        return true;
    }
    return false;
}

/**
 * @brief 
 *  Deletes the specified file from the file system.
 * 
 * @note 
 *  This helper function performs the actual deletion of the file. It removes
 *  the file's entry from the parent directory, frees any associated file data,
 *  and updates the bitmap to mark the blocks as free.
 * 
 * @param[in]  mem        Pointer to the memory block containing file system 
 *                        data.
 * @param[in]  id         The ID of the file node to be deleted.
 * @param[in]  parent_id  The ID of the parent directory of the file.
 */
static void _deleteFile(union Block *mem, int id, int parent_id)
{
    deleteParentDirEntry(&mem[parent_id].dir, id);
    deleteFileData(mem, id);
    setBitmapFree(mem[BITMAP_ID].bitmap, &(struct Interval){id, id + 1});
}