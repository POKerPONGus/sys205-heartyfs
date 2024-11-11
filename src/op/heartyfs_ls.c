/**
 * @file heartyfs_ls.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's ls command on the command line.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "heartyfs.h"

#define MAX_INT_DIGIT 10

static void _printDirEntries(struct DirNode *dir);

bool lsCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
{
    char curr_dir[] = ".";
    char *path;
    if (cmd_len > 2) {
        printf("usage: %s %s [dir-path]\n", exe_path, cmd[0]);
        return false;
    } else if (cmd_len == 2) {
        path = cmd[1];
    } else {
        path = curr_dir;
    }

    int id = getNodeID(mem, path, GETNODEID_USE_CWD);
    if (id == -1) {
    } else if (mem[id].dir.type != TYPE_DIR) {
        errno = ENOTDIR;
        perror(path);
    } else {
        _printDirEntries(&mem[id].dir);
        return true;
    }
    return false;
}

static void _printDirEntries(struct DirNode *dir)
{
    for (int i = PARENT_DIR_ENTRY_IDX + 1; i < dir->len; i++) {
        printf("%s\t", dir->entries[i].name);
    }
    printf("\n");
}