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
#include <raylib.h>

Bricks s_game_bricks_init(void);
// populates and initializes if necessary.
void s_game_bricks_populate(Bricks* b);
void s_game_bricks_deinit(Bricks* b);

void s_game_draw_bricks(GameState* s);
void s_game_draw_hud_left(GameState* s);
void s_game_draw_hud_right(GameState* s);
void s_game_gui_draw(GameState* s);
void s_game_draw(GameState* s);

void s_game_update_paddle(GameState* s);
void s_game_update_ball(GameState* s);
void s_game_update_bricks(GameState* s);
void s_game_update(GameState* s);

GameGui s_game_gui_new(void);
Screen s_game_new(void);

void s_game_deinit(GameState* s);

DeadGui s_dead_gui_new(void);
Screen s_dead_new(void);
void s_dead_gui_draw(DeadScreenState* s);
void s_dead_draw(DeadScreenState* s);
void s_dead_update(DeadScreenState* s);
void s_dead_deinit(DeadScreenState* s);

WinGui s_win_gui_new(void);
Screen s_win_new(void);
void s_win_gui_draw(WinScreenState* s);
void s_win_draw(WinScreenState* s);
void s_win_update(WinScreenState* s);
void s_win_deinit(WinScreenState* s);

#endif
