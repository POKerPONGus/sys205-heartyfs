#include "heartyfs.h"

int main()
{
    // Open the disk file
    int fd = open(DISK_FILE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Cannot open the disk file\n");
        exit(1);
    }

    // Map the disk file onto memory
    void *buffer =
        mmap(NULL, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Cannot map the disk file onto memory\n");
        exit(1);
    }
    // TODO:
    struct IONode_HeartyFS root = {.name = "/", .type = TYPE_DIR_HeartyFS};
    if (writeBlock_HeartyFS(buffer, &root, 0) == NULL) {
        perror("Cannot map initialize root block\n");
        exit(1);
    }
}
