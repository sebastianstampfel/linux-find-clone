//*
// @file main.c
// Betriebssysteme MyFind-Main-File
// Beispiel 1
//
// @author Sebastian Stampfel <ic19b084@technikum-wien.at>
// @author Milan Kollmann <ic19b058@technikum-wien.at>
// @author Benjamin Wiesbauer <ic19b096@technikum-wien.at>
// @date 2020/02/22
//
// @version 1
//
// @todo Test it more seriously and more complete.
// @todo Review it for missing error checks.
// @todo Review it and check the source against the rules at
//       https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/c-rules.html
//

// -------------------------------------------------------------- includes --

#include "action.h"
#include "action_user.h"
#include "action_ls.h"
#include "action_name.h"
#include "action_nouser.h"
#include "action_type.h"
#include "action_path.h"
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// --------------------------------------------------------------- defines --
// -------------------------------------------------------------- typedefs --
// --------------------------------------------------------------- globals --
int FLAG_PRINT = 0;
int FLAG_LS = 0;
int FLAG_NOUSER = 0;
int FLAG_PRINT_ONLY = 1;
int FLAG_STD_DIRS_PRINTED = 0;
int ACTION_COUNT = 0;

// ------------------------------------------------------------- functions --
int doDir(char *dir_name, ACTION *listHead);
int doFile(char * fileName, ACTION *listHead);
int parseParams(int argc, const char *argv[], ACTION *listHead, char **startDir);
ACTION *addListEntry(ACTION *listHead, int type, const char *params);
void cleanupList(ACTION *listHead);
int printEntry(char *fileName);

int main(int argc, const char *argv[])
{
    // prevent warnings regarding unused params
    argc = argc;
    argv = argv;

    ACTION *listHead = calloc(1, sizeof(ACTION));
    if(listHead == NULL){
        error(1, errno, "Out of memory!\n");
    }
    listHead->prev = NULL;
    listHead->next = NULL;
    char *startdir;
    if(parseParams(argc, argv, listHead, &startdir) != 0){
        error(0, errno, "%s: Error while parsing params.\n", argv[0]);
        cleanupList(listHead);
        exit(EXIT_FAILURE);
    }

    doDir(startdir, listHead);

    cleanupList(listHead);
    free(startdir);
    return 0;
}

