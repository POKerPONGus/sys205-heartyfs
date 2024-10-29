#include "_private_heartyfs_utils.h"

union Block_HeartyFS *mapDisk_HeartyFS(int mode)
{
    int mmap_flags;
    switch (mode) {
    case RDONLY_HEARTY_FS:
        mmap_flags = PROT_READ;
        break;
    case WRONLY_HEARTY_FS:
        mmap_flags = PROT_WRITE;
        break;
    case RDWR_HEARTY_FS:
        mmap_flags = PROT_READ | PROT_WRITE;
        break;
    default:
        errno = EINVAL;
        return NULL;
    }
    int fd = open(DISK_FILE_PATH, mode);
    if (fd < 0) {
        perror("Cannot open the disk file\n");
        exit(1);
    }
    void *buffer = mmap(NULL, DISK_SIZE, mmap_flags, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Cannot map the disk file onto memory\n");
        exit(1);
    }
    return buffer;
}

int initDir_HeartyFS(union Block_HeartyFS *mem, char *name, int id,
                     int parent_id)
{
    struct DirNode_HeartyFS *parent_dir = &mem[parent_id].dir;
    struct Array parent_entries = {.val = parent_dir->entries,
                                   .len = parent_dir->len,
                                   .size = sizeof(struct DirEntry_HeartyFS)};
    if (parent_dir->type != TYPE_DIR_HEARTY_FS) {
        errno = ENOTDIR;
        return -1;
    } else if (parent_dir->len == DIR_MAX_ENTRIES) {
        errno = ENOSPC;
        return -1;
    } else if (_findStr(name, &parent_entries, _isDirEntryMatch) != -1) {
        errno = EINVAL;
        return -1;
    }
    _initDirEntry(mem, name, id, parent_id);

    strncpy(mem[id].dir.name, name, NAME_MAX_LEN);
    mem[id].dir.type = TYPE_DIR_HEARTY_FS;
    _initDirEntry(mem, ".", id, id);
    _initDirEntry(mem, "..", parent_id, id);
    return id;
}

// int write_HeartyFS(char path[], void *data, int size, uint8_t mode)
// {
//     switch (mode) {
//     case WRONLY_HEARTY_FS:
//         break;
//     case APPEND_HEARTY_FS:
//         break;
//     default:
//         errno = EINVAL;
//         return -1;
//     }

//     void *buffer = mapDisk_HeartyFS(mode);
// }

#define STR_BOOL(b) (b) ? "true" : "false"

int main()
{
    // uint8_t map[BITMAP_LEN] = {0};
    // memset(map, 0xFF, BITMAP_LEN);
    // int i;
    // for (i = 0; i < BITMAP_LEN / 4; i++) {
    //     map[i] = 0x00;
    // }
    // map[i] = 0xCC;
    // int bounds[2] = {0};
    // unsigned int min = _findFreeDensestBlocks(map, 18, &bounds);
    // printf("%u, %d, %d\n", min, bounds[0], bounds[1]);
    union Block_HeartyFS *buf = mapDisk_HeartyFS(RDWR_HEARTY_FS);
    memset(buf, 0, DISK_SIZE);

    buf[0].dir.type = TYPE_DIR_HEARTY_FS;
    if (initDir_HeartyFS(buf, "/", 0, 0)) {
        perror("Cannot map initialize root block\n");
        exit(1);
    }
    memset(buf[2].bitmap, 0xFF, BITMAP_LEN);

    char path[] = "./../poop/poopinit/../../../poop/../you/are/so/";

    if (initDir_HeartyFS(buf, "you", 7, 0) == -1) {
        perror("Oh no!");
    }
    if (initDir_HeartyFS(buf, "poop", 8, 0) == -1) {
        perror("Oh no!");
    }
    if (initDir_HeartyFS(buf, "poopinit", 9, 8) == -1) {
        perror("Oh no!");
    }
    if (initDir_HeartyFS(buf, "are", 3, 7) == -1) {
        perror("Oh no!");
    }
    if (initDir_HeartyFS(buf, "so", 4, 3) == -1) {
        perror("Oh no!");
    }
    if (initDir_HeartyFS(buf, "dun", 6, 3) == -1) {
        perror("Oh no!");
    }
    int n = 0;
    printf("type %d, name %s\n", buf[n].dir.type, buf[n].dir.name);
    for (int i = 0; i < buf[n].dir.len; i++) {
        printf("e#%d: %s\n", i, buf[n].dir.entries[i].name);
    }
    int id = _getNodeID(buf, path, 7);
    if (id != -1)
        printf("res: %d, %s\n", id, buf[id].dir.name);
}
// [1010 1010]
// [0101 0101]