/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: some common declarations and DEFINEs.
 */

#pragma once

#define PADDLE_DEFAULT_X (i32)((WINWIDTH / 2) - (PADDLE_WIDTH / 2))
#define PADDLE_DEFAULT_Y (i32)(WINHEIGHT - 75)

#define NUM_BRICKS    (i32)(WINWIDTH / (BRICK_WIDTH + 20))
#define BRICK_PADDING (i32)((WINWIDTH - NUM_BRICKS * (BRICK_WIDTH + 20)) / 2)

#define LENGTH(lst) (i32)(sizeof(lst) / sizeof(lst[0]))
