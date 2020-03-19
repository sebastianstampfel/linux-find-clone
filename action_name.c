/**
* @file action_name.c
* Betriebssysteme MyFind Action-Name-File
* Beispiel 1
*
* @brief Contains logic for "-name" action
* @author Sebastian Stampfel <ic19b084@technikum-wien.at>
* @author Milan Kollmann <ic19b058@technikum-wien.at>
* @author Benjamin Wiesbauer <ic19b096@technikum-wien.at>
* @date 2020/02/22
*
* @version 1
*/

#include "action_name.h"
#include <string.h>
#include <fnmatch.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>

/**
 * @brief Function to split up the path to file to get the filename
 *
 * @param filePath path to file
 * @return char array containing filename on success, NULL on failure
 */
static char *fileNameFromPath(char *filePath);

int doActionName(char *filePath, char *params){

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

static char *fileNameFromPath(char *filePath){
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