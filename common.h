/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: common declarations
 */

#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>

typedef size_t usize;
typedef ssize_t isize;
typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short int u16;
typedef signed short int i16;
typedef unsigned int u32;
typedef signed int i32;
typedef unsigned long long int u64;
typedef signed long long int i64;
typedef float f32;
typedef double f64;

#define LENGTH(lst) (i32)(sizeof(lst) / sizeof(lst[0]))

#define S_BOLD "\033[1m"
#define S_DIM  "\033[2m"
#define S_END  "\033[0m"

#define S_BLACK   "\033[30m"
#define S_RED     "\033[31m"
#define S_GREEN   "\033[32m"
#define S_YELLOW  "\033[33m"
#define S_BLUE    "\033[34m"
#define S_MAGENTA "\033[35m"
#define S_CYAN    "\033[36m"
#define S_WHITE   "\033[37m"

#define S_BGBLACK   "\033[40m"
#define S_BGRED     "\033[41m"
#define S_BGGREEN   "\033[42m"
#define S_BGYELLOW  "\033[43m"
#define S_BGBLUE    "\033[44m"
#define S_BGMAGENTA "\033[45m"
#define S_BGCYAN    "\033[46m"
#define S_BGWHITE   "\033[47m"

#define S_CLEAR_SCREEN "\033[2J\033[H"
#define S_CLEAR_LINE   "\r\033[K"

#define S_ENTER_ALT "\033[?1049h"
#define S_LEAVE_ALT "\033[?1049l"

#define S_SHOWCURSOR "\033[?25h"
#define S_HIDECURSOR "\033[?25l"

#define check_alloc(ptr)                                                       \
                                                                               \
    if (ptr == NULL) {                                                         \
        panic("allocation of `%s` failed", #ptr);                              \
        perror("perror");                                                      \
        exit(1);                                                               \
    }

#define eprintf(...) fprintf(stderr, __VA_ARGS__);

#define panic(...)                                                             \
    {                                                                          \
        eprintf("\033[31;1mpanic:\033[0m line %d, func \"%s\" in file "        \
                "\"%s\": ",                                                    \
                __LINE__, __func__, __FILE__);                                 \
        eprintf(__VA_ARGS__);                                                  \
        eprintf(S_DIM                                                          \
                "\n       please report this to the developer.\n" S_END);      \
        exit(1);                                                               \
    }

#define fatal_noexit(...)                                                      \
    {                                                                          \
        eprintf(S_RED S_BOLD "[fatal] " S_END);                                \
        eprintf(S_DIM);                                                        \
        eprintf(__VA_ARGS__);                                                  \
        eprintf(S_END "\n");                                                   \
    }

#define fatal(...)                                                             \
    {                                                                          \
        fatal_noexit(__VA_ARGS__);                                             \
        exit(1);                                                               \
    }

#define warn(...)                                                              \
    {                                                                          \
        eprintf(S_MAGENTA S_BOLD "[warn] " S_END);                             \
        eprintf(S_DIM);                                                        \
        eprintf(__VA_ARGS__);                                                  \
        eprintf(S_END "\n");                                                   \
    }

#define info(...)                                                              \
    {                                                                          \
        eprintf(S_CYAN S_BOLD "[info] " S_END);                                \
        eprintf(S_DIM);                                                        \
        eprintf(__VA_ARGS__);                                                  \
        eprintf(S_END "\n");                                                   \
    }

#define VERSION "0.2.0-pre"

#define HELP                                                                   \
    "\033[1mbeanbricks: a questionable breakout clone in C and "               \
    "raylib.\033[0m\n\n"                                                       \
    "Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.\n"                 \
    "This program is licensed under the MIT/Expat license. View the full "     \
    "text of the\nlicense in the root of the project, or pass --license.\n\n"  \
    "usage: beanbricks [flags]\n"                                              \
    "running the program with no args will launch the game.\n\n"               \
    "options:\n"                                                               \
    "    --help: show this help screen\n"                                      \
    "    --version: show the version of the program\n"                         \
    "    --license: show the license of the program\n"

#define LICENSE                                                                \
    "Copyright (c) 2024-2025 Eason Qin (eason@ezntek.com)\n"                   \
    "\n"                                                                       \
    "Permission is hereby granted, free of charge, to any person\n"            \
    "obtaining a copy of this software and associated documentation\n"         \
    "files (the “Software”), to deal in the Software without\n"                \
    "restriction, including without limitation the rights to use,\n"           \
    "copy, modify, merge, publish, distribute, sublicense, and/or sell\n"      \
    "copies of the Software, and to permit persons to whom the\n"              \
    "Software is furnished to do so, subject to the following\n"               \
    "conditions:\n"                                                            \
    "\n"                                                                       \
    "The above copyright notice and this permission notice shall be\n"         \
    "included in all copies or substantial portions of the Software.\n"        \
    "\n"                                                                       \
    "THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,\n"        \
    "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES\n"        \
    "OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\n"               \
    "NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\n"            \
    "HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,\n"           \
    "WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"           \
    "FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR\n"          \
    "OTHER DEALINGS IN THE SOFTWARE.\n"

#endif
