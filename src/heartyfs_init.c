#include "heartyfs.h"

int main()
{
    // Open the disk file & Map the disk file onto memory
    union Block_HeartyFS *buffer = mapDisk_HeartyFS(RDWR_HEARTY_FS);
    // TODO:
    memset(buffer, 0, DISK_SIZE);

    buffer[0].dir.type = TYPE_DIR_HEARTY_FS;
    if (initDir_HeartyFS(buffer, "/", 0, 0)) {
        perror("Cannot map initialize root block\n");
        exit(1);
    }
    memset(buffer[2].bitmap, 0xFF, BITMAP_LEN);
}
