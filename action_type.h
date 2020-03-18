//
// Created by sebastian on 3/1/20.
//

#ifndef MYFIND_DEBUG_ACTION_TYPE_H
#define MYFIND_DEBUG_ACTION_TYPE_H
#include <sys/types.h>

typedef struct type {
    int (*typeFunction)(mode_t);

}TYPESTRUCT;

int doActionType(char *filePath, char *params);
static int parseTypeParams(char *params, TYPESTRUCT *typeList);
/**
 * b = block
 * c = character
 * d = directory
 * p = named pipe
 * f = regular file
 * l = symbolic link
 * s = socket
 */
static int isBlock(mode_t mode);
static int isCharacter(mode_t mode);
static int isDirectory(mode_t mode);
static int isNamedPipe(mode_t mode);
static int isRegularFile(mode_t mode);
static int isSymbolicLink(mode_t mode);
static int isSocket(mode_t mode);

#endif //MYFIND_DEBUG_ACTION_TYPE_H
