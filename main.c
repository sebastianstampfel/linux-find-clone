//*
// @file hello.c
// Betriebssysteme Hello World File.
// Beispiel 0
//
// @author Bernd Petrovitsch <bernd.petrovitsch@technikum-wien.at>
// @date 2005/02/22
//
// @version 470 
//
// @todo Test it more seriously and more complete.
// @todo Review it for missing error checks.
// @todo Review it and check the source against the rules at
//       https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/c-rules.html
//

// -------------------------------------------------------------- includes --

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

#define USER 10
#define NAME 11
#define TYPE 12
#define PRINT 13
#define LS 14
#define NOUSER 15
#define PATH 16

// -------------------------------------------------------------- typedefs --
/// \struct action
/// \brief Struct for storing a certain action.
/// 
/// Stores a certain action to be performed. Action type is stored as an int according to
/// defined constants. Additionally, stores params for action that might be necessary.
/// For correct order of actions, use as array.
typedef struct action{
	int type;
	char *param;
	struct action *next;
} ACTION;

// --------------------------------------------------------------- globals --

// ------------------------------------------------------------- functions --
int do_dir(const char *dir_name);
int do_file(const char * file_name);
int parse_params(int argc, const char *argv[], ACTION *listHead, char *startDir);

//*
// \brief The most minimalistic C program
//
// This is the main entry point for any C program.
//
// \param argc the number of arguments
// \param argv the arguments itselves (including the program name in argv[0])
//
// \return always "success"
// \retval 0 always
//
int main(int argc, const char *argv[])
{
	// prevent warnings regarding unused params
	argc = argc;
	argv = argv;

	if (printf("Hello world!\n") < 0) {
		error(1, errno, "printf() failed");
	}
	
	for(int i = 0; i < argc; i++){
		printf("%s\n", argv[i]);
	}


	//do_dir(".");

	ACTION *listHead = NULL;
	char startdir[3] = "";
	if(parse_params(argc, argv, listHead, startdir) != 0){
		return EXIT_FAILURE;
	}

	ACTION *current = listHead;
	int i = 0;
	while(current->type != -1){
		printf("Element %d Type: %d\n", i, current->type);
		printf("Element %d Param: %s\n", i, current->param);

		i++;
		current = listHead+i;
	}
	/* DIR *root = opendir(".");
	struct dirent *rootDir = readdir(root);

	while(rootDir != NULL){
		printf("%s, %d\n", rootDir->d_name, rootDir->d_type);
		if(rootDir->d_type == DT_DIR){
			printf("DIRECTORY\n");
		}
		rootDir = readdir(root);
	}

	closedir(root); */

	return 0;
}

int do_dir(const char *dir_name){
	DIR *dir_stream = opendir(dir_name);
	struct dirent *dir_content = readdir(dir_stream);
	while(dir_content != NULL){
		if(dir_content->d_type == DT_DIR){
			// is dir
			if(strcmp(".", dir_content->d_name) != 0 && strcmp("..", dir_content->d_name)){
				printf("%s/%s\t\t\tDIRECTORY!\n", dir_name, dir_content->d_name);
				
				int newPathLength = strlen(dir_name) + strlen(dir_content->d_name) + 2; // 2 = "/" + '\0'
				char *newPath = calloc(newPathLength, sizeof(char));
				strcat(newPath, dir_name);
				strcat(newPath, "/");
				strcat(newPath, dir_content->d_name);
				*(newPath + (newPathLength - 1)) = '\0';
				do_dir(newPath);
				free(newPath);
			}
		} else if(dir_content->d_type == DT_REG){
			//printf("%s, REG\n", dir_content->d_name);
			int newPathLength = (strlen(dir_name) + strlen(dir_content->d_name))+2;
		    char *filePath = calloc(newPathLength, sizeof(char));
			strcat(filePath, dir_name);
			strcat(filePath, "/");
			strcat(filePath, dir_content->d_name);
			*(filePath + newPathLength -1) = '\0';
			do_file(filePath);
			free(filePath);
		} else if(dir_content->d_type == DT_UNKNOWN){
			printf("%s, UNKOWN\n", dir_content->d_name);
		} else {
			printf("%s, ERROR\n", dir_content->d_name);
		}

		dir_content = readdir(dir_stream);
	}
	closedir(dir_stream);

	return 0;
}

