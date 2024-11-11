/**
 * @file heartyfs.h
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  Heartyfs is a program simulating a file system with cli commands to interact
 *  with system.
 *
 *  The main header of heartyfs containing system related functions, structs,
 *  constants, and the user commands.
 *
 * @version 0.1
 * @date 2024-11-11
 */
#ifndef HEARTYFS_H
#define HEARTYFS_H

#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#define DISK_FILE_PATH "/tmp/heartyfs"
#define CWD_STORE_PATH ".heartyfs_cwd"

#define BLOCK_SIZE (1 << 9)
#define DISK_SIZE (1 << 20)
#define BLOCK_COUNT (DISK_SIZE / BLOCK_SIZE)

#define STR_MAX_LEN 200

#define BLOCK_MAX_DATA (int)(BLOCK_SIZE - sizeof(int))

struct DataBlock {
    int size;
    uint8_t data[BLOCK_MAX_DATA];
};

enum InodeTypes { TYPE_FILE = 0, TYPE_DIR = 1 };

#define NAME_MAX_LEN 28

struct DirEntry {
    char name[NAME_MAX_LEN];
    int block_id;
};

#define DIR_MAX_ENTRIES 14
#define SELF_REF_ENTRY_IDX 0
#define PARENT_DIR_ENTRY_IDX 1

struct DirNode {
    char name[NAME_MAX_LEN];
    uint8_t type;
    int len;
    struct DirEntry entries[DIR_MAX_ENTRIES];
};

#define FILE_MAX_BLOCKS 119
#define FILE_MAX_SIZE (FILE_MAX_BLOCKS * BLOCK_MAX_DATA)

struct FileNode {
    char name[NAME_MAX_LEN];
    uint8_t type;
    int len;
    int blocks[FILE_MAX_BLOCKS];
};

#define BITMAP_LEN (BLOCK_COUNT / CHAR_BIT)
#define ROOT_ID 0
#define BITMAP_ID 1

union Block {
    struct FileNode file;
    struct DirNode dir;
    struct DataBlock data;
    uint8_t bitmap[BITMAP_LEN];
};

enum AccessModes { WRONLY, APPEND };

/* Command Functions */

/**
 * @brief 
 *  Changes the current working directory based on the given path.
 *
 * @note 
 *  If the number of command arguments is not equal to 2, the function prints a
 *  usage message and returns false. The function also handles failure cases
 *  where the node ID for the directory cannot be obtained or if changing the
 *  current working directory fails.
 *
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 * 
 * @return 
 *   true  : Successfully changed the working directory. @n
 *   false : Failed to change the working directory or incorrect usage.
 */
bool cdCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);


/**
 * @brief 
 *  Lists the contents of a directory, printing the names of its entries.
 *
 * @note 
 *  If the number of command arguments is greater than 2, the function prints a
 *  usage message and returns false. If a valid path is provided, the function
 *  attempts to retrieve the node ID for the directory. If the path corresponds
 *  to a valid directory, the entries are printed. If the path is not a
 *  directory, an error is raised. If no path is provided, the current working
 *  directory is listed.
 *
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 *
 * @return 
 *   true  : Successfully listed the directory entries. @n 
 *   false : Failed to list the directory due to invalid arguments or errors.
 */
bool lsCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);

/**
 * @brief 
 *  Prints the absolute path of the current working directory.
 * 
 * @note 
 *  If the number of command arguments is not equal to 1, the function will
 *  print a usage message and return false. The function attempts to retrieve
 *  the current working directory ID and calls the helper function to print the
 *  absolute path.
 *
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 * 
 * @return 
 *   true  : Successfully printed the current working directory. @n
 *   false : Failed to print the working directory (e.g., unable to retrieve       
 *           current directory).
 */
bool pwdCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);