int doDir(char *dirName, ACTION *listHead){
    DIR *dirStream = opendir(dirName);
    if(dirStream == NULL){
        // Rest of error message is coming from errno
        error(0, errno, "Directory %s", dirName);
        return 1;
    }
    struct dirent *dirContent = readdir(dirStream);
    while(dirContent != NULL){
        if(dirContent->d_type == DT_DIR){
            int newPathLength = strlen(dirName) + strlen(dirContent->d_name) + 2; // 2 = "/" + '\0'
            // VLA
            char *newPath = calloc(newPathLength, sizeof(char));
            if(newPath == NULL){
                error(1, errno, "Out of memory!");
            }
            // evtl. snprintf()
            if(strcat(newPath, dirName) == NULL){
                error(1, errno, "Out of memory!");
                free(newPath);
                break;
            }
            if(strcmp(dirName, "/") != 0){
                if(strcat(newPath, "/") == NULL){
                    error(1, errno, "Out of memory!");
                    free(newPath);
                    break;
                }
            }
            if(strcat(newPath, dirContent->d_name) == NULL){
                error(1, errno, "Out of memory!");
                free(newPath);
                break;
            }
            *(newPath + (newPathLength - 1)) = '\0';

            // Do not investigate on directories "." and ".."
            if(strcmp(".", dirContent->d_name) == 0 || strcmp("..", dirContent->d_name) == 0){
                if(FLAG_STD_DIRS_PRINTED == 0 && FLAG_PRINT_ONLY){
                    if(printEntry(dirContent->d_name) != 0){
                        error(0, errno, "Error while printing to stdout.");
                        return 1;
                    }
                    FLAG_STD_DIRS_PRINTED = 1;
                }
            } else {
                if(FLAG_PRINT == 1 && FLAG_PRINT_ONLY == 1){
                    if(printEntry(newPath) != 0){
                        error(0, errno, "Error while printing to stdout.");
                        return 1;
                    }

                }

                doDir(newPath, listHead);
                free(newPath);
            }

        } else if(dirContent->d_type == DT_REG){
            //printf("%s, REG\n", dir_content->d_name);
            int newPathLength = (strlen(dirName) + strlen(dirContent->d_name))+2;
            char *filePath = calloc(newPathLength, sizeof(char));
            if(filePath == NULL){
                error(1, errno, "Out of memory!");
            }
            if(strcat(filePath, dirName) == NULL){
                error(1, errno, "Out of memory!");
                free(filePath);
                break;
            }
            if(strcat(filePath, "/") == NULL){
                error(1, errno, "Out of memory!");
                free(filePath);
                break;
            }
            if(strcat(filePath, dirContent->d_name) == NULL){
                error(1, errno, "Out of memory!");
                free(filePath);
                break;
            }
            *(filePath + newPathLength -1) = '\0';
            doFile(filePath, listHead);
            free(filePath);
        } else if(dirContent->d_type == DT_UNKNOWN){
            printf("%s, UNKOWN\n", dirContent->d_name);
        } else {
            // TODO: Still print to stdout!
            printf("%s, VERY UNKNOWN TYPE\n", dirContent->d_name);
        }

        // errno rücksetzen
        dirContent = readdir(dirStream);
    }

    // errorhandling readdir => errno ansehen

    // errorhandling oder kommentar warum nicht nötig
    closedir(dirStream);

    return 0;
}

int doFile(char  *fileName, ACTION *listHead){
    if(FLAG_PRINT == 1 && FLAG_PRINT_ONLY == 1){
        if(printf("%s\n", fileName) < 0){
            return 1;
        }
    } else {
        // Iterate through action list and call function pointer
        ACTION *current = listHead;
        int matchedActions = 0;

        while(1){
            int retVal = (*current->actionFunction)(fileName, current->param);
            if(retVal < 0){
                error(0, errno, "Something bad happened, idk what.");
            } else if(retVal == 0){
                matchedActions++;
            }

            if(current->next != NULL){
                current = current->next;
            } else {
                break;
            }
        }

        if(matchedActions == ACTION_COUNT){
            printEntry(fileName);
        }
    }

    return 0;
}

