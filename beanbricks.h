/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: common game-related types and definitions
 */

#ifndef _BEANBRICKS_H
#define _BEANBRICKS_H

#include <raylib.h>

#include "common.h"
#include "leaderboard.h"
#include "theme.h"

#define WIN_WIDTH            (cfg.win_width)
#define WIN_HEIGHT           (cfg.win_height)
#define PADDLE_WIDTH         (cfg.paddle_width)
#define PADDLE_HEIGHT        (cfg.paddle_height)
#define INITIAL_PADDLE_SPEED (cfg.initial_paddle_speed)
#define LAYERS               (cfg.layers)
#define BRICK_WIDTH          (cfg.brick_width)
#define BRICK_HEIGHT         (cfg.brick_height)
#define BALL_RADIUS          (cfg.ball_radius)

#define BG_COLOR            (color(theme.bg_color))
#define DARK_SURFACE_COLOR  (color(theme.dark_surface_color))
#define LIGHT_SURFACE_COLOR (color(theme.light_surface_color))
#define BALL_COLOR          (color(theme.ball_color))
#define TXT_PRIMARY         (color(theme.txt_primary))
#define TXT_SECONDARY       (color(theme.txt_secondary))
#define BRICK_COLORS        (theme.brick_colors)

// TODO: refactor
#define LEADERBOARD_ENTRY_HEIGHT 30  // height of one leaderboard item
#define LEADERBOARD_ENTRY_WIDTH  400 // width of one leaderboard item
#define PADDLE_DEFAULT_X         (i32)((cfg.win_width / 2) - (cfg.paddle_width / 2))
#define PADDLE_DEFAULT_Y         (i32)(cfg.win_height - 75)
#define NUM_BRICKS               (i32)(WIN_WIDTH / (BRICK_WIDTH + 20))
#define BRICK_PADDING            (i32)((WIN_WIDTH - NUM_BRICKS * (BRICK_WIDTH + 20)) / 2)
#define SPEED                    1

typedef struct {
    f64 x;
    f64 y;
    f64 xspd;
    f64 yspd;
    Color color;
} Ball;

typedef struct {
    Rectangle rec;
    Color color;
    i32 speed_offset;
} Paddle;

typedef struct {
    Rectangle rec;
    i32 value;
    bool active;
} Brick;

typedef enum {
    SCR_GAME = 0,
    SCR_DEAD = 1,
    SCR_WIN = 2,
    SCR_TITLE = 3,
    SCR_SETTINGS = 4,
} Screen;

typedef struct {
    Rectangle quit_button;
    Rectangle exit_overlay_yes_button;
    Rectangle exit_overlay_no_button;
    bool draw;
} GameGui;

typedef struct {
    Rectangle title_button;
    Rectangle restart_button;
    Rectangle quit_button;
} WinDeadGui; // GUI elements displayed on both the death and win screens

typedef struct {
    Rectangle start_button;
    Rectangle quit_button;
    Rectangle settings_button;
} TitleScreenGui;

typedef struct {
    Brick* data;
    bool* status;
} Bricks;

typedef struct {
    Paddle paddle;
    Ball ball;
    GameGui gui;
    u32 score;
    u32 bricks_broken; // HUD
    i32 paddle_speed;
    bool paused;
    bool exit_overlay;
    Bricks bricks;
} GameState;

typedef struct {
    TitleScreenGui gui;
    i32 title_anim_stage;
    bool title_anim_growing;
} TitleScreenState;

typedef struct {
    GameState game;
    TitleScreenState title_screen;
    WinDeadGui win_dead_gui;
    Screen screen;
    bool should_close;
} State;

Color color(i32);

extern u32 maxscore;

// The global config singleton (sorry no singleton pattern. this is C.)
extern Config cfg;

// The global theme spec
extern ThemeSpec theme;

// The global game state (sorry rustaceans)
extern State s;

/* TCC DOES NOT LIKE DECLARATIONS HERE*/
// Reference to s.game
extern GameState* gs;

// Reference to s.game.bricks
extern Bricks* bricks;

// Reference to s.title_screen
extern TitleScreenState* tss;

extern Leaderboard lb;

#endif
