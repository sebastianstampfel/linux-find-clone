/**
* @file action_nouser.c
* Betriebssysteme MyFind Action-Nouser-File
* Beispiel 1
*
* @author Sebastian Stampfel <ic19b084@technikum-wien.at>
* @author Milan Kollmann <ic19b058@technikum-wien.at>
* @author Benjamin Wiesbauer <ic19b096@technikum-wien.at>
* @date 2020/02/22
*
* @version 1
*/

#include "action_nouser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <pwd.h>

/*
 * TODO: Implement!
 */
int doActionNoUser(char *fileName, char *params){
    // Prevent errors about unused params; Delete once function is implemented!
    //fileName = fileName;
    params = params;
    struct stat buf = {0};

    errno = 0;
    if (lstat(fileName, &buf) == -1) {
        error(0, errno, "File does not exist!");
        return -1;
    }
    if (getpwuid(buf.st_uid) == NULL) {
        return 0;
    }
    return 1;
}
