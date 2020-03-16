//
// Created by sebastian on 3/1/20.
//

#include "action_user.h"
#include <errno.h>
#include <error.h>

// TODO: Implement
int doActionUser(char *fileName, char *params){
    static int paramType;   // 0 = not set yet; 1 = Params is valid name; 2 = Params is valid UID

    if(paramType == 0){
        int paramCheck = checkPWFile(params);

        switch(paramCheck){
            case -1:
                return -1;
            case 0:
                paramType = 1;
                break;
            case 1:
                paramType = 2;
                break;
            default:
                return -1;
        }
    }

    struct stat sb = {0};
    errno = 0;
    if(lstat(fileName, &sb) == -1){
        error(0, errno, "Error while getting file stats");
        return -1;
    }
    long fileUid = sb.st_uid;

    if(paramType == 0){

    } else if(paramType == 1){
        char *name = getUser(fileUid);
        if(strcmp(params, name) == 0){
            return 0;
        } else {
            return 1;
        }
    } else if(paramType == 2){
        errno = 0;
        char *ptr;
        long uid = strtol(params, &ptr, 10);

        if(ptr == params){
            // total conversion fail
            return -1;
        } else if(*ptr != '\0') {
            // incomplete conversion
            return -1;
        } else {
            if(errno){
                // overflow
                return -1;
            } else {
                if(uid == (long)sb.st_uid){
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    }

    return -1;
}

/**
 * @brief Checks /etc/passwd file for supplied params
 *
 * Params are treated as a valid username first. Should no user matching the supplied
 * name be found, params is treated as an UID.
 *
 * @param params Username or uid to be checked for
 * @return -1 on error, 0 if valid name was supplied, 1 if valid uid was supplied
 */
int checkPWFile(char *params){
    int retValue = 0;

    errno = 0;
    if(getpwnam(params) == NULL){
        if(errno == EINTR || errno == EIO || errno == EMFILE || errno == ENFILE || errno == ENOMEM || errno == ERANGE){
            error(0, errno, "Error while checking for user:");
            retValue = -1;
        } else {
            errno = 0;
            char *ptr;
            long uid =  strtol(params, &ptr, 10);

            if(ptr == params){
                // total conversion fail
                error(0, errno, "User %s does not exist!", params);
                retValue = -1;
            } else if(*ptr != '\0') {
                // incomplete conversion
                error(0, errno, "User %s does not exist!", params);
                retValue = -1;
            } else {
                if(errno){
                    // overflow
                    error(0, errno, "User %s does not exist!", params);
                    retValue = -1;
                } else {
                    errno = 0;
                    if(getpwuid((uid_t)uid) == NULL){
                        if(errno == EINTR || errno == EIO || errno == EMFILE || errno == ENFILE ||
                            errno == ENOMEM || errno == ERANGE) {
                            error(0, errno, "Error while checking for user");
                            retValue = -1;
                        } else {
                            error(0, errno, "User %s does not exist!", params);
                            retValue = -1;
                        }
                    } else {
                        // valid uid was supplied
                        retValue = 1;
                    }
                }
            }
        }

    } else {
        // valid username was supplied
        retValue = 0;
    }

    return retValue;
}

char* getUser(long uid){
    char *retValue = "";
    struct passwd *pwd = getpwuid((uid_t)uid);
    if(pwd != NULL){
        retValue = pwd->pw_name;
    }
    return retValue; 
}


