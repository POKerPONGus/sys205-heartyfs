#include "_heartyfs_main_utils.h"
#include "_heartyfs_math_utils.h"
#include "_heartyfs_string_utils.h"

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

    _updateBitmapUsed(mem[BITMAP_ID].bitmap, &(struct Interval){0, 2});
}

int initDir_HeartyFS(union Block_HeartyFS *mem, char *name, int parent_id)
{
    struct DirNode_HeartyFS *parent_dir = &mem[parent_id].dir;
    struct Array parent_entries = {.val = parent_dir->entries,
                                   .len = parent_dir->len,
                                   .size = sizeof(struct DirEntry_HeartyFS)};
    struct Interval id_bounds = {0};
    if (parent_dir->type != TYPE_DIR_HEARTY_FS) {
        errno = ENOTDIR;
        return -1;
    } else if (parent_dir->len == DIR_MAX_ENTRIES) {
        errno = ENOMEM;
        return -1;
    } else if (_findStr(name, &parent_entries, _isDirEntryMatch) != -1) {
        errno = EINVAL;
        return -1;
    } else if (!_findFreeDensestBlocks(mem[BITMAP_ID].bitmap, 1, NULL,
                                       &id_bounds)) {
        errno = ENOSPC;
        return -1;
    }
    _updateBitmapUsed(mem[BITMAP_ID].bitmap, &id_bounds);

    int id = id_bounds.start;
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
    struct Interval id_bounds = {0};
    if (parent_dir->type != TYPE_DIR_HEARTY_FS) {
        errno = ENOTDIR;
        return -1;
    } else if (parent_dir->len == DIR_MAX_ENTRIES) {
        errno = ENOMEM;
        return -1;
    } else if (_findStr(name, &parent_entries, _isDirEntryMatch) != -1) {
        errno = EINVAL;
        return -1;
    } else if (!_findFreeDensestBlocks(mem[BITMAP_ID].bitmap, 1, NULL,
                                       &id_bounds)) {
        errno = ENOSPC;
        return -1;
    }
    _updateBitmapUsed(mem[BITMAP_ID].bitmap, &id_bounds);
    int id = id_bounds.start;
    _initDirEntry(mem, name, id, parent_id);

    mem[id].file =
        (struct FileNode_HeartyFS){0}; // // // // // // // // // // // // // //
                                       // // // // May not be necessary
    strncpy(mem[id].file.name, name, NAME_MAX_LEN);
    mem[id].file.type = TYPE_FILE_HEARTY_FS;
    return id;
}

int writeFile_HeartyFS(union Block_HeartyFS *mem, int id, void *data, int size,
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

    struct Interval curr_bounds, *curr_bounds_ptr;
    int file_size;
    switch (mode) {
    case WRONLY_HEARTY_FS:
        file_size = 0;

        if (FILE_MAX_SIZE < size) {
            errno = ENOMEM;
            return -1;
        }
        curr_bounds_ptr = NULL;
        _deleteFileData(mem, id);
        break;
    case APPEND_HEARTY_FS:
        file_size = _calcFileSize(mem, id);

        if (FILE_MAX_SIZE - file_size < size) {
            errno = ENOMEM;
            return -1;
        }
        curr_bounds = _intArrInterval(file->blocks, file->len);
        curr_bounds_ptr = &curr_bounds;
        break;
    default:
        errno = EINVAL;
        return -1;
    }

    int new_len = _ceilDivInt(file_size + size, BLOCK_MAX_DATA) - file->len;
    struct Interval block_bounds;
    if (!_findFreeDensestBlocks(mem[BITMAP_ID].bitmap, new_len - file->len,
                                curr_bounds_ptr, &block_bounds)) {
        errno = ENOSPC;
        return -1;
    }

    int curr_block;
    uint8_t *data_ptr = data;
    if (file_size > 0) {
        curr_block = file->blocks[file->len - 1];
        
        int size_wrote =
            _writeDataBlock(&mem[curr_block].data, data_ptr, size, mode);
        size -= size_wrote;
        data_ptr += size_wrote;
    }
    curr_block = block_bounds.start;
    for (int i = file->len; size > 0 && curr_block < block_bounds.end; i++) {
        file->blocks[i] = curr_block;

        int size_wrote =
            _writeDataBlock(&mem[curr_block].data, data_ptr, size, mode);
        size -= size_wrote;
        data_ptr += size_wrote;
        curr_block = _findNextFreeBlock(mem[BITMAP_ID].bitmap, curr_block + 1);
    }
    _updateBitmapUsed(mem[BITMAP_ID].bitmap, &block_bounds);
    file->len = new_len;
    return id;
}

int read_HeartyFS(union Block_HeartyFS *mem, int id, void *buf, int size,
                  int *offset)
{
    struct FileNode_HeartyFS *file = &mem[id].file;
    if (id == BITMAP_ID) {
        errno = EFAULT;
        return -1;
    } else if (file->type != TYPE_FILE_HEARTY_FS) {
        errno = EISDIR;
        return -1;
    }
    uint8_t *buf_ptr = buf;
    for (int i = *offset / BLOCK_MAX_DATA; i < file->len && size > 0; i++) {
        struct DataBlock_HeartyFS *data_block = &mem[file->blocks[i]].data;
        int block_offset = *offset % BLOCK_MAX_DATA;

        uint8_t *block_ptr = data_block->data + block_offset;
        int size_read = _minInt(size, data_block->size - block_offset);
        memcpy(buf_ptr, block_ptr, size_read);
        *offset += size_read;
        buf_ptr += size_read;
        size -= size_read;
    }
    return 0;
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