/**
 * @file heartyfs.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief  
 *  Heartyfs is a program simulating a file system with cli commands to interact
 *  with system.
 * 
 *  The main module of heartyfs.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "heartyfs.h"
#include "heartyfs_bitmap.h"
#include "heartyfs_string.h"

/* Private Functions */

static union Block *_mapDisk();
static int _unmapDisk(union Block *);
static void _createVirtualDisk();
static void _initSys(union Block *);
static void _helpCmd(char *exe);
static bool _getOpts(int argc, char *argv[], bool *opts, int *resume_idx);
static bool _isCmdMatch(char *name, const void *cmd);

#define ARG_STR_LEN 32
#define CMD_START_DEFAULT 1

enum Options { OPT_HELP, OPT_RESET, OPT_PRINT_BITMAP };
const char OPT_LIST[][ARG_STR_LEN] = {"help", "reset", "print-bitmap"};
#define OPT_LIST_LEN (int)(sizeof(OPT_LIST) / sizeof(OPT_LIST[0]))

struct Cmd {
    char name[ARG_STR_LEN];
    bool (*call)(union Block *, char *, char **, int);
};

const struct Cmd CMD_LIST[] = {
    {.name = "cd", .call = cdCmd},       {.name = "ls", .call = lsCmd},
    {.name = "pwd", .call = pwdCmd},     {.name = "mkdir", .call = mkdirCmd},
    {.name = "rmdir", .call = rmdirCmd}, {.name = "create", .call = createCmd},
    {.name = "rm", .call = rmCmd},       {.name = "read", .call = readCmd},
    {.name = "write", .call = writeCmd}};
#define CMD_LIST_LEN (int)(sizeof(CMD_LIST) / sizeof(struct Cmd))

int main(int argc, char *argv[])
{
    bool opts[OPT_LIST_LEN] = {0};
    int cmd_start;
    if (!_getOpts(argc, argv, opts, &cmd_start)) {
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (opts[OPT_RESET] || access(DISK_FILE_PATH, F_OK) != 0) {
        _createVirtualDisk();

        union Block *mem = _mapDisk();
        _initSys(mem);
        _unmapDisk(mem);
        if (setCWD(ROOT_ID) == false)
            return EXIT_FAILURE;
    }
    if (access(CWD_STORE_PATH, F_OK) != 0 && setCWD(ROOT_ID) == false) {
        return EXIT_FAILURE;
    }

    if (opts[OPT_HELP]) {
        _helpCmd(argv[0]);
        return EXIT_SUCCESS;
    }

    int status = EXIT_SUCCESS;
    char **cmd = argv + cmd_start;
    union Block *mem = _mapDisk();

    if (cmd_start < argc) {
        int idx = findStr(cmd[0], CMD_LIST, CMD_LIST_LEN, sizeof(struct Cmd),
                          _isCmdMatch);
        if (idx != -1) {
            if (!CMD_LIST[idx].call(mem, argv[0], cmd, argc - cmd_start))
                status = EXIT_FAILURE;
        } else {
            errno = EINVAL;
            fprintf(stderr, "%s: Invalid command\n", cmd[0]);
            fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
            status = EXIT_FAILURE;
        }
    } else if (cmd_start == CMD_START_DEFAULT) {
        fprintf(stderr, "No command found.\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        status = EXIT_FAILURE;
    }
    if (status == EXIT_FAILURE) {
    } else if (opts[OPT_PRINT_BITMAP]) {
        printf("\n---Bitmap---\n");
        printBitmap(mem[BITMAP_ID].bitmap);
    }
    _unmapDisk(mem);
    return status;
}

/**
 * @brief 
 *  Checks if a command matches a given name.
 * 
 * @param[in] name  Name to compare.
 * @param[in] cmd   Pointer to the command.
 * 
 * @return 
 *   true if the command name matches, false otherwise.
 */
static bool _isCmdMatch(char *name, const void *cmd)
{
    char *cmd_name = ((struct Cmd *)cmd)->name;
    return (strcmp(name, cmd_name) == 0) ? true : false;
}

/**
 * @brief
 *  Parses command-line options and sets corresponding flags.
 *
 * @param[in] argc	        Number of command-line arguments.
 * @param[in] argv	        Array of command-line arguments.
 * @param[out] opts	        Array to store the status of each option.
 * @param[out] resume_idx	Index of the first non-option argument in argv.
 *
 * @return
 *   true  : Options parsed successfully @n
 *   false : Invalid option encountered (sets errno).
 */
static bool _getOpts(int argc, char *argv[], bool *opts, int *resume_idx)
{
    int i = CMD_START_DEFAULT;
    char opt_prefix[] = "--";
    size_t preifx_len = strlen(opt_prefix);
    for (; i < argc && strncmp(argv[i], opt_prefix, preifx_len) == 0; i++) {
        bool is_valid = false;
        for (int j = 0; j < OPT_LIST_LEN; j++)
            if (strncmp(argv[i] + preifx_len, OPT_LIST[j], ARG_STR_LEN) == 0) {
                opts[j] = true;
                is_valid = true;
            }
        if (!is_valid) {
            errno = EINVAL;
            fprintf(stderr, "%s: Invalid option\n", argv[i]);
            return false;
        }
    }
    *resume_idx = i;
    return true;
}

/**
 * @brief
 *  Displays the usage and available commands for the program.
 *
 * @param[in] exe	Name of the executable.
 */
static void _helpCmd(char *exe)
{
    printf("usage: %s [options] <cmd> ...\n", exe);

    printf("Command List:\n");
    for (int i = 0; i < CMD_LIST_LEN; i++)
        printf("   %s\n", CMD_LIST[i].name);

    printf("\nOptions List:\n");
    for (int i = 0; i < OPT_LIST_LEN; i++)
        printf("   --%s\n", OPT_LIST[i]);
}

/**
 * @brief
 *  Creates a new virtual disk file of the specified size.
 *
 * @note
 *  The size of the virtual disk is defined by `DISK_SIZE`.
 */
static void _createVirtualDisk()
{
    char cmd[STR_MAX_LEN];
    sprintf(cmd, "truncate -s %d %s", DISK_SIZE, DISK_FILE_PATH);
    system(cmd);
}

/**
 * @brief
 *  Maps the virtual disk file to memory.
 *
 * @return
 *   Pointer to the mapped memory on success @n
 *   Exits the program on failure.
 */
static union Block *_mapDisk()
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

/**
 * @brief
 *  Unmaps the virtual disk from memory.
 *
 * @param[in] mem   Pointer to the mapped memory.
 *
 * @return
 *   Result of the munmap operation.
 */
static int _unmapDisk(union Block *mem) { return munmap(mem, DISK_SIZE); }

/**
 * @brief
 *  Initializes the root directory and bitmap of the virtual file system.
 *
 * @param[in, out] mem  Pointer to the mapped memory where the system is
 *                      initialized.
 */
static void _initSys(union Block *mem)
{
    memset(mem, 0, DISK_SIZE);

    strncpy(mem[ROOT_ID].dir.name, "/", NAME_MAX_LEN);
    mem[ROOT_ID].dir.type = TYPE_DIR;
    initDirEntry(mem, ".", ROOT_ID, ROOT_ID);
    initDirEntry(mem, "..", ROOT_ID, ROOT_ID);

    memset(mem[BITMAP_ID].bitmap, 0xFF, BITMAP_LEN);

    setBitmapUsed(mem[BITMAP_ID].bitmap, &(struct Interval){0, 2});
}