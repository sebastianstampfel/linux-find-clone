//
// Created by sebastian on 3/1/20.
//

#include "action_name.h"
#include <string.h>
#include <fnmatch.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>



int doActionName(char *filePath, char *params){

    printf("%d\n", FNM_NOESCAPE);
    printf("%d\n", FNM_PATHNAME);
    printf("%d\n", FNM_PERIOD);

    exit(EXIT_FAILURE);
    char *fileName = fileNameFromPath(filePath);
    if(fileName != NULL && params != NULL){
        errno = 0;
        int result = fnmatch(params, fileName, 0);
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