//
// Created by sebastian on 3/1/20.
//


#include "action_ls.h"

/*
 * TODO: Refactor; Not working as of now!
 * */
int doActionLs(char *fileName, char *params){
    // Prevent errors about unused params; Delete once function is implemented!
    fileName = fileName;
    params = params;
    // ------------------------------------------------------------------------
    struct stat *fileStats = malloc(sizeof(struct stat));
    stat(fileName, fileStats);

    char *permissions = calloc(11, sizeof(char));
    if (S_ISDIR(fileStats->st_mode)) {
        *permissions = 'd';
    } else {
        *permissions = '-';
    }

    if (fileStats->st_mode & S_IRUSR) {
        *(permissions + 1) = 'r';
    } else {
        *(permissions + 1) = '-';
    }
    if (fileStats->st_mode & S_IWUSR) {
        *(permissions + 2) = 'w';
    } else {
        *(permissions + 2) = '-';
    }
    if (fileStats->st_mode & S_IXUSR) {
        *(permissions + 3) = 'x';
    } else {
        *(permissions + 3) = '-';
    }


    if (fileStats->st_mode & S_IRGRP) {
        *(permissions + 4) = 'r';
    } else {
        *(permissions + 4) = '-';
    }
    if (fileStats->st_mode & S_IWGRP) {
        *(permissions + 5) = 'w';
    } else {
        *(permissions + 5) = '-';
    }
    if (fileStats->st_mode & S_IXGRP) {
        *(permissions + 6) = 'x';
    } else {
        *(permissions + 6) = '-';
    }


    if (fileStats->st_mode & S_IROTH) {
        *(permissions + 7) = 'r';
    } else {
        *(permissions + 7) = '-';
    }
    if (fileStats->st_mode & S_IWOTH) {
        *(permissions + 8) = 'w';
    } else {
        *(permissions + 8) = '-';
    }
    if (fileStats->st_mode & S_IXOTH) {
        *(permissions + 9) = 'x';
    } else {
        *(permissions + 9) = '-';
    }


    *(permissions + 10) = '\0';

    printf("%ld\t%ld\t%s\t%ld\t%d\t%d\t%s\n", fileStats->st_ino, fileStats->st_blocks, permissions,
           fileStats->st_nlink, fileStats->st_uid, fileStats->st_gid, fileName);

    return 0;
}