//
// Created by sebastian on 3/1/20.
//

#ifndef MYFIND_DEBUG_ACTION_TYPE_H
#define MYFIND_DEBUG_ACTION_TYPE_H
#include <sys/types.h>

typedef struct type {
    int (*typeFunction)(mode_t);

}TYPESTRUCT;


/**
 * @brief Check to see if file is of type x
 *
 * Type could be:
 * b = block device
 * c = character device
 * d = directory
 * p = named pipe
 * f = regular file
 * l = symbolic link
 * s = socket
 * 
 * @param filePath Path to the file
 * @param params Params of action
 * @return int 0 on success, 1 on failure 
 */
int doActionType(char *filePath, char *params);

/**
 * @brief Auxiliary function to parse the types
 * 
 * @param typelist List of types
 * @param params Params of type
 * @return int 0 on success, 1 on failure 
 */
int parseTypeParams(char *params, TYPESTRUCT *typeList);

int isBlock(mode_t mode);
int isCharacter(mode_t mode);
int isDirectory(mode_t mode);
int isNamedPipe(mode_t mode);
int isRegularFile(mode_t mode);
int isSymbolicLink(mode_t mode);
int isSocket(mode_t mode);

#endif //MYFIND_DEBUG_ACTION_TYPE_H
