cmake_minimum_required(VERSION 3.15)
project(myfind_debug C)

set(CMAKE_C_STANDARD 11)

add_executable(myfind_debug main.c action_ls.c action_ls.h action.c action.h action_user.c action_user.h action_name.c action_name.h action_nouser.c action_nouser.h action_type.c action_type.h action_path.c action_path.h)