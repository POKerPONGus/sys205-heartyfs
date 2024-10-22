#include "heartyfs.h"

void *writeBlock_HeartyFS(void *mem, void *data, int block_id)
{
    if (block_id < 0 || block_id > BLOCK_NUM) {
        errno = EFAULT;
        return NULL;
    }
    return memcpy(mem + BLOCK_SIZE * block_id, data, BLOCK_SIZE);
}