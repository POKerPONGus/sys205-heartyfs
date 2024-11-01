#include "_heartyfs_main_utils.h"

union Block_HeartyFS *mapDisk_HeartyFS()
{
    int fd = open(DISK_FILE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Cannot open the disk file\n");
        exit(1);
    }
    void *buffer =
        mmap(NULL, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Cannot map the disk file onto memory\n");
        exit(1);
    }
    return buffer;
}

void initSys_HeartyFS(union Block_HeartyFS *mem)
{
    memset(mem, 0, DISK_SIZE);

    strncpy(mem[ROOT_ID].dir.name, "/", NAME_MAX_LEN);
    mem[ROOT_ID].dir.type = TYPE_DIR_HEARTY_FS;
    _initDirEntry(mem, ".", ROOT_ID, ROOT_ID);
    _initDirEntry(mem, "..", ROOT_ID, ROOT_ID);

    memset(mem[BITMAP_ID].bitmap, 0xFF, BITMAP_LEN);

    _updateBitmapUsed(mem[BITMAP_ID].bitmap, (struct Range){0, 2});
}

int initDir_HeartyFS(union Block_HeartyFS *mem, char *name, int parent_id)
{
    struct DirNode_HeartyFS *parent_dir = &mem[parent_id].dir;
    struct Array parent_entries = {.val = parent_dir->entries,
                                   .len = parent_dir->len,
                                   .size = sizeof(struct DirEntry_HeartyFS)};
    struct Range id_range = {0};
    if (parent_id == BITMAP_ID) {
        errno = EFAULT;
        return -1;
    } else if (parent_dir->type != TYPE_DIR_HEARTY_FS) {
        errno = ENOTDIR;
        return -1;
    } else if (parent_dir->len == DIR_MAX_ENTRIES) {
        errno = ENOMEM;
        return -1;
    } else if (_findStr(name, &parent_entries, _isDirEntryMatch) != -1) {
        errno = EINVAL;
        return -1;
    } else if (!_findFreeDensestBlocks(mem[1].bitmap, 1, NULL, &id_range)) {
        errno = ENOSPC;
        return -1;
    }
    _updateBitmapUsed(mem[BITMAP_ID].bitmap, id_range);

    int id = id_range.start;
    _initDirEntry(mem, name, id, parent_id);

    mem[id].dir =
        (struct DirNode_HeartyFS){0}; // // // // // // // // // // // // // //
                                      // // // // May not be necessary
    strncpy(mem[id].dir.name, name, NAME_MAX_LEN);
    mem[id].dir.type = TYPE_DIR_HEARTY_FS;
    _initDirEntry(mem, ".", id, id);
    _initDirEntry(mem, "..", parent_id, id);
    return id;
}

int initFile_HeartyFS(union Block_HeartyFS *mem, char *name, int parent_id)
{
    struct DirNode_HeartyFS *parent_dir = &mem[parent_id].dir;
    struct Array parent_entries = {.val = parent_dir->entries,
                                   .len = parent_dir->len,
                                   .size = sizeof(struct DirEntry_HeartyFS)};
    struct Range id_range = {0};
    if (parent_id == BITMAP_ID) {
        errno = EFAULT;
        return -1;
    } else if (parent_dir->type != TYPE_DIR_HEARTY_FS) {
        errno = ENOTDIR;
        return -1;
    } else if (parent_dir->len == DIR_MAX_ENTRIES) {
        errno = ENOMEM;
        return -1;
    } else if (_findStr(name, &parent_entries, _isDirEntryMatch) != -1) {
        errno = EINVAL;
        return -1;
    } else if (!_findFreeDensestBlocks(mem[1].bitmap, 1, NULL, &id_range)) {
        errno = ENOSPC;
        return -1;
    }
    _updateBitmapUsed(mem[BITMAP_ID].bitmap, id_range);
    int id = id_range.start;
    _initDirEntry(mem, name, id, parent_id);

    mem[id].file =
        (struct FileNode_HeartyFS){0}; // // // // // // // // // // // // // //
                                       // // // // May not be necessary
    strncpy(mem[id].file.name, name, NAME_MAX_LEN);
    mem[id].file.type = TYPE_FILE_HEARTY_FS;
    return id;
}

int _calcFileSize(union Block_HeartyFS *mem, int id)
{
    int data_id = mem[id].file.blocks[mem[id].file.len - 1];
    return (mem[id].file.len - 1) * BLOCK_MAX_DATA + mem[data_id].data.size;
}

int write_HeartyFS(union Block_HeartyFS *mem, int id, void *data, int size,
                   enum AccessModes_HeartyFS mode)
{
    struct FileNode_HeartyFS *file = &mem[id].file;
    if (id == BITMAP_ID) {
        errno = EFAULT;
        return -1;
    } else if (file->type != TYPE_FILE_HEARTY_FS) {
        errno = EISDIR;
        return -1;
    }

    int offset;
    switch (mode) {
    case WRONLY_HEARTY_FS:
        offset = 0;
        break;
    case APPEND_HEARTY_FS:
        offset = _calcFileSize(mem, id);
        break;
    default:
        errno = EINVAL;
        return -1;
    }

    struct Range block_range;
    if (FILE_MAX_SIZE - offset < (uint64_t)size) {
        errno = ENOMEM;
        return -1;
    }
    int len = _ceilDivInt(offset, BLOCK_MAX_DATA);
    int max_id = INT_MIN;
    int min_id = INT_MAX;
    for (int i = 0; i < len; i++) {
        max_id = _maxInt(max_id, file->blocks[i]);
        min_id = _minInt(min_id, file->blocks[i]);
    }
    struct Range existing_range = {.start = min_id, .len = max_id - min_id};
    if (!_findFreeDensestBlocks(mem[1].bitmap, 1, NULL, &block_range)) {
        errno = ENOSPC;
        return -1;
    }

    int curr_block = block_range.start;
    int end_block = block_range.start + block_range.len;
    while (size > 0) {
        curr_block = _findNextFreeBlock(mem[BITMAP_ID].bitmap, curr_block);
        memcpy(mem[curr_block].data.data, );
        curr_block++;
    }
}

int getNodeID_HeartyFS(union Block_HeartyFS *mem, char rel_path[], int start_id)
{
    if (rel_path[0] == '/') {
        errno = EINVAL;
        return -1;
    }
    int id = start_id;
    char *ptr = rel_path;
    char *substr = NULL;
    while (_splitStr(&substr, '/', &ptr)) {
        if (mem[id].dir.type != TYPE_DIR_HEARTY_FS)
            break;

        struct Array entries = {.val = mem[id].dir.entries,
                                .len = mem[id].dir.len,
                                .size = sizeof(struct DirEntry_HeartyFS)};
        int idx = _findStr(substr, &entries, _isDirEntryMatch);
        if (idx != -1) {
            id = mem[id].dir.entries[idx].block_id;
        } else {
            id = -1;
            break;
        }
    }
    return id;
}