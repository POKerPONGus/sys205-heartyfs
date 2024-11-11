/**
 * @file heartyfs_pwd.c
 * @author Sarutch Supaibulpipat (Pokpong) {ssupaibu@cmkl.ac.th}
 * @brief 
 *  The module implementing heartyfs's pwd command on the command line.
 * 
 * @version 0.1
 * @date 2024-11-11
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "heartyfs.h"

#define MAX_INT_DIGIT 10

struct StackNode {
    char *name;
    struct StackNode *prev;
};

static void _freeAllStackNode(struct StackNode *head);
static void _printAbsPath(union Block *mem, int start_id) ;

bool pwdCmd(union Block *mem, char *exe_path, char **cmd, int cmd_len)
{
    if (cmd_len != 1) {
        printf("usage: %s %s\n", exe_path, cmd[0]);
        return false;
    }
    int cwd_id = getCWD();
    if (cwd_id != -1) {
        _printAbsPath(mem, cwd_id);
    } else {
        return false;
    }
    return true;
}

/**
 * @brief 
 *  Frees all nodes in the stack.
 * 
 * @note 
 *  This function iterates through the stack and frees each node. It is used to
 *  clean up dynamically allocated memory for the stack nodes after their usage.
 * 
 * @param[in]  head  Pointer to the head of the stack to be freed.
 */
static void _freeAllStackNode(struct StackNode *head)
{
    struct StackNode *curr_node = head;
    while (curr_node != NULL) {
        struct StackNode *tmp = curr_node;
        curr_node = curr_node->prev;
        free(tmp);
    }
}

/**
 * @brief 
 *  Prints the absolute path of a given node in the file system.
 * 
 * @note 
 *  This function iterates through the directory structure starting from the
 *  specified node ID and prints the path from the root to the given node. It
 *  uses a stack to store directory names and prints them in reverse order once
 *  the root is reached.
 * 
 * @param[in]  mem        Pointer to the memory block containing file system
 *                        data.
 * @param[in]  start_id   The ID of the starting directory node for the path.
 */
static void _printAbsPath(union Block *mem, int start_id) 
{
    if (start_id == ROOT_ID) {
        printf("/");
        return;
    }
    struct StackNode *curr_node = NULL;
    int id = start_id;
    while (id != ROOT_ID) {
        struct StackNode *new_node = malloc(sizeof(struct StackNode));
        if (new_node == NULL) {
            perror(__func__);
            _freeAllStackNode(curr_node);
            return;
        }
        new_node->name = mem[id].dir.name;
        new_node->prev = curr_node;
        curr_node = new_node;
        id = mem[id].dir.entries[PARENT_DIR_ENTRY_IDX].block_id;
    }
    while (curr_node != NULL) {
        printf("/%s", curr_node->name);
        struct StackNode *tmp = curr_node;
        curr_node = curr_node->prev;
        free(tmp);
    }
    printf("\n");
}