int do_file(const char * file_name){
	struct stat *fileStats = malloc(sizeof(struct stat));
	stat(file_name, fileStats);

	char *permissions = calloc(11, sizeof(char));
	if(S_ISDIR(fileStats->st_mode)){
		*permissions = 'd';
	} else {
		*permissions = '-';
	}

	if(fileStats->st_mode & S_IRUSR){
		*(permissions + 1) = 'r';
	} else {
		*(permissions + 1) = '-';
	}
	if(fileStats->st_mode & S_IWUSR){
		*(permissions + 2) = 'w';
	} else {
		*(permissions + 2) = '-';
	}
	if(fileStats->st_mode & S_IXUSR){
		*(permissions + 3) = 'x';
	} else {
		*(permissions + 3) = '-';
	}



	if(fileStats->st_mode & S_IRGRP){
		*(permissions + 4) = 'r';
	} else {
		*(permissions + 4) = '-';
	}
	if(fileStats->st_mode & S_IWGRP){
		*(permissions + 5) = 'w';
	} else {
		*(permissions + 5) = '-';
	}
	if(fileStats->st_mode & S_IXGRP){
		*(permissions + 6) = 'x';
	} else {
		*(permissions + 6) = '-';
	}



	if(fileStats->st_mode & S_IROTH){
		*(permissions + 7) = 'r';
	} else {
		*(permissions + 7) = '-';
	}
	if(fileStats->st_mode & S_IWOTH){
		*(permissions + 8) = 'w';
	} else {
		*(permissions + 8) = '-';
	}
	if(fileStats->st_mode & S_IXOTH){
		*(permissions + 9) = 'x';
	} else {
		*(permissions + 9) = '-';
	}
	

	*(permissions + 10) = '\0';

	printf("%ld\t%ld\t%s\t%ld\t%d\t%d\t%s\n", fileStats->st_ino, fileStats->st_blocks, permissions, fileStats->st_nlink, fileStats->st_uid, fileStats->st_gid, file_name);

	return 0;
}

int parse_params(int argc, const char *argv[], ACTION *listHead, char *startDir){
	listHead = calloc(1, sizeof(ACTION));
	ACTION *currentAction = listHead;

	if(startDir == NULL){
		// do smthg
	}

	if(argc <= 1){
		fprintf(stderr, "%s: Not enough arguments provided.\n", argv[0]);
		return 1;
	} else {
		if(strcmp(argv[1], "./") == 0){
			startDir = calloc(2, sizeof(char));
			*startDir = '.';
			*(startDir + 1) = '\0';
		} else {
			startDir = calloc(strlen(argv[1]), sizeof(char));
			strcpy(startDir, argv[1]);
		}

		for(int i = 2; i < argc; i ++){
			currentAction = listHead + (i - 2);

			if(strcmp(argv[i], "-user") == 0){
				currentAction->type = USER;
				currentAction->param = calloc(strlen(argv[i + 1]), sizeof(char));
				strcpy(currentAction->param, argv[i + 1]);
				i++;
			} else if(strcmp(argv[i], "-name") == 0){
				currentAction->type = NAME;
				currentAction->param = calloc(strlen(argv[i + 1]), sizeof(char));
				strcpy(currentAction->param, argv[i + 1]);
				i++;
			} else if(strcmp(argv[i], "-type") == 0){
				currentAction->type = NAME;
				currentAction->param = calloc(strlen(argv[i + 1]), sizeof(char));
				strcpy(currentAction->param, argv[i + 1]);
				i++;
			}
		}
	}
	
	

	for(int i = 0; i < 3; i++){
		ACTION *current = listHead + i;
		if(i == 2){
			current->type = -1;
		} else {
			current->type = USER + i;
			current->param = calloc(10, sizeof(char));
			*(current->param) = 'H';
			*(current->param+1) = 'i';
			*(current->param+2) = '!';
			*(current->param+3) = '\0';
		}
	}

	return 0;
}

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End:
