/**
* @file action_user.h
* Betriebssysteme MyFind Action-User-Header-File
* Beispiel 1
* 
* @brief Contains logic for "-user" action
* @author Sebastian Stampfel <ic19b084@technikum-wien.at>
* @author Milan Kollmann <ic19b058@technikum-wien.at>
* @author Benjamin Wiesbauer <ic19b096@technikum-wien.at>
* @date 2020/02/22
*
* @version 1
*/

#ifndef MYFIND_DEBUG_ACTION_USER_H
#define MYFIND_DEBUG_ACTION_USER_H

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int doActionUser(char *fileName, char *params);
int checkPWFile(char *params);
char* getUser(long uid);


#endif //MYFIND_DEBUG_ACTION_USER_H