/**
 * @brief 
 *  Creates a new directory at the specified path.
 *
 * @note 
 *  If the number of command arguments is not equal to 2, the function prints a
 *  usage message and returns false. If the parent directory is full or the
 *  directory already exists, an appropriate error message is displayed. The
 *  function attempts to initialize a new directory if all checks pass. The
 *  directory will be created with the special entries for `.` (current
 *  directory) and `..` (parent directory).
 *
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 *
 * @return 
 *   true  : The directory was successfully created. @n 
 *   false : Failed to create the directory due to errors (e.g., invalid  
 *           arguments, full directory, or existing directory).
 */
bool mkdirCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);

/**
 * @brief 
 *  Removes a directory from the file system.
 * 
 * @note 
 *  This function checks if the correct number of command arguments is provided. 
 *  It verifies that the directory exists, is not the current working directory, 
 *  and is empty. If these conditions are met, the directory is deleted by 
 *  calling the helper function `_deleteDir`.
 * 
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 * 
 * @return 
 *   true  : Successfully removed the directory. @n
 *   false : Failed to remove the directory (e.g., invalid path, directory not 
 *           empty, etc.).
 */
bool rmdirCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);

/**
 * @brief 
 *  Creates a new file in the specified directory, based on the provided file
 *  path.
 *
 * @note 
 *  If the number of command arguments is not exactly 2, the function will print
 *  a usage message and return false. The function checks for a valid parent
 *  directory, ensures the directory is not full, verifies that a file with the
 *  same name doesn't already exist, and then initializes the new file.
 *
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 *
 * @return 
 *   true  : The file was successfully created. @n
 *   false : Failed to create the file due to errors (e.g., invalid arguments, 
 *           full directory, file already exists, or file initialization 
 *           failure).
 */
bool createCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);

/**
 * @brief 
 *  Removes a file from the file system.
 * 
 * @note 
 *  This function checks if the correct number of command arguments is provided.
 *  It retrieves the parent directory of the file and verifies if the specified
 *  path corresponds to a valid file. If it is a valid file, it deletes the file
 *  by calling the helper function `_deleteFile`.
 * 
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 * 
 * @return 
 *   true  : Successfully removed the file. @n
 *   false : Failed to remove the file (e.g., invalid file path, directory 
 *           instead of file, etc.).
 */
bool rmCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);

/**
 * @brief 
 *  Reads the contents of a file and prints them to standard output.
 *
 * @note 
 *  If the number of command arguments is not equal to 2, the function will
 *  print a usage message and return false. The function retrieves the node ID
 *  for the specified file path, checks if it is a valid file, and then reads
 *  the file in chunks (of size `READ_BUF_SIZE`). The contents are printed to
 *  the standard output using `fwrite` until the entire file is read.
 *
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 * 
 * @return 
 *   true  : Successfully read and printed the file contents. @n
 *   false : Failed to read or print the file contents (e.g., invalid file path, 
 *           directory instead of file, etc.).
 */
bool readCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);

/**
 * @brief 
 *  Writes data to a file in the file system, either from standard input or
 *  another file.
 * 
 * @note 
 *  This function supports two modes of writing:
 *  1. Overwriting a file (`-w` or default mode) - This mode will replace the
 *     existing content of the file.
 *  2. Appending to a file (`-a`) - This mode will append the data to the
 *     existing content of the file. The function handles reading from standard
 *     input or from a specified file path. If the file system's file size limit
 *     is exceeded during writing, an error will be set (`ENOMEM`).
 * 
 * @param[in]  mem      Pointer to the memory block containing file system data.
 * @param[in]  exe_path The executable path for displaying the usage message.
 * @param[in]  cmd      Array of command arguments.
 * @param[in]  cmd_len  The length of the command argument array.
 * 
 * @return 
 *   true  : Data successfully written to the file. @n
 *   false : Failed to write data (e.g., invalid path, size exceeds limit, 
 *           etc.).
 */
bool writeCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len);

#define GETNODEID_USE_CWD -2

/**
 * @brief 
 *  Retrieves the ID of a node (file or directory) from its path.
 * 
 * @param[in] mem         Memory block representing the file system.
 * @param[in] path        Path to the node.
 * @param[in] start_id    Starting ID; use GETNODEID_USE_CWD to start from the current directory.
 * 
 * @return 
 *   Node ID if successful, -1 if the node does not exist.
 */
