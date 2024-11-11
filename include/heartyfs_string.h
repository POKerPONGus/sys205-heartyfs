/**
 * @file heartyfs_math.h
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  A header for simple string operations for heartyfs.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#ifndef _HEARTYFS_STRING_UTILS_H
#define _HEARTYFS_STRING_UTILS_H

#include <stdbool.h>

#include "heartyfs_helper_structs.h"

/**
 * @brief 
 *  Splits a string into substrings by a delimiter.
 * 
 *  Modifies the original string by replacing each occurrence of the delimiter
 *  with `\0`. Updates the current pointer to the next segment or to the end of
 *  the string.
 *
 * @param[out]      substr      Pointer to the next substring in the input
 *                              string.
 * @param[in]       delim       Delimiter character for splitting.
 * @param[in, out]  curr_ptr    Pointer to the current position in the string
 *                              being split.
 * 
 * @return 
 *  `true` if a substring was found, `false` if end of string was reached.
 */
bool splitStr(char **substr, char delim, char **curr_ptr);

/**
 * @brief 
 *  Finds a target string in an array based on a match function.
 *
 *  Iterates through an array of elements and applies the `is_match` function to
 *  check if the target matches an element in the array.
 *
 * @param[in] target    String to search for.
 * @param[in] arr       Pointer to the array to search.
 * @param[in] len       Number of elements in the array.
 * @param[in] size      Size of each element in the array.
 * @param[in] is_match  Function pointer that returns `true` if the target
 *                      matches an element.
 * 
 * @return 
 *  Index of the target if found, `-1` otherwise.
 */
int findStr(char *target, const void *arr, int len, int size,
            bool is_match(char *, const void *));


/**
 * @brief 
 *  Extracts the base name from a path.
 *
 *  Copies the base name from the given path into a buffer. If the path does not
 *  contain any '/', the entire path is copied into the buffer.
 *
 * @param[in]   path    The path string.
 * @param[out]  buf     Buffer to store the base name.
 * @param[in]   buf_len Length of the buffer.
 */
void parseBasename(char path[], char *buf, int buf_len);

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
void parseDir(char path[], char *buf, int buf_len);

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
char parseOpt(char **start_arg, int len, int *idx);
#endif