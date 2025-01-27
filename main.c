/**
* @file main.c
* Betriebssysteme MyFind-Main-File
* Beispiel 1
*
* @date 2020/02/22
*
* @version 1
*/

/* -------------------------------------------------------------- includes -- */

#include "action.h"
#include "action_user.h"
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
#include <grp.h>
#include <time.h>
#include <sys/sysmacros.h>

/* --------------------------------------------------------------- defines -- */
/** 
 * @def SUCCESS
 * @brief Macro for return value in case of successful computation.
 */
#define SUCCESS 0
/**
 * @def WARNING
 * @brief Macro for return value in case of errors. Program may continue.
 */
#define WARNING 1
/**
 * @def CRITICAL
 * @brief Macro for return value in case of critical errors. Program should
 *          be suspended safely.
 * @see cleanupList
 */
#define CRITICAL -1
/**
 * @def FLAG_STPOINT
 * @brief Flag to indicate passing of starting point to _doDir()_.
 * @see doDir
 */
#define FLAG_STPOINT 1
/**
 * @def FLAG_PRINTENTRY_PRINT
 * @brief Flag to indicate to _printEntry()_ to do a non-detailed print
 *          to stdout.
 * @see printEntry
 */
#define FLAG_PRINTENTRY_PRINT 1
/**
 * @def FLAG_PRINTENTRY_LS
 * @brief Flag to indicate to _printEntry()_ to do detailed print
 *          to stdout.
 * @see printEntry
 */
#define FLAG_PRINTENTRY_LS 2

/* --------------------------------------------------------------- globals -- */

/**
 * @brief GLOBAL - Indicates if -print was the only action supplied as an argument.
 * 0: Actions other than -print were provided, 1: -print is only action supplied.
 * Defaults to 1, in case no argument other than a start-path was provided.
 */
static int FLAG_PRINT_ONLY = 1;

/**
 * @brief GLOBAL - Tracks if "." and ".." have been printed to stdout
 * 0: not printed yet, 1: printed
 */
static int FLAG_STD_DIRS_PRINTED = 0;

/**
 * @brief GLOBAL - Total count of actions supplied as command line arguments.
 */
static int ACTION_COUNT = 0;

/* ------------------------------------------------------------- functions -- */

/**
 * @brief Prints usage instructions to stdout
 * 
 * Prints usage instructions to stdout _and exits the application afterwards_.
 * 
 * @param argv Argument vector
 * @return int CRITICAL in case of error
 */
static int printUsage(char *argv[]);
/**
 * @brief Function to perform actions on file
 * 
 * Iterates through the action list and performs checks. If 
 * 
 * @param fullPath Absolute or relative path to file
 * @param listHead Head of action list
 * @see ACTION
 * @return int SUCCESS, WARNING, CRITICAL
 */
static int checkFile(char *fullPath, ACTION *listHead);

/**
 * @brief Function to handle a directory on the filesystem.
 *
 * Travels down the dir path until the end. Recursively calls
 * itself if more dirs are foun.
 * 
 * @param dir_name Name of the directory
 * @param listHead Head of doubly linked list of action struct
 * @param flags Indicates if dir provided is starting point.
*                 (FLAG_STPOINT) 
 * @see action
 * @see FLAG_STPOINT
 * @return int SUCCESS, WARNING, CRITICAL
 */
static int doDir(char *dir_name, ACTION *listHead, int flags);

/**
 * @brief Function to handle a file on the filesystem.
 * 
 * @param fileName Path to the file
 * @param listHead Head of doubly linked list of action struct
 * @see action
 * @return int SUCCESS, WARNING, CRITICAL
 */
static int doFile(char * fileName, ACTION *listHead);

/**
 * @brief Parses params on application start
 * 
 * Iterates through argument vector and creates a doubly linked
 * list of actions.
 * 
 * @see action
 * @param argc Argument count from main()
 * @param argv Argument vector from main()
 * @param listHead Head of doubly linked list of action struct
 * @param startDir Directory to start at
 * @return int 0 on success, 1 on failure
 */
