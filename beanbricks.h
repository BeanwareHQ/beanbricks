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
#include "title.h"

#define WIN_WIDTH            (cfg.win_width)
#define WIN_HEIGHT           (cfg.win_height)
#define PADDLE_WIDTH         (cfg.paddle_width)
#define PADDLE_HEIGHT        (cfg.paddle_height)
#define INITIAL_PADDLE_SPEED (cfg.initial_paddle_speed)
#define LAYERS               (cfg.layers)
#define BRICK_WIDTH          (cfg.brick_width)
#define BRICK_HEIGHT         (cfg.brick_height)
#define BALL_RADIUS          (cfg.ball_radius)

#define BG_COLOR            (color(state.theme.bg_color))
#define DARK_SURFACE_COLOR  (color(state.theme.dark_surface_color))
#define LIGHT_SURFACE_COLOR (color(state.theme.light_surface_color))
#define BALL_COLOR          (color(state.theme.ball_color))
#define TXT_PRIMARY         (color(state.theme.txt_primary))
#define TXT_SECONDARY       (color(state.theme.txt_secondary))
#define BRICK_COLORS        (state.theme.brick_colors)

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

typedef struct {
    Rectangle quit_button;
    Rectangle exit_overlay_yes_button;
    Rectangle exit_overlay_no_button;
    Title exit_overlay_title;
} GameGui;

typedef struct {
    Brick* data;
    bool* status;
} Bricks;

typedef struct {
    Paddle paddle;
    Ball ball;
    GameGui gui;
    Bricks bricks;
    u32 score;
    u32 bricks_broken; // HUD
    i32 paddle_speed;
    u32 elapsed_time;
    bool paused;
    bool exit_overlay;
} GameState;

typedef struct {
    Rectangle title_button;
    Rectangle restart_button;
    Rectangle quit_button;
    Title title;
} DeadGui;

typedef struct {
    Rectangle title_button;
    Rectangle restart_button;
    Rectangle quit_button;
    Rectangle text_input;
    Title title;
} WinGui;

typedef struct {
    DeadGui gui;
} DeadScreenState;

typedef struct {
    WinGui gui;
    // lb entry currently being worked on
    LeaderboardEntry entry;
} WinScreenState;

typedef struct {
    Rectangle start_button;
    Rectangle quit_button;
    Rectangle settings_button;
} TitleScreenGui;

typedef struct {
    TitleScreenGui gui;
    i32 title_anim_stage;
    bool title_anim_growing;
} TitleScreenState;

typedef struct {
    Title title;
} SettingsGui;

typedef struct {
    SettingsGui gui;
} SettingsState;

typedef enum {
    SCR_NONE = 0, // intern
    SCR_GAME = 1,
    SCR_DEAD = 2,
    SCR_WIN = 3,
    SCR_TITLE = 4,
    SCR_SETTINGS = 5,
    SCR_QUIT = 6, // on this screen, the game should quit
} ScreenVariant;

typedef union {
    GameState game;
    DeadScreenState dead;
    WinScreenState win;
    TitleScreenState title;
    SettingsState settings;
} ScreenData;

typedef struct {
    ScreenVariant variant;
    ScreenData data;
} Screen;

typedef struct {
    ThemeSpec theme;
    Screen screen;
    Leaderboard lb;
    u32 maxscore;
} State;

Color color(i32);

// switches to a new screen gracefully
void switch_screen(ScreenVariant scr);

// The global config singleton (sorry no singleton pattern. this is C.)
extern Config cfg;

extern State state;

#endif
