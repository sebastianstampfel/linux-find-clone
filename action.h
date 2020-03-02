//
// Created by sebastian on 3/1/20.
//

#ifndef MYFIND_DEBUG_ACTION_H
#define MYFIND_DEBUG_ACTION_H

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
    int (*actionFunction)(char *, char *);
    struct action *next;
    struct action *prev;
} ACTION;

#endif //MYFIND_DEBUG_ACTION_H
