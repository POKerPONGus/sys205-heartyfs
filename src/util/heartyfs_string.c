/**
 * @file heartyfs_string.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing the string helper functions.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "heartyfs_string.h"
#include "heartyfs_math.h"

bool splitStr(char **substr, char delim, char **curr_ptr)
{
    if (**curr_ptr == '\0')
        return false;
    *substr = *curr_ptr;
    while (**curr_ptr != '\0' && **curr_ptr != delim) {
        (*curr_ptr)++;
    }
    if (**curr_ptr == delim) {
        **curr_ptr = '\0';
        (*curr_ptr)++;
    }
    return true;
}

int findStr(char *target, const void *arr, int len, int size,
            bool is_match(char *, const void *))
{
    const uint8_t *buf = arr;
    for (int i = 0; i < len; i++) {
        if (is_match(target, (const void *)(buf + size * i))) {
            return i;
        }
    }
    return -1;
}

void parseBasename(char path[], char *buf, int buf_len)
{
    int i = strlen(path);
    for (; i > 0 && path[i] != '/'; i--)
        ;
    if (path[i] == '/')
        i++;
    strncpy(buf, path + i, buf_len);
}

/**
 * @brief 
 *  Extracts the directory portion from a path.
 *
 *  Copies the directory part from the given path into a buffer, excluding the
 *  base name. If no '/' is found, copies "." to the buffer, representing the
 *  current directory.
 *
 * @param[in]   path    The path string.
 * @param[out]  buf     Buffer to store the directory portion.
 * @param[in]   buf_len Length of the buffer.
 */
void parseDir(char path[], char *buf, int buf_len)
{
    int len = strlen(path);
    int new_len = 0;
    for (int i = 0; i < len; i++)
        if (path[i] == '/')
            new_len = i + 1;
    if (new_len != 0) {
        strncpy(buf, path, minInt(new_len, buf_len));
        buf[new_len] = '\0';
    } else {
        strncpy(buf, ".", buf_len);
    }
}

/**
 * @brief 
 *  Parses a single-character option from the command-line arguments.
 *
 *  Iterates through arguments starting from `*start_arg` to find a
 *  single-character option. Updates the index to the next argument.
 *
 * @param[in]       start_arg   Array of arguments to parse.
 * @param[in]       len         Number of arguments.
 * @param[in, out]  idx         Index in the arguments array, updated to the
 *                              next argument.
 *
 * @return 
 *  The found option character, or '\0' if none is found.
 */
char parseOpt(char **start_arg, int len, int *idx)
{
    char **arg = start_arg;
    char opt = '\0';
    while (*idx < len) {
        if (arg[*idx][0] != '-') {
            break;
        } else if (strlen(arg[*idx]) == 2) {
            opt = arg[*idx][1];
            (*idx)++;
            break;
        }
        (*idx)++;
    }
    return opt;
}