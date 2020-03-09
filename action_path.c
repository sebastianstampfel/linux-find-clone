//
// Created by sebastian on 3/1/20.
//

#include "action_path.h"
#include <fnmatch.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>

int doActionPath(char *filePath, char *params){

    // Prevent errors about unused params; Delete once function is implemented!
    filePath = filePath;
    params = params;
    // ------------------------------------------------------------------------
    if(filePath != NULL){
        errno = 0;
        int result = fnmatch(params, filePath, FNM_NOESCAPE | FNM_PERIOD);
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

    return 1;
}
