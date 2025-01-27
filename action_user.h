/**
* @file action_user.h
* Betriebssysteme MyFind Action-User-Header-File
* Beispiel 1
* 
* @brief Contains logic for "-user" action
* @date 2020/02/22
*
* @version 1
*/

#ifndef MYFIND_DEBUG_ACTION_USER_H
#define MYFIND_DEBUG_ACTION_USER_H

// -------------------------------------------------------------- includes --
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>


// ------------------------------------------------------------- functions --

/**
 * @brief Function to check if the passed user owns the passed file/directory 
 *
 * If the function is called the first time, it checks if the passed username or
 * uid exists. If not int -1 will be returned and the program will be terminated.
 * 
 * If the username or uid exists, the functions compare the passed username or
 * uid with the owner of the passed file or directory
 *  
 * @param fileName path to file or directory
 * @param params username or uid 
 * @return int 0 on success, 1 if passed user does not own the file or directory
 *         -1 on failure 
 */
int doActionUser(char *fileName, char *params);


#endif //MYFIND_DEBUG_ACTION_USER_H
