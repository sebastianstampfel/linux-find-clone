//
// Created by sebastian on 3/1/20.
//

#include "action_user.h"
#include <errno.h>


// TODO: Implement
int doActionUser(char *fileName, char *params){
    int retValue = 0;
    if(checkPWFile(params) != 0){
        fprintf(stderr, "user: \"%s\" does not exist\n", params);
        exit(EXIT_FAILURE);
    }
    else{
        struct stat sb = {0};
        errno = 0; 
        if(lstat(fileName, &sb) == -1){
            fprintf(stderr, "Something bad happened while using lstat():\n%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else{
            long file_uid = sb.st_uid;
            char *name = getUser(file_uid);    
            if(strcmp(params, name) != 0){

//                long uid = -1;
//                if(sscanf(params, "%ld", &uid) == 1){
//                    if(uid != (long)sb.st_uid){
//                        retValue = 1;
//                    }
//                }
//                else{
//                    retValue = 1;
//                }

                errno = 0;
                char *ptr;
                long uid =  strtol(params, &ptr, 10);

                if(ptr == params){
                    // total conversion fail
                    retValue = 1;
                } else if(*ptr != '\0') {
                    // incomplete conversion
                    retValue = 1;
                } else {
                    if(errno){
                        // overflow
                        retValue = 1;
                    } else {
                        if(uid != (long)sb.st_uid){
                            retValue = 1;
                        }
                    }
                }
            }
        }
    }
    
    return retValue;
}

int checkPWFile(char *params){
    int retValue = 0;
    if(getpwnam(params) == NULL){
        errno = 0;
        char *ptr;
        long uid =  strtol(params, &ptr, 10);

        if(ptr == params){
            // total conversion fail
            retValue = 1;
        } else if(*ptr != '\0') {
            // incomplete conversion
            retValue = 1;
        } else {
            if(errno){
                // overflow
                retValue = 1;
            } else {
                if(getpwuid((uid_t)uid) == NULL){
                    retValue = 1;
                }
            }
        }
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


