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

// Populates the bricks.
void s_game_bricks_init(void);
void s_game_bricks_make(void);
void s_game_bricks_deinit(void);
void s_game_draw_bricks(void);
void s_game_draw_hud_left(void);
void s_game_draw_hud_right(void);
void s_game_gui_draw(void);
void s_game_draw(void);

void s_game_update_paddle(void);
void s_game_update_ball(void);
void s_game_update_bricks(void);
void s_game_update(void);

void s_game_reset(void);

DeadGui s_dead_gui_new(void);
Screen s_dead_new(void);
void s_dead_gui_draw(DeadScreenState* dss);
void s_dead_draw(DeadScreenState* dss);
void s_dead_update(DeadScreenState* dss);
void s_dead_destroy(DeadScreenState* dss);

WinGui s_win_gui_new(void);
Screen s_win_new(void);
void s_win_gui_draw(WinScreenState* s);
void s_win_draw(WinScreenState* s);
void s_win_update(WinScreenState* s);
void s_win_destroy(WinScreenState* s);

#endif
