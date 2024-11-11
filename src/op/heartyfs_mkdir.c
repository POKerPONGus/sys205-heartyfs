/**
 * @file heartyfs_mkdir.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's mkdir command on the command line.
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

static int _initDir(union Block *mem, char *name, int parent_id);

bool mkdirCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
{
    if (cmd_len != 2) {
        printf("usage: %s %s <dir-path>\n", exe_path, cmd[0]);
        return false;
    }

    int parent_id = getParentID(mem, cmd[1]);
    if (parent_id == -1)
        return false;

    char name[NAME_MAX_LEN];
    parseBasename(cmd[1], name, NAME_MAX_LEN);

    struct DirNode *parent = &mem[parent_id].dir;
    if (parent->len == DIR_MAX_ENTRIES) {
        errno = ENOMEM;
        perror("Directory Full");
    } else if (findStr(name, parent->entries, parent->len,
                       sizeof(struct DirEntry), isDirEntryMatch) != -1) {
        errno = EEXIST;
        perror(cmd[1]);
    } else if (_initDir(mem, name, parent_id) != -1) {
        return true;
    }
    return false;
}

/**
 * @brief 
 *  Initializes a new directory and its associated entries in the file system.
 *
 * @note 
 *  This function allocates a block for the new directory, marks it as used, and
 *  initializes the directory node. It also creates the `.` (current) and `..`
 *  (parent) directory entries for the new directory. The parent directory's
 *  entry list is updated to include the new directory.
 *
 * @param[in]  mem        Pointer to the memory block containing file system
 *                        data.
 * @param[in]  name       The name of the new directory to be created.
 * @param[in]  parent_id  The ID of the parent directory.
 *
 * @return 
 *   The ID of the newly created directory node, or -1 if directory
 *   initialization failed.
 */
static int _initDir(union Block *mem, char *name, int parent_id)
{
    struct Interval id_bounds = {0};
    if (!findFreeDensestBlocks(mem[BITMAP_ID].bitmap, 1, &EMPTY_INTERVAL,
                               &id_bounds)) {
        return -1;
    }
    setBitmapUsed(mem[BITMAP_ID].bitmap, &id_bounds);

    int id = id_bounds.start;
    initDirEntry(mem, name, id, parent_id);

    mem[id].dir = (struct DirNode){0};
    strncpy(mem[id].dir.name, name, NAME_MAX_LEN);
    mem[id].dir.type = TYPE_DIR;
    initDirEntry(mem, ".", id, id);
    initDirEntry(mem, "..", parent_id, id);
    return id;
}
