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

#pragma once

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

#define PADDLE_DEFAULT_X (i32)((WINWIDTH / 2) - (PADDLE_WIDTH / 2))
#define PADDLE_DEFAULT_Y (i32)(WINHEIGHT - 75)

#define NUM_BRICKS    (i32)(WINWIDTH / (BRICK_WIDTH + 20))
#define BRICK_PADDING (i32)((WINWIDTH - NUM_BRICKS * (BRICK_WIDTH + 20)) / 2)

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
        eprintf("\033[31;1mpanic:\033[0m line `%d`, func `%s` in file `%s`: "  \
                "`",                                                           \
                __LINE__, __func__, __FILE__);                                 \
        eprintf(__VA_ARGS__);                                                  \
        eprintf(S_DIM                                                          \
                "`\n       please report this to the developer.\n" S_END);     \
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
