//*
// @file action_name.h
// Betriebssysteme MyFind-Main-File
// Beispiel 1
//
// @author Sebastian Stampfel <ic19b084@technikum-wien.at>
// @author Milan Kollmann <ic19b058@technikum-wien.at>
// @author Benjamin Wiesbauer <ic19b096@technikum-wien.at>
// @date 2020/02/22
//
//

#ifndef MYFIND_DEBUG_ACTION_NAME_H
#define MYFIND_DEBUG_ACTION_NAME_H

// ------------------------------------------------------------- functions --
/**
 * @brief Function to check if the passed file (filePath) contains the passed string (params)
 * 
 * @param filePath path to file
 * @param params string to be searched for 
 * @return int 0 on match, 1 on no match, -1 on error
 */
int doActionName(char *filePath, char *params);
/**
 * @brief Function to split up the path to file to get the filename 
 * 
 * @param filePath path to file
 * @return char array containing filename on success, NULL on failure
 */
static char *fileNameFromPath(char *filePath);

#endif //MYFIND_DEBUG_ACTION_NAME_H
