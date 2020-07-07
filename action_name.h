/**
* @file action_name.h
* Betriebssysteme MyFind Action-Name-Header-File
* Beispiel 1
*
*
* @date 2020/02/22
*
* @version 1
*/

#ifndef MYFIND_DEBUG_ACTION_NAME_H
#define MYFIND_DEBUG_ACTION_NAME_H

/**
 * @brief Function to check if the passed file (filePath) contains the passed string (params)
 * 
 * @param filePath path to file
 * @param params string to be searched for 
 * @return int 0 on match, 1 on no match, -1 on error
 */
int doActionName(char *filePath, char *params);

#endif //MYFIND_DEBUG_ACTION_NAME_H
