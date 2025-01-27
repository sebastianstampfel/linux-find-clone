/**
* @file action_type.c
* Betriebssysteme MyFind Action-Type-File
* Beispiel 1
*
* @date 2020/02/22
*
* @version 1
*/

#include "action_type.h"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <stdio.h>


static int TYPES_TOTAL = 0;

static int parseTypeParams(char *params, TYPESTRUCT *typeList);
static int isBlock(mode_t mode);
static int isCharacter(mode_t mode);
static int isDirectory(mode_t mode);
static int isNamedPipe(mode_t mode);
static int isRegularFile(mode_t mode);
static int isSymbolicLink(mode_t mode);
static int isSocket(mode_t mode);

int doActionType(char *filePath, char *params){
    int returnValue = 0;

    if(params == NULL){
        returnValue = -1;
        return returnValue;
    }
    const int length = strlen(params);
    TYPES_TOTAL = length - ((length - 1)/2);
    TYPESTRUCT types[TYPES_TOTAL];

    if(parseTypeParams(params, types) != 0){
        returnValue = -1;
        fprintf(stderr, "Error while parsing params for -type.\n");
        return returnValue;
    }

    struct stat buf;
    errno = 0;
    if(lstat(filePath, &buf) == -1){
        returnValue = -1;
        error(0, errno, "Error while inspecting file.");
        return returnValue;
    }

    for(int i = 0; i < TYPES_TOTAL; i++){
        if(types[i].typeFunction(buf.st_mode) == 0){
            return returnValue;
        }
    }

    returnValue = 1;
    return returnValue;
}

/**
 * @brief Auxiliary function to parse the types
 *
 * @param typelist List of types
 * @param params Params of type
 * @return int 0 on success, 1 on failure
 */
static int parseTypeParams(char *params, TYPESTRUCT *typeList){
    char tmpParams[strlen(params)];
    if(strcpy(tmpParams, params) == NULL){
        fprintf(stderr, "Error while parsing params for -type.\n");
        return -1;
    }

    char *p = strtok(tmpParams, ",");
    for(int i = 0; i < TYPES_TOTAL; i++){
        if(p == NULL){
            fprintf(stderr, "%s is an invalid param chain for -type!\n", params);
            return -1;
        }
        TYPESTRUCT *current = typeList + i;

        switch(*p){
            case 'b':
                current->typeFunction = &isBlock;
                break;
            case 'c':
                current->typeFunction = &isCharacter;
                break;
            case 'd':
                current->typeFunction = &isDirectory;
                break;
            case 'p':
                current->typeFunction = &isNamedPipe;
                break;
            case 'f':
                current->typeFunction = &isRegularFile;
                break;
            case 'l':
                current->typeFunction = &isSymbolicLink;
                break;
            case 's':
                current->typeFunction = &isSocket;
                break;
            default:
                fprintf(stderr, "%s is an invalid parameter chain for -type!\n", params);
                return -1;
        }
        p = strtok(NULL, ",");
    }

    return 0;
}

static int isBlock(mode_t mode){
    if(S_ISBLK(mode) != 0){
        return 0;
    } else {
        return 1;
    }
}
static int isCharacter(mode_t mode){
    if(S_ISCHR(mode) != 0){
        return 0;
    } else {
        return 1;
    }
}
static int isDirectory(mode_t mode){
    if(S_ISDIR(mode) != 0){
        return 0;
    } else {
        return 1;
    }
}
static int isNamedPipe(mode_t mode){
    if(S_ISFIFO(mode) != 0){
        return 0;
    } else {
        return 1;
    }
}
static int isRegularFile(mode_t mode){
    if(S_ISREG(mode) != 0){
        return 0;
    } else {
        return 1;
    }
}
static int isSymbolicLink(mode_t mode){
    if(S_ISLNK(mode) != 0){
        return 0;
    } else {
        return 1;
    }
}
static int isSocket(mode_t mode){
    if(S_ISSOCK(mode) != 0){
        return 0;
    } else {
        return 1;
    }
}