int getNodeID(union Block *mem, char path[], int start_id);

/* System Utility Functions */

/**
 * @brief 
 *  Retrieves the ID of the parent directory of a given path.
 * 
 * @param[in] mem   Memory block representing the file system.
 * @param[in] path  Path to the directory.
 * 
 * @return 
 *   Parent directory ID if successful, -1 otherwise.
 */
int getParentID(union Block *mem, char path[]);

/**
 * @brief 
 *  Sets the current working directory.
 * 
 * @param[in] cwd_id  ID of the new current working directory.
 * 
 * @return 
 *   true if successful, false otherwise.
 */
bool setCWD(int cwd_id);

/**
 * @brief 
 *  Retrieves the current working directory ID.
 * 
 * @return 
 *   ID of the current working directory.
 */
int getCWD();

/**
 * @brief 
 *  Checks if a directory entry matches a given name.
 * 
 * @param[in] name    Name to compare.
 * @param[in] entry   Pointer to the directory entry.
 * 
 * @return 
 *   true if the entry name matches, false otherwise.
 */
bool isDirEntryMatch(char *name, const void *entry);

/**
 * @brief 
 *  Initializes a new directory entry within a parent directory.
 * 
 * @param[in, out] mem        Memory block representing the file system.
 * @param[in]      name       Name of the new directory entry.
 * @param[in]      id         ID of the new entry.
 * @param[in]      parent_id  ID of the parent directory.
 */
void initDirEntry(union Block *mem, char *name, int id, int parent_id);

/**
 * @brief 
 *  Deletes an entry from a parent directory by ID.
 * 
 * @param[in, out] parent_dir  Pointer to the parent directory.
 * @param[in]      id          ID of the entry to delete.
 */
void deleteParentDirEntry(struct DirNode *parent_dir, int id);

/**
 * @brief 
 *  Calculates the total file size based on its data blocks.
 * 
 * @param[in] mem  Memory block representing the file system.
 * @param[in] id   ID of the file.
 * 
 * @return 
 *   Total file size in bytes.
 */
int calcFileSize(union Block *mem, int id);

/**
 * @brief 
 *  Deletes all data blocks associated with a file, marking them free in the bitmap.
 * 
 * @param[in, out] mem  Memory block representing the file system.
 * @param[in]      id   ID of the file to delete.
 */
void deleteFileData(union Block *mem, int id);

/**
 * @brief 
 *  Writes data to a data block with size constraints.
 * 
 * @param[in, out] d_block     Pointer to the data block.
 * @param[in]      size_used   Amount of space already used in the block.
 * @param[in]      data        Data to write to the block.
 * @param[in]      size        Size of the data to write.
 * 
 * @return 
 *   Number of bytes written to the block.
 */
int writeDataBlock(struct DataBlock *d_block, int size_used, void *data,
                   int size);

/**
 * @brief 
 *  Reads data from a file by ID into a buffer, starting from an offset.
 * 
 * @param[in]       mem     Memory block representing the file system.
 * @param[in]       id      ID of the file to read.
 * @param[out]      buf     Buffer to store the read data.
 * @param[in]       size    Size of the buffer.
 * @param[in, out]  offset  Offset to start reading from and updated as reading progresses.
 * 
 * @return 
 *   Number of bytes read.
 */
int readFileID(union Block *mem, int id, void *buf, int size, int *offset);

/**
 * @brief 
 *  Reads data from a file specified by its path.
 * 
 * @param[in]     mem   Memory block representing the file system.
 * @param[in]     path  Path to the file.
 * @param[out]    buf   Pointer to store the file data buffer.
 * @param[out]    size  Pointer to store the size of the data.
 * 
 * @return 
 *   true if successful, false otherwise.
 */
bool readFilePath(union Block *mem, char *path, char **buf, int *size);

/**
 * @brief 
 *  Displays the bitmap as rows of binary values.
 * 
 * @param[in] bitmap  Pointer to the bitmap to print.
 */
void printBitmap(uint8_t *bitmap);
#endif