static int parseParams(int argc, char *argv[], ACTION *listHead, 
                        char **startDir);

/**
 * @brief Adds an action to doubly linked list. Calles by parseParams().
 * 
 * @see parseParams
 * @param listHead Head of doubly linked list
 * @param type Type of action
 * @param params Params of action
 * @return ACTION* Added action
 */
static ACTION *addListEntry(ACTION *listHead, int type, const char *params);

/**
 * @brief Frees all memory alloc'd for list of actions
 * 
 * @param listHead Start of action list
 * @see action
 */
static void cleanupList(ACTION *listHead);

/**
 * @brief Prints a certain file or directory to stdout
 * 
 * Behaviour is dependet on wether or not -ls was provided as a command
 * line argument.
 * 
 * @param fileName Path to file to be printed
 * @return int 0 on success, 1 on failure
 */
static int printEntry(char *fileName, int flags);

int main(int argc, char *argv[])
{
    int returnValue = SUCCESS;

    ACTION *listHead = calloc(1, sizeof(ACTION));
    if(listHead == NULL){
        error(1, errno, "Out of memory!\n");
    }
    listHead->prev = NULL;
    listHead->next = NULL;
    listHead->type = 0;

    char *startdir;
    if(parseParams(argc, argv, listHead, &startdir) != 0){
        error(0, errno, "%s: Error while parsing params.\n", argv[0]);
        cleanupList(listHead);
        exit(EXIT_FAILURE);
    }

    struct stat buf;
    errno = 0;
    if(lstat(startdir, &buf) == -1){
        error(0, errno, "Error reading starting point");
        returnValue = CRITICAL;
        cleanupList(listHead);
        return returnValue;
    }

    // If user supplies a valid directory as starting point,
    // travel down the dir path.
    // Else, treat as a single file.
    if(S_ISDIR(buf.st_mode) != 0){
        if(doDir(startdir, listHead, FLAG_STPOINT) != SUCCESS){
            returnValue = CRITICAL;
            cleanupList(listHead);
            free(startdir);
            return returnValue;
        }
    } else {
        if(doFile(startdir, listHead) != SUCCESS){
            returnValue = CRITICAL;
            cleanupList(listHead);
            free(startdir);
            return returnValue;
        }
    }


    cleanupList(listHead);
    free(startdir);

    return 0;
}

