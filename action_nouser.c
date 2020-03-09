//
// Created by sebastian on 3/1/20.
//

#include "action_nouser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>

/*
 * TODO: Implement!
 */
int doActionNoUser(char *fileName, char *params){
    // Prevent errors about unused params; Delete once function is implemented!
    //fileName = fileName;
    params = params;
    struct stat buf = {0};

    errno = 0;
    if (lstat(fileName, &buf) == -1) {
        error(0, errno, "File does not exist kek");
        return -1;
    }
    if (getpwuid(buf.st_uid) == NULL) {
        return 0;
    }
    return 1;
}
