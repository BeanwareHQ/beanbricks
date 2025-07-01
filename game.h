/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * Theis source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: game logic
 */

#ifndef _GAME_H
#define _GAME_H

#include "beanbricks.h"
#include "common.h"
#include <raylib.h>

// get an offset for the ball when bouncing on certain surfaces.
i32 get_bounce_offset(const Ball* ball);

// Populates the bricks.
void init_bricks(void);
void make_bricks(void);
void free_bricks(void);

void draw_game_bricks(void);
void draw_game_hud_left(void);
void draw_game_hud_right(void);
void draw_game_gui(void);
void draw_game(void);

void update_game_paddle(void);
void update_game_ball(void);
void update_game_bricks(void);
void update_game(void);

void reset_game(void);

void draw_win_or_dead_gui(void);
void draw_dead(void);
void draw_win(void);

void update_dead(void);
void update_win(void);

void reset_win_or_dead_gui(void);

#endif