static int doDir(char *dirName, ACTION *listHead, int flags){
    int returnValue = SUCCESS;

    DIR *dirStream = NULL;

    if(flags == FLAG_STPOINT){
        struct stat buf;
        errno = 0;
        if(lstat(dirName, &buf) == -1){
            error(0, errno, "Error reading %s", dirName);
            returnValue = WARNING;
            goto CLEANEXIT_DODIR;
        }

        if(S_ISDIR(buf.st_mode) != 0){
            // do actions
            if(listHead->type == LS){
                if(strcmp(".", dirName) != 0 && strcmp("..", dirName) != 0){
                    FLAG_STD_DIRS_PRINTED = 1;
                    if(printEntry(dirName, FLAG_PRINTENTRY_LS) != 0){
                        error(0, errno, "Error while printing to stdout.");
                        returnValue = WARNING;
                        goto CLEANEXIT_DODIR;
                    }
                } else {
                    if (printEntry(dirName, FLAG_PRINTENTRY_LS) != 0) {
                        error(0, errno, "Error while printing to stdout.");
                        returnValue = WARNING;
                        goto CLEANEXIT_DODIR;
                    }
                }
            } else if(listHead->type == PRINT){
                if(strcmp(".", dirName) != 0 && strcmp("..", dirName) != 0){
                    FLAG_STD_DIRS_PRINTED = 1;
                    if(printEntry(dirName, FLAG_PRINTENTRY_PRINT) != 0){
                        error(0, errno, "Error while printing to stdout.");
                        returnValue = WARNING;
                        goto CLEANEXIT_DODIR;
                    }
                } else {
                    if (printEntry(dirName, FLAG_PRINTENTRY_PRINT) != 0) {
                        error(0, errno, "Error while printing to stdout.");
                        returnValue = WARNING;
                        goto CLEANEXIT_DODIR;
                    }
                }
            } else {
                if(checkFile(dirName, listHead) != SUCCESS){
                    returnValue = WARNING;
                    goto CLEANEXIT_DODIR;
                }
            }
        }
    }

    dirStream = opendir(dirName);
    if(dirStream == NULL){
        // Rest of error message is coming from errno
        error(0, errno, "Directory %s", dirName);
        returnValue = WARNING;
        goto CLEANEXIT_DODIR;
    }

    struct dirent *dirContent = readdir(dirStream);
    while(1){
        const int pathLength = strlen(dirName) + strlen(dirContent->d_name) + 2;
        char fullPath[pathLength];

        if(strcmp(dirName, "/") != 0){
            if(snprintf(fullPath, pathLength, "%s/%s", dirName, dirContent->d_name) >= pathLength){
                // output truncated
                error(0, errno, "Error while building new file path");
                returnValue = CRITICAL;
                goto CLEANEXIT_DODIR;
            }
        } else {
            if(snprintf(fullPath, pathLength, "/%s", dirContent->d_name) >= pathLength){
                // output truncated
                error(0, errno, "Error while building new file path");
                returnValue = CRITICAL;
                goto CLEANEXIT_DODIR;
            }
        }

        struct stat buf;
        errno = 0;
        if(lstat(fullPath, &buf) == -1){
            error(0, errno, "Error reading %s", fullPath);
            returnValue = WARNING;
            goto CLEANEXIT_DODIR;
        }

        if(S_ISDIR(buf.st_mode) != 0){
            // do actions
            if(strcmp(".", dirContent->d_name) == 0 || strcmp("..", dirContent->d_name) == 0){
                goto CONTINUE_DODIR;
            } else {
                if (checkFile(fullPath, listHead) != SUCCESS) {
                    returnValue = WARNING;
                    goto CLEANEXIT_DODIR;
                }
            }

            const int retVal = doDir(fullPath, listHead, 0);
            if(retVal == CRITICAL){
                returnValue = CRITICAL;
                goto CLEANEXIT_DODIR;
            } else if(retVal == WARNING){
                goto CONTINUE_DODIR;
            }

        } else {
            int ret = doFile(fullPath, listHead);

            if(ret < 0){
                // critical
                returnValue = CRITICAL;
                goto CLEANEXIT_DODIR;
            } else if(ret > 0){
                // warning
                returnValue = WARNING;
                goto CLEANEXIT_DODIR;
            }
        }

        CONTINUE_DODIR:
        errno = 0;
        dirContent = readdir(dirStream);
        if(dirContent == NULL && errno != 0){
            error(0, errno, "Error while reading directory!");
            returnValue = CRITICAL;
            goto CLEANEXIT_DODIR;
        } else if(dirContent == NULL){
            break;
        }
    }

    CLEANEXIT_DODIR:
    if(dirStream != NULL){
        errno = 0;
        if(closedir(dirStream) != 0){
            error(0, errno, "Error closing directorystream");
            returnValue = CRITICAL;
        }
    }

    return returnValue;
}

