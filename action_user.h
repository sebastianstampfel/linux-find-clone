//*
// @file action_user.h
// Betriebssysteme MyFind-Main-File
// Beispiel 1
//
// @author Sebastian Stampfel <ic19b084@technikum-wien.at>
// @author Milan Kollmann <ic19b058@technikum-wien.at>
// @author Benjamin Wiesbauer <ic19b096@technikum-wien.at>
// @date 2020/02/22

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

/**
 * @brief Checks /etc/passwd file for supplied params
 *
 * Params are treated as a valid username first. Should no user matching the supplied
 * name be found, params is treated as an UID.
 *
 * @param params Username or uid to be checked for
 * @return -1 on error, 0 if valid name was supplied, 1 if valid uid was supplied
 */
int checkPWFile(char *params);

/**
 * @brief Function to get the related username of a passed uid
 *
 * @param uid uid to get related username
 * @return char array containing the username on success, empty char array on failure
 */
char* getUser(long uid);


#endif //MYFIND_DEBUG_ACTION_USER_H
