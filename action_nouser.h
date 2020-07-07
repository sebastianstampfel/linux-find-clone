/**
* @file action_nouser.h
* Betriebssysteme MyFind Action-Nouser-Header-File
* Beispiel 1
*
* @brief Contains logic for "-nouser" action
* @date 2020/02/22
*
* @version 1
*/

#ifndef MYFIND_DEBUG_ACTION_NOUSER_H
#define MYFIND_DEBUG_ACTION_NOUSER_H


/**
 * @brief Check if owner of file exists in /etc/passwd
 * 
 * @param fileName Name of the file
 * @param params Params of action
 * @return int 0 on success, 1 on failure 
 */
int doActionNoUser(char *fileName, char *params);

#endif //MYFIND_DEBUG_ACTION_NOUSER_H