static int checkFile(char *fullPath, ACTION *listHead){
    ACTION *current = listHead;
    int matchedActions = 0;
    int checksPerformed = 0;
    int printCount = 0;

    while(1){
        if(*current->actionFunction != NULL){
            int retVal = (*current->actionFunction)(fullPath, current->param);
            if(retVal < 0){
                return CRITICAL;
            } else if(retVal == 0){
                checksPerformed++;
                matchedActions++;
            } else {
                break;
            }
        } else if(current->type == PRINT && (matchedActions == checksPerformed)){
            if(current->next != NULL){
                if(current->next->type == PRINT || current->next->type == LS){
                    if(printEntry(fullPath, FLAG_PRINTENTRY_PRINT) != 0){
                        return WARNING;
                    } else {
                        printCount++;
                    }
                } else {
                    if(checksPerformed != 0){
                        if(printEntry(fullPath, FLAG_PRINTENTRY_PRINT) != 0){
                            return WARNING;
                        } else {
                            printCount++;
                        }
                    } else if(current == listHead){
                         if(printEntry(fullPath, FLAG_PRINTENTRY_PRINT) != 0){
                             return WARNING;
                         } else {
                            printCount++;
                         }
                    }
                }
            } else {
                if(printEntry(fullPath, FLAG_PRINTENTRY_PRINT) != 0){
                    return WARNING;
                } else {
                    printCount++;
                }
            }
        } else if(current->type == LS && (matchedActions == checksPerformed)){
            if(current->next != NULL){
                if(current->next->type == PRINT || current->next->type == LS){
                    if(printEntry(fullPath, FLAG_PRINTENTRY_LS) != 0){
                        return WARNING;
                    } else {
                        printCount++;
                    }
                } else {
                    if(checksPerformed != 0){
                        if(printEntry(fullPath, FLAG_PRINTENTRY_LS) != 0){
                            return WARNING;
                        } else {
                            printCount++;
                        }
                    } else if(current == listHead){
                        if(printEntry(fullPath, FLAG_PRINTENTRY_PRINT) != 0){
                            return WARNING;
                        } else {
                            printCount++;
                        }
                    }
                }
            } else {
                if(printEntry(fullPath, FLAG_PRINTENTRY_LS) != 0){
                    return WARNING;
                } else {
                    printCount++;
                }
            }
        }

        if(current->next != NULL){
            current = current->next;
        } else {
            break;
        }
    }

    if((matchedActions == ACTION_COUNT) && printCount == 0){
        if(printEntry(fullPath, FLAG_PRINTENTRY_PRINT) != 0){
            return WARNING;
        }
    }

    return SUCCESS;
}

static int doFile(char  *fileName, ACTION *listHead){
    int returnValue = 0;

    if(checkFile(fileName, listHead) != SUCCESS){
        returnValue = WARNING;
    }

    return returnValue;
}

