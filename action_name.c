//
// Created by sebastian on 3/1/20.
//

#include "action_name.h"
#include <string.h>
#include <fnmatch.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>


/*
 * TODO: Implement!
 */
int doActionName(char *filePath, char *params){
    // Prevent errors about unused params; Delete once function is implemented!
    // filePath = filePath;
    // params = params;
    // ------------------------------------------------------------------------
    char *fileName = fileNameFromPath(filePath);
    if(fileName != NULL){
        errno = 0;
        int result = fnmatch(params, fileName, FNM_NOESCAPE | FNM_PERIOD);
        switch(result){
            case 0:
                // match
                free(fileName);
                return 0;
            case FNM_NOMATCH:
                // no match
                free(fileName);
                return 1;
            default:
                // error
                free(fileName);
                error(0, errno, "Error while checking for name action");
                return -1;
        }
    } else {
        free(fileName);
        return -1;
    }
}

char *fileNameFromPath(char *filePath){
    char *path = calloc(strlen(filePath)+1, sizeof(char));
    if(path != NULL){
        if(strcpy(path, filePath) == NULL){
            return NULL;
        }
    }

    char *p = strtok(path, "/");
    char *retVal = calloc(strlen(filePath), sizeof(char));

    while(p != NULL) {
        // hello
        strcpy(retVal, p);
        p = strtok(NULL, "/");
    }
    free(path);

    if(retVal != NULL){
        return retVal;
    } else {
        return NULL;
    }
}