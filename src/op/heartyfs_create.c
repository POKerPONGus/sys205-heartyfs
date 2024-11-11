/**
 * @file heartyfs_create.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's create command on the command line.
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
#include "heartyfs_string.h"

static int _initFile(union Block *mem, char *name, int parent_id);

bool createCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
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

    struct DirNode *parent = &mem[parent_id].dir;
    if (parent->len == DIR_MAX_ENTRIES) {
        errno = ENOMEM;
        perror("Directory Full");
    } else if (findStr(name, parent->entries, parent->len,
                       sizeof(struct DirEntry), isDirEntryMatch) != -1) {
        errno = EEXIST;
        perror(cmd[1]);
    } else if (_initFile(mem, name, parent_id) == -1)
        return false;
    return true;
}


/**
 * @brief 
 *  Initializes a new file entry in the file system and allocates blocks for it.
 *
 * @note 
 *  This function attempts to find a free block, marks it as used, and
 *  initializes the corresponding file node structure. It also creates an entry
 *  in the parent directory to reference the new file.
 *
 * @param[in]  mem        Pointer to the memory block containing file system
 *                        data.
 * @param[in]  name       The name of the new file to be created.
 * @param[in]  parent_id  The ID of the parent directory in which the file will
 *                        be created.
 * 
 * @return 
 *   The ID of the newly created file node on success. @n
 *   -1 if the file initialization failed (e.g., no free blocks available).
 */
static int _initFile(union Block *mem, char *name, int parent_id)
{
    struct Interval id_bounds = {0};
    if (!findFreeDensestBlocks(mem[BITMAP_ID].bitmap, 1, &EMPTY_INTERVAL,
                               &id_bounds)) {
        return -1;
    }
    setBitmapUsed(mem[BITMAP_ID].bitmap, &id_bounds);
    int id = id_bounds.start;
    initDirEntry(mem, name, id, parent_id);

    mem[id].file =
        (struct FileNode){0}; // // // // // // // // // // // // // //
                              // // // // May not be necessary
    strncpy(mem[id].file.name, name, NAME_MAX_LEN);
    mem[id].file.type = TYPE_FILE;
    return id;
}
