/**
 * @file heartyfs_rmdir.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's rmdir command on the command line.
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

static void _deleteDir(union Block *mem, int id);

bool rmdirCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
{
    if (cmd_len != 2) {
        printf("usage: %s %s <dir-path>\n", exe_path, cmd[0]);
        return false;
    }

    int id = getNodeID(mem, cmd[1], GETNODEID_USE_CWD);
    int cwd_id = getCWD();
    if (cwd_id == -1 || id == -1) {
    } else if (id == cwd_id) {
        errno = EPERM;
        perror("Cannot delete current directory");
    } else if (mem[id].dir.type != TYPE_DIR) {
        errno = ENOTDIR;
        perror(cmd[1]);
    } else if (mem[id].dir.len > 2) {
        errno = ENOTEMPTY;
        perror(cmd[1]);
    } else {
        _deleteDir(mem, id);
        return true;
    }
    return false;
}

/**
 * @brief 
 *  Deletes the specified directory from the file system.
 * 
 * @note 
 *  This helper function performs the actual deletion of the directory. It removes 
 *  the directory's entry from its parent directory and updates the bitmap to mark 
 *  the directory's block as free.
 * 
 * @param[in]  mem  Pointer to the memory block containing file system data.
 * @param[in]  id   The ID of the directory node to be deleted.
 */
static void _deleteDir(union Block *mem, int id)
{
    struct DirNode *dir = &mem[id].dir;
    int parent_id = dir->entries[PARENT_DIR_ENTRY_IDX].block_id;
    deleteParentDirEntry(&mem[parent_id].dir, id);
    setBitmapFree(mem[BITMAP_ID].bitmap, &(struct Interval){id, id + 1});
}