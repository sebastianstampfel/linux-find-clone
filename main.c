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
#include <grp.h>
#include <time.h>

// --------------------------------------------------------------- defines --
#define SUCCESS 0
#define WARNING 1
#define CRITICAL -1
// -------------------------------------------------------------- typedefs --
// --------------------------------------------------------------- globals --

/**
 * @brief GLOBAL - Indicates if -print was supplied as a command line argument.
 * 0: -print not supplied, 1: -print was supplied
 */
int FLAG_PRINT = 0;

/**
 * @brief GLOBAL - Indicates if -ls was supplied as a command line argument.
 * 0: -ls not supplied; 1: -ls was supplied
 * 
 */
int FLAG_LS = 0;

/**
 * @brief GLOBAL - Indicates if -print was the only action supplied as a command line argument.
 * 0: Actions other than -print were provided, 1: -print is only action supplied.
 * Defaults to 1, in case no argument other than a start-path was provided.
 */
int FLAG_PRINT_ONLY = 1;

/**
 * @brief GLOBAL - Tracks if "." and ".." have been printed to stdout
 * 0: not printed yet, 1: printed
 */
int FLAG_STD_DIRS_PRINTED = 0;

/**
 * @brief GLOBAL - Total count of actions supplied as command line arguments.
 */
int ACTION_COUNT = 0;

// ------------------------------------------------------------- functions --
/**
 * @brief Function to handle a directory on the filesystem.
 *
 * Travels down the dir path until the end. Recursively calls
 * itself if more dirs are foun.
 * 
 * @param dir_name Name of the directory
 * @param listHead Head of doubly linked list of action struct
 * @see action
 * @return int 0 on success, 1 on failure 
 */
static int doDir(char *dir_name, ACTION *listHead);

