#include <stdbool.h>
#include <stdint.h>
#include "_heartyfs_string_utils.h"

bool _splitStr(char **substr, char delim, char **curr_ptr)
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

int _findStr(char *target, struct Array *arr, bool is_match(char *, void *))
{
    uint8_t *buf = arr->val;
    for (int i = 0; i < arr->len; i++) {
        if (is_match(target, (void *)(buf + arr->size * i))) {
            return i;
        }
    }
    return -1;
}