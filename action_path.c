/**
* @file action_path.c
* Betriebssysteme MyFind Action-Path-File
* Beispiel 1
*
* @date 2020/02/22
*
* @version 1
*/

#include "action_path.h"
#include <fnmatch.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>

int doActionPath(char *filePath, char *params){

    // Prevent errors about unused params; Delete once function is implemented!
    filePath = filePath;
    params = params;
    // ------------------------------------------------------------------------
    if(filePath != NULL && params != NULL){
        const int pathLength = strlen(params);

        if(*(params + (pathLength - 1)) == '/'){
            error(0, errno, "Path %s will not match anything because it ends with /.", params);
            return -1;
        }

        errno = 0;
        int result = fnmatch(params, filePath, 0);
        switch(result){
            case 0:
                // match
                return 0;
            case FNM_NOMATCH:
                // no match
                return 1;
            default:
                // error
                error(0, errno, "Error while checking for name action");
                return -1;
        }
    } else {
        return -1;
    }
}
