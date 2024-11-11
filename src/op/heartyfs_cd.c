/**
 * @file heartyfs_cd.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's cd command on the command line.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <stdio.h>
#include <stdlib.h>

#include "heartyfs.h"

#define MAX_INT_DIGIT 10

bool cdCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
{
    if (cmd_len != 2) {
        printf("usage: %s %s <dir-path>\n", exe_path, cmd[0]);
        return false;
    }

    int id = getNodeID(mem, cmd[1], GETNODEID_USE_CWD);
    
    if (id == -1 || !setCWD(id))
        return false;
    return true;
}