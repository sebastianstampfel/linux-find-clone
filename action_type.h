/**
* @file action_type.h
* Betriebssysteme MyFind Action-Type-Header-File
* Beispiel 1
*
* @brief Contains logic for "-type" action
* @date 2020/02/22
*
* @version 1
*/

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

#endif //MYFIND_DEBUG_ACTION_TYPE_H
