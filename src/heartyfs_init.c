#include "_private_heartyfs_utils.h"

int main()
{
    // Open the disk file & Map the disk file onto memory
    union Block_HeartyFS *buffer = mapDisk_HeartyFS();
    // TODO:
    initSys_HeartyFS(buffer);
}
