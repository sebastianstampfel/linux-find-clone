//
// Created by sebastian on 3/1/20.
//

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