/**
 * @brief Function to handle a file on the filesystem.
 * 
 * @param fileName Path to the file
 * @param listHead Head of doubly linked list of action struct
 * @see action
 * @return int 0 on success, 1 on failure 
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
static int parseParams(int argc, const char *argv[], ACTION *listHead, char **startDir);

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
static int printEntry(char *fileName);

int main(int argc, const char *argv[])
{
    int returnValue = SUCCESS;

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

    struct stat buf;
    errno = 0;
    if(lstat(startdir, &buf) == -1){
        error(0, errno, "Error reading starting point");
    }

    // If user supplies a valid directory as starting point,
    // travel down the dir path.
    // Else, treat as a single file.
    if(S_ISDIR(buf.st_mode) != 0){
        if(doDir(startdir, listHead) == CRITICAL){
            returnValue = CRITICAL;
            cleanupList(listHead);
            free(startdir);
            return returnValue;
        }
    } else {
        doFile(startdir, listHead);
    }


    cleanupList(listHead);
    free(startdir);

    exit(0);
}

static int doDir(char *dirName, ACTION *listHead){
    int returnValue = SUCCESS;
    DIR *dirStream = opendir(dirName);
    if(dirStream == NULL){
        // Rest of error message is coming from errno
        error(0, errno, "Directory %s", dirName);
        returnValue = WARNING;
        goto CLEANEXIT_DODIR;
    }

    struct dirent *dirContent = readdir(dirStream);
    while(dirContent != NULL){
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
            if(strcmp(".", dirContent->d_name) == 0 || strcmp("..", dirContent->d_name) == 0){
                if(FLAG_STD_DIRS_PRINTED == 0 && FLAG_PRINT_ONLY){
                    if(printEntry(dirContent->d_name) != 0){
                        error(0, errno, "Error while printing to stdout.");
                        returnValue = WARNING;
                        goto CLEANEXIT_DODIR;
                    }
                    FLAG_STD_DIRS_PRINTED = 1;
                }
            } else {
                if((FLAG_PRINT == 1 && FLAG_PRINT_ONLY == 1) || (FLAG_LS == 1 && FLAG_PRINT_ONLY == 1)){
                    if(printEntry(fullPath) != 0){
                        error(0, errno, "Error while printing to stdout.");
                        returnValue = WARNING;
                        goto CLEANEXIT_DODIR;
                    }

                } else {
                    // do actions
                    ACTION *current = listHead;
                    int matchedActions = 0;

                    while(1){
                        int retVal = (*current->actionFunction)(fullPath, current->param);
                        if(retVal < 0){
                            error(0, errno, "Something bad happened, idk what.");
                            returnValue = CRITICAL;
                            goto CLEANEXIT_DODIR;
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
                        printEntry(fullPath);
                    }
                }

                const int retVal = doDir(fullPath, listHead);
                if(retVal == CRITICAL){
                    returnValue = CRITICAL;
                    goto CLEANEXIT_DODIR;
                } else if(retVal == WARNING){
                    goto CONTINUE_DODIR;
                }
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

static int doFile(char  *fileName, ACTION *listHead){
    int returnValue = 0;

    if((FLAG_PRINT == 1 && FLAG_PRINT_ONLY == 1) || (FLAG_LS == 1 && FLAG_PRINT_ONLY == 1)){
        const int retVal = printEntry(fileName);
        if(retVal != SUCCESS){
            returnValue = retVal;
        }
    } else {
        // Iterate through action list and call function pointer
        ACTION *current = listHead;
        int matchedActions = 0;

        while(1){
            int retVal = (*current->actionFunction)(fileName, current->param);
            if(retVal < 0){
                //error(0, errno, "Something bad happened, idk what.");
                returnValue = CRITICAL;
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
            const int retVal = printEntry(fileName);
            if(retVal != SUCCESS){
                returnValue = retVal;
            }
        }
    }

    return returnValue;
}

static int parseParams(int argc, const char *argv[], ACTION *listHead, char **startDir){
    int returnValue = SUCCESS;

    if(argc <= 1){
        fprintf(stderr, "%s: Not enough arguments provided.\n", argv[0]);
        returnValue = CRITICAL;
    } else {
        if(strcmp(argv[1], "./") == 0){
            *startDir = calloc(2, sizeof(char)); // warum calloc()? => array
            if(*startDir == NULL){
                fprintf(stderr, "%s: Error while allocating memory.\n", argv[0]);
                returnValue = CRITICAL;
                goto EXIT_PARSEPARAMS;
            }
            **startDir = '.';
            *(*(startDir) + 1) = '\0';
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
            FLAG_PRINT = 1;
            goto EXIT_PARSEPARAMS;
        }

        for(int i = 2; i < argc; i ++){
            if(strcmp(argv[i], "-user") == 0){
                if(addListEntry(listHead, USER, argv[i + 1]) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else if(strcmp(argv[i], "-name") == 0){
                if(addListEntry(listHead, NAME, argv[i + 1]) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else if(strcmp(argv[i], "-type") == 0){
                if(addListEntry(listHead, TYPE, argv[i + 1]) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    returnValue = CRITICAL;
                    break;
                }
                ACTION_COUNT++;
                i++;
            } else if(strcmp(argv[i], "-print") == 0){
                FLAG_PRINT = 1;
            } else if(strcmp(argv[i], "-ls") == 0){
                FLAG_LS = 1;
            } else if(strcmp(argv[i], "-nouser") == 0){
                if(addListEntry(listHead, NOUSER, NULL) == NULL){
                    fprintf(stderr, "Error while adding list entry!\n");
                    break;
                }
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

    EXIT_PARSEPARAMS:
    return returnValue;
}

static ACTION *addListEntry(ACTION *listHead, int type, const char *params){
    if(FLAG_PRINT_ONLY != 0){
        FLAG_PRINT_ONLY = 0;    // Action(s) different to print was provided as arguments
    }
    ACTION *currentEntry = listHead;

    if(currentEntry != NULL && currentEntry->prev == NULL && currentEntry->next == NULL && currentEntry->param == NULL){
        currentEntry->type = type;
        if(params != NULL || type == NOUSER){
            if(type == NOUSER){
                currentEntry->param = NULL;
            } else {
                currentEntry->param = calloc(strlen(params)+1, sizeof(char));
            }

            if(currentEntry->param != NULL || type == NOUSER){
                if(type != NOUSER){
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

static int printEntry(char *fileName){
    if(FLAG_LS == 1){
       // complex printout required
        struct stat fileStats;
        errno = 0;
        if(lstat(fileName, &fileStats) == -1){
            error(0, errno, "Error while getting file stats.");
            return WARNING;
        }

        if(printf("%ld\t%ld\t", fileStats.st_ino, fileStats.st_blocks) < 0){
            error(0, errno, "Error while printing to stdout");
            return CRITICAL;
        }

        int isLink = 0;
        char *permissions = calloc(12, sizeof(char));
        if(S_ISDIR(fileStats.st_mode) != 0) {
            *permissions = 'd';
        } else if(S_ISBLK(fileStats.st_mode) != 0){
            *permissions = 'b';
        } else if(S_ISCHR(fileStats.st_mode) != 0){
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
        if (fileStats.st_mode & S_IXUSR) {
            *(permissions + 3) = 'x';
        } else {
            *(permissions + 3) = '-';
        }


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
        if (fileStats.st_mode & S_IXGRP) {
            *(permissions + 6) = 'x';
        } else {
            *(permissions + 6) = '-';
        }


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

        // TODO: Stickybit
        if (fileStats.st_mode & S_IXOTH) {
            *(permissions + 9) = 'x';
        } else {
            *(permissions + 9) = '-';
        }


        *(permissions + 10) = '\0';

        printf("%s\t%ld\t", permissions, fileStats.st_nlink);

        errno = 0;
        const struct passwd *pwdOwner = getpwuid(fileStats.st_uid);
        if (pwdOwner == NULL){
            if(errno == EINTR || errno == EIO || errno == EMFILE || errno == ENFILE || errno == ENOMEM || errno == ERANGE){
                error(0, errno, "Error while checking user info");
                return CRITICAL;
            } else {
                // user not found; print uid
                printf("%u\t\t", fileStats.st_uid);
            }
        } else {
            // user found; print name
            printf("%s\t", pwdOwner->pw_name);
        }

        errno = 0;
        const struct group *grpOwner = getgrgid(fileStats.st_gid);
        if (grpOwner == NULL){
            if(errno == EINTR || errno == EIO || errno == EMFILE || errno == ENFILE || errno == ENOMEM || errno == ERANGE){
                error(0, errno, "Error while checking group info");
                return CRITICAL;
            } else {
                // group not found; print gid
                printf("%u\t\t", fileStats.st_gid);
            }
        } else {
            // group found; print name
            printf("%s\t", grpOwner->gr_name);
        }

        struct tm *lastModifiedCon = localtime(&fileStats.st_mtim.tv_sec);

        // st_mtim

        char lastModDateFormatted[13];

        strftime(lastModDateFormatted,13,"%b %d %H:%M", lastModifiedCon);

        printf("%s\t", lastModDateFormatted);

        if(!isLink){
            printf("%s\n", fileName);
        } else {
            char linkbuf[fileStats.st_size + 1];
            const int charsread = readlink(fileName ,linkbuf, fileStats.st_size);

            if(charsread == -1){
                /* error */
            } else {
                linkbuf[fileStats.st_size] = '\0';
                printf("%s -> %s\n", fileName, linkbuf);
            }
        }

        // Nummer des Inodes
        // Anzahl der Blocks
        // Permissions
        // Anzahl der Links
        // Owner (Name!)
        // Group (Name!)
        // Last Modification Time
        // Namen

        //printf("%ld\t%ld\t%s\t%ld\t%d\t%d\t%s\n", fileStats.st_ino, fileStats.st_blocks, permissions,
               //fileStats.st_nlink, fileStats.st_uid, fileStats.st_gid, fileName);

       return SUCCESS;
    } else {
        if(printf("%s\n", fileName) < 0){
            fprintf(stderr, "Error printing to stdout()\n");
            return WARNING;
        } else {
            return SUCCESS;
        }
    }
}