int parseParams(int argc, const char *argv[], ACTION *listHead, char **startDir){

    if(argc <= 1){
        fprintf(stderr, "%s: Not enough arguments provided.\n", argv[0]);
        return 1;
    } else {
        if(strcmp(argv[1], "./") == 0){
            *startDir = calloc(2, sizeof(char)); // warum calloc()? => array
            if(*startDir == NULL){
                fprintf(stderr, "%s: Error while allocating memory.\n", argv[0]);
                return 1;
            }
            **startDir = '.';
            *(*(startDir) + 1) = '\0';
        } else {
            *startDir = calloc(strlen(argv[1]) + 1, sizeof(char));
            if(*startDir == NULL){
                fprintf(stderr, "%s: Error while allocating memory.\n", argv[0]);
                return 1;
            }
            if(strcpy(*startDir, argv[1]) == NULL){
                return 1;
            }
        }

        if(argc == 2){
            FLAG_PRINT = 1;
            return 0;
        }

        for(int i = 2; i < argc; i ++){
            if(strcmp(argv[i], "-user") == 0){
                if(addListEntry(listHead, USER, argv[i + 1]) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else if(strcmp(argv[i], "-name") == 0){
                if(addListEntry(listHead, NAME, argv[i + 1]) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else if(strcmp(argv[i], "-type") == 0){
                if(addListEntry(listHead, TYPE, argv[i + 1]) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    break;
                };
                i++;
            } else if(strcmp(argv[i], "-print") == 0){
                FLAG_PRINT = 1;
            } else if(strcmp(argv[i], "-ls") == 0){
                FLAG_LS = 1;
            } else if(strcmp(argv[i], "-nouser") == 0){
                FLAG_NOUSER = 1;
                ACTION_COUNT++;
            } else if(strcmp(argv[i], "-path") == 0){
                if(addListEntry(listHead, PATH, argv[i + 1]) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else {
                fprintf(stderr, "%s: %s is not a valid argument.\n", argv[0], argv[i]);
                fprintf(stderr, "You should see usage instructions now. If they were implemented already...\n");
                return 1;
            }
        }
    }

    return 0;
}

ACTION *addListEntry(ACTION *listHead, int type, const char *params){
    if(FLAG_PRINT_ONLY != 0){
        FLAG_PRINT_ONLY = 0;    // Action(s) different to print was provided as arguments
    }
    ACTION *currentEntry = listHead;

    if(currentEntry != NULL && currentEntry->prev == NULL && currentEntry->next == NULL && currentEntry->param == NULL){
        currentEntry->type = type;
        if(params != NULL){
            currentEntry->param = calloc(strlen(params)+1, sizeof(char));
            if(currentEntry->param != NULL){
                if(strcpy(currentEntry->param, params) == NULL){
                    return NULL;
                }
                switch(type) {
                    case USER:
                        currentEntry->actionFunction = &doActionUser;
                        break;
                    case NAME:
                        currentEntry->actionFunction = &doActionName;
                        break;
                    case LS:
                        currentEntry->actionFunction = &doActionLs;
                        break;
                    case NOUSER:
                        currentEntry->actionFunction = &doActionNoUser;
                        break;
                    case TYPE:
                        currentEntry->actionFunction = &doActionType;
                        break;
                    case PATH:
                        currentEntry->actionFunction = &doActionPath;
                        break;
                    default:
                        error(1, errno, "Unknown type!\n");;
                        currentEntry->actionFunction = NULL;
                        break;
                }
                return currentEntry;
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
    } else {
        while (1) {
            if (currentEntry->next != NULL) {
                currentEntry = currentEntry->next;
            } else {
                break;
            }
        }

        currentEntry->next = calloc(1, sizeof(ACTION));
        if(currentEntry->next == NULL){
            return NULL;
        }

        currentEntry->next->prev = currentEntry;
        currentEntry->next->next = NULL;
        currentEntry->next->type = type;
        if (params != NULL) {
            currentEntry->next->param = calloc(strlen(params)+1, sizeof(char));
            if(currentEntry->next->param == NULL){
                return NULL;
            }
            if(strcpy(currentEntry->next->param, params) == NULL){
                return NULL;
            }
        } else {
            return NULL;
        }
        switch(type){
            case USER:
                currentEntry->next->actionFunction = &doActionUser;
                break;
            case NAME:
                currentEntry->next->actionFunction = &doActionName;
                break;
            case LS:
                currentEntry->next->actionFunction = &doActionLs;
                break;
            case NOUSER:
                currentEntry->next->actionFunction = &doActionNoUser;
                break;
            case TYPE:
                currentEntry->next->actionFunction = &doActionType;
                break;
            case PATH:
                currentEntry->next->actionFunction = &doActionPath;
                break;
            default:
                error(1, errno, "Unknown type!\n");;
                currentEntry->next->actionFunction = NULL;
                break;
        }

        return currentEntry->next;
    }
}

void cleanupList(ACTION *listHead){
    ACTION *current = listHead;

    while(1){
        if(current->next != NULL){
            current = current->next;
            free(current->prev->param);
            free(current->prev);
        } else {
            free(current->param);
            free(current);
            break;
        }
    }
}

int printEntry(char *fileName){
    if(FLAG_LS == 1){
       // complex printout required
       return 0;
    } else {
        if(printf("%s\n", fileName) < 0){
            return 1;
        } else {
            return 0;
        }
    }
}