static int parseParams(int argc, char *argv[], ACTION *listHead, char **startDir){
    int returnValue = SUCCESS;

    if(argc <= 1){
        fprintf(stderr, "%s: Not enough arguments provided.\n", argv[0]);
        printUsage(argv);
        returnValue = CRITICAL;
    } else {
        if(strcmp(argv[1], "./") == 0){
            *startDir = calloc(2, sizeof(char));
            if(*startDir == NULL){
                fprintf(stderr, "%s: Error while allocating memory.\n", argv[0]);
                returnValue = CRITICAL;
                goto EXIT_PARSEPARAMS;
            }
            **startDir = '.';
            *(*(startDir) + 1) = '\0';
        } else if(strcmp(argv[1], "-help") == 0){
            printUsage(argv);
            return CRITICAL;
        } else {
            *startDir = calloc(strlen(argv[1]) + 1, sizeof(char));
            if(*startDir == NULL){
                fprintf(stderr, "%s: Error while allocating memory.\n", argv[0]);
                returnValue = CRITICAL;
                goto EXIT_PARSEPARAMS;
            }
            if(strcpy(*startDir, argv[1]) == NULL){
                returnValue = CRITICAL;
                goto EXIT_PARSEPARAMS;
            }
        }

        if(argc == 2){
            if (addListEntry(listHead, PRINT, NULL) == NULL) {
                fprintf(stderr, "Error while adding list entry!\n");
                returnValue = CRITICAL;
            }
            goto EXIT_PARSEPARAMS;
        }

        for(int i = 2; i < argc; i ++){
            if(strcmp(argv[i], "-help") == 0){
                printUsage(argv);
                returnValue = CRITICAL;
                break;
            } else if(strcmp(argv[i], "-user") == 0){
                if(addListEntry(listHead, USER, argv[i + 1]) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else if(strcmp(argv[i], "-name") == 0){
                if (addListEntry(listHead, NAME, argv[i + 1]) == NULL) {
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else if(strcmp(argv[i], "-type") == 0){
                if (addListEntry(listHead, TYPE, argv[i + 1]) == NULL) {
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else if(strcmp(argv[i], "-print") == 0){
                if (addListEntry(listHead, PRINT, NULL) == NULL) {
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
            } else if(strcmp(argv[i], "-ls") == 0){
                if (addListEntry(listHead, LS, NULL) == NULL) {
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
            } else if(strcmp(argv[i], "-nouser") == 0){
                if (addListEntry(listHead, NOUSER, NULL) == NULL) {
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
                ACTION_COUNT++;
            } else if(strcmp(argv[i], "-path") == 0){
                if (addListEntry(listHead, PATH, argv[i + 1]) == NULL) {
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else {
                fprintf(stderr, "%s: %s is not a valid argument.\n", argv[0], argv[i]);
                printUsage(argv);
                return 1;
            }
        }
    }

    EXIT_PARSEPARAMS:
    return returnValue;
}

static ACTION *addListEntry(ACTION *listHead, int type, const char *params){
    if(FLAG_PRINT_ONLY != 0 && type != PRINT && type != LS){
        FLAG_PRINT_ONLY = 0;    // Action(s) different to print was provided as arguments
    }

    ACTION *currentEntry = listHead;

    // Is the current entry head entry and are no further nodes available?
    // Then fill head node
    if(currentEntry != NULL && currentEntry->prev == NULL && currentEntry->next == NULL && currentEntry->param == NULL && currentEntry->type == 0){
        currentEntry->type = type;
        if(params != NULL || type == NOUSER || type == PRINT || type == LS){
            if(type == NOUSER || type == PRINT || type == LS){
                currentEntry->param = NULL;
            } else {
                currentEntry->param = calloc(strlen(params)+1, sizeof(char));
            }

            if(currentEntry->param != NULL || type == NOUSER || type == PRINT || type == LS){
                if(type != NOUSER && type != PRINT && type != LS){
                    if(strcpy(currentEntry->param, params) == NULL){
                        return NULL;
                    }
                }

                switch(type) {
                    case USER:
                        currentEntry->actionFunction = &doActionUser;
                        break;
                    case NAME:
                        currentEntry->actionFunction = &doActionName;
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
                    case PRINT:
                        currentEntry->actionFunction = NULL;
                        break;
                    case LS:
                        currentEntry->actionFunction = NULL;
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
    } else { // More nodes than just head node available; travel down list and add new node at end
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
        if (params != NULL && type != NOUSER) {
            currentEntry->next->param = calloc(strlen(params)+1, sizeof(char));
            if(currentEntry->next->param == NULL){
                return NULL;
            }
            if(strcpy(currentEntry->next->param, params) == NULL){
                return NULL;
            }
        } else if(params == NULL && type == NOUSER){
            currentEntry->next->param = NULL;
        } else if(params == NULL && (type == PRINT || type == LS)){
            currentEntry->next->param = NULL;
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
                currentEntry->next->actionFunction = NULL;
                break;
            case PRINT:
                currentEntry->next->actionFunction = NULL;
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

static void cleanupList(ACTION *listHead){
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

static int printEntry(char *fileName, int flags){
    if(flags == FLAG_PRINTENTRY_PRINT){
        if(printf("%s\n", fileName) < 0){
            fprintf(stderr, "Error printing to stdout()\n");
            return WARNING;
        }
    }

    if(flags == FLAG_PRINTENTRY_LS){
       // complex printout required
        struct stat fileStats;
        errno = 0;
        if(lstat(fileName, &fileStats) == -1){
            error(0, errno, "Error while getting file stats.");
            return WARNING;
        }

        if(printf("%9ld %6ld ", fileStats.st_ino, fileStats.st_blocks/2) < 0){
            error(0, errno, "Error while printing to stdout");
            return CRITICAL;
        }

        int isLink = 0;
        int isDevice = 0;
        char *permissions = calloc(12, sizeof(char));
        if(S_ISDIR(fileStats.st_mode) != 0) {
            *permissions = 'd';
        } else if(S_ISBLK(fileStats.st_mode) != 0){
            isDevice = 1;
            *permissions = 'b';
        } else if(S_ISCHR(fileStats.st_mode) != 0){
            isDevice = 1;
            *permissions = 'c';
        } else if(S_ISLNK(fileStats.st_mode) != 0){
            isLink = 1;
            *permissions = 'l';
        } else if(S_ISFIFO(fileStats.st_mode) != 0){
            *permissions = 'p';
        } else if(S_ISSOCK(fileStats.st_mode) != 0){
            *permissions = 's';
        } else {
            *permissions = '-';
        }

        /* User-permissions */
        if (fileStats.st_mode & S_IRUSR) {
            *(permissions + 1) = 'r';
        } else {
            *(permissions + 1) = '-';
        }
        if (fileStats.st_mode & S_IWUSR) {
            *(permissions + 2) = 'w';
        } else {
            *(permissions + 2) = '-';
        }

        /* Setuid-Bit */
        if(fileStats.st_mode & S_ISUID){
            if (fileStats.st_mode & S_IXUSR) {
                *(permissions + 3) = 's';
            } else {
                *(permissions + 3) = 'S';
            }
        } else {
            if (fileStats.st_mode & S_IXUSR) {
                *(permissions + 3) = 'x';
            } else {
                *(permissions + 3) = '-';
            }
        }

        /* Group permissions */
        if (fileStats.st_mode & S_IRGRP) {
            *(permissions + 4) = 'r';
        } else {
            *(permissions + 4) = '-';
        }
        if (fileStats.st_mode & S_IWGRP) {
            *(permissions + 5) = 'w';
        } else {
            *(permissions + 5) = '-';
        }

        /* Setgid bits */
        if(fileStats.st_mode & S_ISGID){
            if (fileStats.st_mode & S_IXGRP) {
                *(permissions + 6) = 's';
            } else {
                *(permissions + 6) = 'S';
            }
        } else {
            if (fileStats.st_mode & S_IXGRP) {
                *(permissions + 6) = 'x';
            } else {
                *(permissions + 6) = '-';
            }
        }

        /* Others permissions */
        if (fileStats.st_mode & S_IROTH) {
            *(permissions + 7) = 'r';
        } else {
            *(permissions + 7) = '-';
        }
        if (fileStats.st_mode & S_IWOTH) {
            *(permissions + 8) = 'w';
        } else {
            *(permissions + 8) = '-';
        }

        /* Sticky-bit */
        if(fileStats.st_mode & S_ISVTX){
            if (fileStats.st_mode & S_IXOTH) {
                *(permissions + 9) = 't';
            } else {
                *(permissions + 9) = 'T';
            }
        } else {
            if (fileStats.st_mode & S_IXOTH) {
                *(permissions + 9) = 'x';
            } else {
                *(permissions + 9) = '-';
            }
        }

        *(permissions + 10) = '\0';

        if(printf("%s %3ld ", permissions, fileStats.st_nlink) < 0){
            error(0, errno, "Error while printing to stdout");
            return CRITICAL;
        }

        errno = 0;
        const struct passwd *pwdOwner = getpwuid(fileStats.st_uid);
        if (pwdOwner == NULL){
            if(errno == EINTR || errno == EIO || errno == EMFILE || errno == ENFILE || errno == ENOMEM || errno == ERANGE){
                error(0, errno, "Error while checking user info");
                return CRITICAL;
            } else {
                // user not found; print uid
                if(printf("%-8u ", fileStats.st_uid) < 0){
                    error(0, errno, "Error while printing to stdout");
                    return CRITICAL;
                }
            }
        } else {
            // user found; print name
            if(printf("%-8s ", pwdOwner->pw_name) < 0){
                error(0, errno, "Error while printing to stdout");
                return CRITICAL;
            }
        }

        errno = 0;
        const struct group *grpOwner = getgrgid(fileStats.st_gid);
        if (grpOwner == NULL){
            if(errno == EINTR || errno == EIO || errno == EMFILE || errno == ENFILE || errno == ENOMEM || errno == ERANGE){
                error(0, errno, "Error while checking group info");
                return CRITICAL;
            } else {
                // group not found; print gid
                if(printf("%-8u ", fileStats.st_gid) < 0){
                    error(0, errno, "Error while printing to stdout");
                    return CRITICAL;
                }
            }
        } else {
            // group found; print name
            if(printf("%-8s ", grpOwner->gr_name) < 0){
                error(0, errno, "Error while printing to stdout");
                return CRITICAL;
            }
        }

        struct tm *lastModifiedCon = localtime(&fileStats.st_mtim.tv_sec);
        char lastModDateFormatted[13]; /* 13 - fixed date length */

        if(strftime(lastModDateFormatted,13,"%b %e %H:%M", lastModifiedCon) == 0){
            error(0, errno, "Error while formatting date.");
            return CRITICAL;
        }

        if(isDevice == 1){
            if(printf("%3d, %3d ", major(fileStats.st_rdev), minor(fileStats.st_rdev)) < 0){
                error(0, errno, "Error while printing to stdout");
                return CRITICAL;
            }
        } else {
            if(printf("%8ld ", fileStats.st_size) < 0){
                error(0, errno, "Error while printing to stdout");
                return CRITICAL;
            }
        }

        if(printf("%s ", lastModDateFormatted) < 0){
            error(0, errno, "Error while printing to stdout");
            return CRITICAL;
        }

        if(!isLink){
            if(printf("%s\n", fileName) < 0){
                error(0, errno, "Error while printing to stdout");
                return CRITICAL;
            }
        } else {
            char linkbuf[fileStats.st_size + 1];
            const int charsread = readlink(fileName ,linkbuf, fileStats.st_size);

            if(charsread == -1){
                /* error */
            } else {
                linkbuf[fileStats.st_size] = '\0';
                if(printf("%s -> %s\n", fileName, linkbuf) < 0){
                    error(0, errno, "Error while printing to stdout");
                    return CRITICAL;
                }
            }
        }
    }

    return SUCCESS;
}

static int printUsage(char *argv[]){
    if(printf("Myfind - Group 11\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("Usage: %s <file or directory> [action]\n\n", argv[0]) < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("-help:\tCan be added anywhere to show this message\n\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }
    if(printf("File or directory:\tStarting point for searching\n\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("Actions:\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("-user <username or uid>:\tShows files or directories owned by this user\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("-name <pattern>:\tShows files or directories matching supplied pattern\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("-type [bcdpfls]:\tShows files or directories matching supplied type\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("-print:\tPrints results to stdout (default if nothing else is entered)\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("-ls:\tPrints ls(1) like information for results\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("-nouser:\tShows files or directories owned by nonexistent user\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    if(printf("-path <pattern>:\tShows files or directories with matching path\n") < 0){
        error(0, errno, "Error while printing to stdout");
        return CRITICAL;
    }

    exit(0);
}