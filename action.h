/**
 * @file action.h
 * @brief Header file containing the struct definition for action struct and 
 *          macros for action types.
 * @version 0.1
 * @date 2020-03-10
 * 
 * 
 */
#ifndef MYFIND_DEBUG_ACTION_H
#define MYFIND_DEBUG_ACTION_H

#define USER 10
#define NAME 11
#define TYPE 12
#define PRINT 13
#define LS 14
#define NOUSER 15
#define PATH 16


/**
 * @struct action
 * @brief Struct for storing an action
 * 
 * Stores a certain action to be performes. Action type is stored as an int according to
 * defined macros. Additionally, stores params for action that might be necessary or NULL
 * if action does not require further params. Used as a doubly linked list.
 * 
 */
typedef struct action{
    int type; ///< Type of action
    char *param; ///< Further params for this action
    int (*actionFunction)(char *, char *); ///< Functionpointer to matching function of action
    struct action *next; ///< The next action in doubly linked list
    struct action *prev; ///< The previous action in doubly linked list
} ACTION;

#endif //MYFIND_DEBUG_ACTION_H
