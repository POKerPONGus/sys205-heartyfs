/**
 * @file heartyfs_read.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's read command on the command line.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "heartyfs.h"
#include "heartyfs_math.h"

#define READ_BUF_SIZE 128

bool readCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
{
    if (cmd_len != 2) {
        printf("usage: %s %s <file-path>\n", exe_path, cmd[0]);
        return false;
    }
    int id = getNodeID(mem, cmd[1], GETNODEID_USE_CWD);
    if (id == -1) {
        return false;
    } else if (mem[id].file.type != TYPE_FILE) {
        errno = EISDIR;
        perror(cmd[1]);
        return false;
    }

    int offset = 0;
    char buf[READ_BUF_SIZE];
    int size_read;
    do {
        size_read = readFileID(mem, id, buf, READ_BUF_SIZE, &offset);
        fwrite(buf, sizeof(char), size_read, stdout);
    } while (size_read == READ_BUF_SIZE);
    printf("\n");

    return false;
}
