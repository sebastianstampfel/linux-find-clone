//*
// @file action_user.c
// Betriebssysteme MyFind-Main-File
// Beispiel 1
//
// @author Sebastian Stampfel <ic19b084@technikum-wien.at>
// @author Milan Kollmann <ic19b058@technikum-wien.at>
// @author Benjamin Wiesbauer <ic19b096@technikum-wien.at>
// @date 2020/02/22
//
//

// -------------------------------------------------------------- includes --

#include "action_user.h"
#include <errno.h>
#include <error.h>


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

    } else if(paramType == 1){      //checks if passed username is owner of file
        char *name = getUser(fileUid);
        if(strcmp(params, name) == 0){
            return 0;
        } else {
            return 1;
        }
    } else if(paramType == 2){      //tries to convert char array (passed user) to long (uid)
        errno = 0;                  //and checks if uid is owner of file
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

static int checkPWFile(char *params){
    int retValue = 0;

    errno = 0;
    if(getpwnam(params) == NULL){ //if username does not exist
        if(errno == EINTR || errno == EIO || errno == EMFILE || errno == ENFILE || errno == ENOMEM || errno == ERANGE){
            error(0, errno, "Error while checking for user:");
            retValue = -1;
        } else {        //tries to convert char array (passed user) to long (uid)
            errno = 0;  //and checks if uid exists 
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

static char* getUser(long uid){
    char *retValue = "";
    struct passwd *pwd = getpwuid((uid_t)uid); 
    if(pwd != NULL){ //get the related username if uid exists
        retValue = pwd->pw_name;
    }
    return retValue; 
}


