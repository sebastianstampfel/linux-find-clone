//
// Created by sebastian on 3/1/20.
//

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
