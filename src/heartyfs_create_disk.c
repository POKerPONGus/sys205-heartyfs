#include <stdlib.h>
#include <stdio.h>
#include "heartyfs.h"

#define CMD_LEN 40

int main()
{
    char cmd[CMD_LEN];
    sprintf(cmd, "truncate -s %d %s", DISK_SIZE, DISK_FILE_PATH);
    system(cmd);
    return 0;
}