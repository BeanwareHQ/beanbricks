/*
 * brickout.c: a questionable brick-out/breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 */

#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "settings.h"

#define RAYGUI_IMPLEMENTATION
#include "3rdparty/include/raygui.h"

#define VERSION "0.1.0"
#define HELP                                                                   \
    "\033[1mbrickout: a questionable brick-out/breakout clone in C and "       \
    "raylib.\033[0m\n\n"                                                       \
    "Copyright (c) Eason Qin <eason@ezntek.com>, 2024.\n"                      \
    "This program and all source code in the project directory including "     \
    "this file is licensed under the MIT/Expat license; unless otherwise "     \
    "stated.\n"                                                                \
    "View the full text of the license in the root of the project, or pass "   \
    "--license."                                                               \
    "usage: brickout [flags]\n"                                                \
    "running the program with no args will launch the game.\n\n"               \
    "options:\n"                                                               \
    "    --help: show this help screen\n"                                      \
    "    --version: show the version of the program\n"

// color is a hex code, rgb
Color color(int color) {
    // copied from somewhere
    uint8_t r = color >> 16 & 0xFF;
    uint8_t g = color >> 8 & 0xFF;
    uint8_t b = color & 0xFF;

    return (Color){r, g, b, 0xFF};
}

#define PADDLE_DEFAULT_X (int)((WINWIDTH / 2) - (PADDLE_WIDTH / 2))
#define PADDLE_DEFAULT_Y (int)(WINHEIGHT - 75)

#define NUM_BRICKS    (int)(WINWIDTH / (BRICK_WIDTH + 20))
#define BRICK_PADDING (int)((WINWIDTH - NUM_BRICKS * (BRICK_WIDTH + 20)) / 2)

#if THEME == THEME_DARK
// dark theme
const int BRICK_COLORS[] = {
    [1] = 0xe62937, [2] = 0xffa100, [3] = 0xffcb00, [4] = 0x00e4e0,
    [5] = 0x0079f1, [6] = 0xc87aff, [7] = 0x873cbe,
};
#define BG_COLOR            0x10101a
#define BALL_COLOR          0xc8c8c8
#define TXT_PRIMARY_COLOR   0xffffff
#define TXT_SECONDARY_COLOR 0xf5f5f5
#elif THEME == THEME_CTP_MOCHA
static void LoadRayguiStyle(void) { return; }
// mocha
const int BRICK_COLORS[] = {
    [1] = 0xf38ba8, [2] = 0xfab387, [3] = 0xf9e2af, [4] = 0xa6e3a1,
    [5] = 0x74c7ec, [6] = 0x89b4fa, [7] = 0xb4befe,
};
#define BG_COLOR            0x1e1e2e
#define BALL_COLOR          0x9399b2
#define TXT_PRIMARY_COLOR   0xcdd6f4
#define TXT_SECONDARY_COLOR 0xbac2de
#include "assets/catppuccinmochamauve.h"
extern void GuiLoadStyleCatppuccinMochaMauve(void);
static void LoadRayguiStyle(void) { GuiLoadStyleCatppuccinMochaMauve(); }
#elif THEME == THEME_CTP_MACCHIATO
// macchiato
const int BRICK_COLORS[] = {
    [1] = 0xed8796, [2] = 0xf5a97f, [3] = 0xeed49f, [4] = 0xa6da95,
    [5] = 0x7dc4e4, [6] = 0x8aadf4, [7] = 0xb7bdf8,
};
#define BG_COLOR            0x24273a
#define BALL_COLOR          0x939ab7
#define TXT_PRIMARY_COLOR   0xcad3f5
#define TXT_SECONDARY_COLOR 0xb8c0e0
#include "assets/catppuccinmacchiatosapphire.h"
extern void GuiLoadStyleCatppuccinMacchiatoSapphire(void);
static void LoadRayguiStyle(void) { GuiLoadStyleCatppuccinMacchiatoSapphire(); }
#elif THEME == THEME_CTP_FRAPPE
// frappe
const int BRICK_COLORS[] = {
    [1] = 0xe78284, [2] = 0xef9f76, [3] = 0xe5c890, [4] = 0xa6d189,
    [5] = 0x85c1dc, [6] = 0x8caaee, [7] = 0xbabbf1};
#define BG_COLOR            0x303446
#define BALL_COLOR          0x949cbb
#define TXT_PRIMARY_COLOR   0xc6d0f5
#define TXT_SECONDARY_COLOR 0xb5bfe2
#include "assets/catppuccinfrappesapphire.h"
extern void GuiLoadStyleCatppuccinFrappeSapphire(void);
static void LoadRayguiStyle(void) { GuiLoadStyleCatppuccinFrappeSapphire(); }
#elif THEME == THEME_CTP_LATTE
// frappe
const int BRICK_COLORS[] = {
    [1] = 0xd20f39, [2] = 0xfe640b, [3] = 0xdf8e1d, [4] = 0x40a02b,
    [5] = 0x04a5e5, [6] = 0x1e66f5, [7] = 0x7287fd,
};
#define BG_COLOR            0xeff1f5
#define BALL_COLOR          0x6c6f85
#define TXT_PRIMARY_COLOR   0x4c4f69
#define TXT_SECONDARY_COLOR 0x4c4f69
#include "assets/catppuccinlattesapphire.h"
extern void GuiLoadStyleCatppuccinLatteSapphire(void);
static void LoadRayguiStyle(void) { GuiLoadStyleCatppuccinLatteSapphire(); }
#else
const int BRICK_COLORS[] = {
    [1] = 0xe62937, [2] = 0xffa100, [3] = 0xffcb00, [4] = 0x00e4e0,
    [5] = 0x0079f1, [6] = 0xc87aff, [7] = 0x873cbe,
};
#define BG_COLOR            0xf5f5f5
#define BALL_COLOR          0x828282
#define TXT_PRIMARY_COLOR   0x000000
#define TXT_SECONDARY_COLOR 0x505050
static void LoadRayguiStyle(void) { return; }
#endif

typedef struct {
    double x;
    double y;
    double xspd;
    double yspd;
    Color color;
} Ball;

typedef struct {
    Rectangle rec;
    Color color;
    int speed_offset;
} Paddle;

typedef struct {
    Rectangle rec;
    int value;
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
} TitleScreenGui;

typedef struct {
    Paddle paddle;
    Ball ball;
    GameGui gui;
    int score;
    int bricks_broken; // HUD
    int paddle_speed;
    bool paused;
    bool exit_overlay;
    Brick bricks[LAYERS][NUM_BRICKS];
} GameState;

typedef struct {
    TitleScreenGui gui;
    int title_anim_stage;
    bool title_anim_growing;
} TitleScreenState;

typedef struct {
    GameState game;
    TitleScreenState title_screen;
    WinDeadGui win_dead_gui;
    Screen screen;
} State;

static int maxscore;

// The global game state
State s;

// Reference to s.game
GameState* const gs = &s.game;

// Reference to s.title_screen
TitleScreenState* const tss = &s.title_screen;

bool should_close = false;

// get an offset for the ball when bouncing on certain surfaces.
int get_bounce_offset(const Ball* ball);

// Populates the bricks.
void make_bricks(void);

void draw_game_bricks(void);
void draw_game_hud_left(void);
void draw_game_hud_right(void);
void draw_game_gui(void);
void draw_win_or_dead_gui(void);
void draw_dead(void);
void draw_win(void);
void draw_titlescreen(void);
void draw_titlescreen_gui(void);
void draw_settings(void);
void draw_game(void);
void draw(void);

void update_game_paddle(void);
void update_game_ball(void);
void update_game_bricks(void);
void update_dead(void);
void update_win(void);
void update_titlescreen(void);
void update_settings(void);
void update_game(void);
void update(void);

void reset_game(void);
void reset_win_or_dead_gui(void);
void reset_titlescreen(void);
void reset_all(void);

int get_bounce_offset(const Ball* ball) {
    double avg =
        (double)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd));
    double max = fabs(avg) / 5;
    double min = -max;
    double base = (double)rand() / (double)(RAND_MAX);
    double result = min + base * (max - min);

    return result + 0.2;
}

void make_bricks(void) {
    int cur_x = BRICK_PADDING + 10;
    int cur_y = 60;
    int starting_x = cur_x;

    for (int layer = 0; layer < LAYERS; layer++) {
        for (int i = 0; i < NUM_BRICKS; i++) {
            Rectangle rec = {cur_x, cur_y, BRICK_WIDTH, BRICK_HEIGHT};
            gs->bricks[layer][i] = (Brick){rec, LAYERS - layer, true};
            cur_x += BRICK_WIDTH + 20;
        }
        cur_x = starting_x;
        cur_y += BRICK_HEIGHT + 15;
    }
}

void draw_game_bricks(void) {
    for (int y = 0; y < LAYERS; y++) {
        for (int x = 0; x < NUM_BRICKS; x++) {
            Brick* b = &gs->bricks[y][x];

            if (b->active) {
                DrawRectangleRec(b->rec, color(BRICK_COLORS[b->value]));
            }
        }
    }
}

void draw_game_hud_left(void) {
    char txt[20] = {0};
    snprintf(txt, sizeof(txt), "Score: %d/%d", gs->score, maxscore);
    DrawText(txt, 20, 20, 20, color(TXT_PRIMARY_COLOR));

#ifdef DEBUG_INFO
    const int txt_width = MeasureText(txt, 20);
    char spd[30] = {0};
    const double avg_speed = (double)sqrt(gs->ball.xspd * gs->ball.xspd +
                                          gs->ball.yspd * gs->ball.yspd);
    snprintf(spd, sizeof(spd), "Speed: %0.4f (%0.3f,%0.3f)", avg_speed,
             gs->ball.xspd, gs->ball.yspd);

    DrawText(spd, 20 + txt_width + 10, 20, 20, color(TXT_SECONDARY_COLOR));
#endif
}

void draw_game_hud_right(void) {
    char buf[10] = {0};

    const int BAR_WIDTH = 150;
    int BAR_X = WINWIDTH - BAR_WIDTH - 20;

    if (gs->gui.draw) {
        BAR_X -= gs->gui.quit_button.width;
        BAR_X -= 10;
    }

    const double FRAC_BROKEN =
        (double)gs->bricks_broken / (LAYERS * NUM_BRICKS);
    const double PERCENT_BROKEN = FRAC_BROKEN * 100;
    Color bar_color;

    snprintf(buf, sizeof(buf), "%.01lf%%", PERCENT_BROKEN);
    const int TEXT_WIDTH = MeasureText(buf, 20);

    if (PERCENT_BROKEN < 25) {
        bar_color = color(BRICK_COLORS[1]); // red
    } else if (PERCENT_BROKEN < 60) {
        bar_color = color(BRICK_COLORS[3]); // yellow
    } else if (PERCENT_BROKEN < 80) {
        bar_color = color(BRICK_COLORS[4]); // green
    } else {
        bar_color = color(BRICK_COLORS[5]); // blue
    }

    const Rectangle border = {
        BAR_X, // - width - padding,
        20,
        BAR_WIDTH,
        18,
    };

    const Rectangle background = {
        BAR_X + 2, // padding
        22,        // 20 + 2
        BAR_WIDTH - 4,
        14, // 18 - 4
    };

    const Rectangle filling = {
        BAR_X + 2, // padding
        22,        // 20 + 2
        (int)((BAR_WIDTH - 4) * FRAC_BROKEN),
        14, // 18 - 4
    };

    DrawRectangleRec(border, color(TXT_PRIMARY_COLOR));
    DrawRectangleRec(background, color(BG_COLOR));
    DrawRectangleRec(filling, bar_color);
    DrawText(buf, BAR_X - TEXT_WIDTH - 10, 20, 20, color(TXT_PRIMARY_COLOR));
}

void draw_game_gui(void) {
    if (GuiButton(gs->gui.quit_button, GuiIconText(ICON_EXIT, "Quit"))) {
        gs->exit_overlay = true;
    }
}

void draw_win_or_dead_gui(void) {
    if (GuiButton(s.win_dead_gui.restart_button,
                  GuiIconText(ICON_REPEAT_FILL, "[R]estart"))) {
        reset_game();
        s.screen = SCR_GAME;
    } else if (GuiButton(s.win_dead_gui.title_button,
                         GuiIconText(ICON_HOUSE, "[T]itle Screen"))) {
        reset_titlescreen();
        s.screen = SCR_TITLE;
    } else if (GuiButton(s.win_dead_gui.quit_button,
                         GuiIconText(ICON_EXIT, "[Q]uit"))) {
        should_close = true;
    }
}

void draw_dead(void) {
    const char* death_txt = "Game over!";
    const int death_txtsz = 100;

    int death_width = MeasureText(death_txt, death_txtsz);

    int death_posx = (WINWIDTH / 2) - death_width / 2;
    int death_posy = (WINHEIGHT / 2) - death_txtsz / 2;

    DrawText(death_txt, death_posx, death_posy, death_txtsz,
             color(TXT_PRIMARY_COLOR));

    draw_game_hud_left();
    draw_game_hud_right();
    draw_win_or_dead_gui();
}

void draw_win(void) {
    const char* win_txt = "You won!";
    const int win_txtsz = 100;

    int win_width = MeasureText(win_txt, win_txtsz);

    int win_posx = (WINWIDTH / 2) - win_width / 2;
    int win_posy = (WINHEIGHT / 2) - win_txtsz / 2;

    DrawText(win_txt, win_posx, win_posy, win_txtsz, color(TXT_PRIMARY_COLOR));

    draw_game_hud_left();
    draw_game_hud_right();
    draw_win_or_dead_gui();
}

void draw_titlescreen(void) {
    const char* title = "Brick-out";
    int title_txtsz;

    if (tss->title_anim_stage == 0) {
        title_txtsz = 100;
    } else {
        title_txtsz = 100 + (int)(tss->title_anim_stage / 5);
    }

    int title_width = MeasureText(title, title_txtsz);
    int title_posx = (WINWIDTH / 2) - title_width / 2;
    int title_posy = WINHEIGHT * 0.16;

    const char* begin = "or press enter to begin";
    const int begin_txtsz = 20;
    int begin_width = MeasureText(begin, begin_txtsz);
    int begin_posx = (WINWIDTH / 2) - begin_width / 2;
    int begin_posy = WINHEIGHT - begin_txtsz - 20;

    DrawText(title, title_posx, title_posy, title_txtsz,
             color(TXT_PRIMARY_COLOR));
    DrawText(begin, begin_posx, begin_posy, begin_txtsz,
             color(TXT_SECONDARY_COLOR));

    DrawText("Copyright (c) Eason Qin <eason@ezntek.com>, 2024", 20, 20, 10,
             color(TXT_SECONDARY_COLOR));
    DrawText("version " VERSION, 20, 34, 10, color(TXT_SECONDARY_COLOR));

    draw_titlescreen_gui();
}

void draw_titlescreen_gui(void) {
    if (GuiButton(tss->gui.start_button,
                  GuiIconText(ICON_PLAYER_PLAY, "[P]lay"))) {
        reset_game();
        s.screen = SCR_GAME;
    }

    if (GuiButton(tss->gui.quit_button, GuiIconText(ICON_EXIT, "[Q]uit"))) {
        should_close = true;
        return;
    }
}

void draw_settings(void) { draw_dead(); }

void draw_game(void) {
    DrawRectangleRec(gs->paddle.rec, gs->paddle.color);
    DrawCircle(gs->ball.x, gs->ball.y, BALL_RADIUS, color(BALL_COLOR));
    draw_game_bricks();
    draw_game_hud_left();
    draw_game_hud_right();

    if (gs->paused) {
        Rectangle darken = (Rectangle){0, 0, WINWIDTH, WINHEIGHT};
        DrawRectangleRec(darken, (Color){100, 100, 100, 100});

        const char* pause = "paused";
        const int pause_txtsz = 60;
        int pause_width = MeasureText(pause, pause_txtsz);
        int pause_posx = (WINWIDTH / 2) - pause_width / 2;
        int pause_posy = (WINHEIGHT / 2) - pause_txtsz / 2;

        DrawText(pause, pause_posx, pause_posy, pause_txtsz,
                 color(TXT_PRIMARY_COLOR));
    }

    if (gs->exit_overlay) {
        Rectangle darken = (Rectangle){0, 0, WINWIDTH, WINHEIGHT};
        DrawRectangleRec(darken, (Color){100, 100, 100, 100});

        const char* exit = "exit?";
        const int exit_txtsz = 60;
        int exit_width = MeasureText(exit, exit_txtsz);
        int exit_posx = (WINWIDTH / 2) - exit_width / 2;
        int exit_posy =
            (WINHEIGHT / 2) - 60; // exit_txtsz + padding + buttons = 120

        if (GuiButton(gs->gui.exit_overlay_yes_button,
                      GuiIconText(ICON_OK_TICK, "[Y]es"))) {
            s.screen = SCR_TITLE;
            reset_titlescreen();
            reset_game();
            return;
        }

        if (GuiButton(gs->gui.exit_overlay_no_button,
                      GuiIconText(ICON_CROSS, "[N]o"))) {
            gs->exit_overlay = false; // back
            return;
        }

        DrawText(exit, exit_posx, exit_posy, exit_txtsz,
                 color(TXT_PRIMARY_COLOR));
    }

    if (gs->exit_overlay || gs->paused)
        return;

    draw_game_gui();
}

void draw(void) {
    switch (s.screen) {
        case SCR_GAME: {
            draw_game();
        } break;
        case SCR_DEAD: {
            draw_dead();
        } break;
        case SCR_WIN: {
            draw_win();
        } break;
        case SCR_TITLE: {
            draw_titlescreen();
        } break;
        case SCR_SETTINGS: {
            draw_settings();
        } break;
    }
}

void update_game_paddle(void) {
    Paddle* paddle = &gs->paddle;
    Ball* ball = &gs->ball;
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

    // paddle update logic
    if (IsKeyDown(KEY_LEFT)) {
        if (paddle->rec.x - gs->paddle_speed >= 0) {
            paddle->rec.x -= gs->paddle_speed;
        } else {
            paddle->rec.x = 0;
        }
    } else if (IsKeyDown(KEY_RIGHT)) {
        if (paddle->rec.x + gs->paddle_speed <= WINWIDTH - PADDLE_WIDTH) {
            paddle->rec.x += gs->paddle_speed;
        } else {
            paddle->rec.x = WINWIDTH - PADDLE_WIDTH;
        }
    }

    if (CheckCollisionCircleRec(ball_pos, BALL_RADIUS, paddle->rec)) {
        ball->y = paddle->rec.y - BALL_RADIUS;
        ball->yspd = -ball->yspd;
        if (ball->yspd < 0) {
            ball->yspd -= get_bounce_offset(ball);
        } else {
            ball->yspd += get_bounce_offset(ball);
        }

        bool ball_and_paddle_direction_opposite =
            (ball->xspd < 0 && IsKeyDown(KEY_RIGHT)) ||
            (ball->xspd > 0 && IsKeyDown(KEY_LEFT));

        if (ball_and_paddle_direction_opposite) {
            ball->xspd = -ball->xspd;

            if (ball->xspd < 0) {
                ball->xspd -= get_bounce_offset(ball);
                ball->xspd -= 0.055;
                ball->yspd -= 0.05;
            } else {
                ball->xspd += get_bounce_offset(ball);
                ball->xspd += 0.055;
                ball->yspd += 0.05;
            }
        } else {
            if (ball->xspd < 0) {
                ball->xspd -= 0.05;
                ball->yspd -= 0.05;
            } else {
                ball->xspd += 0.05;
                ball->yspd += 0.05;
            }
        }
    }
}

void update_game_ball(void) {
    Ball* ball = &gs->ball;

    // ball update logic
    if (ball->xspd > 0) {
        if (ball->x + ball->xspd < WINWIDTH - BALL_RADIUS) {
            gs->ball.x += gs->ball.xspd;
        } else {
            gs->ball.x = WINWIDTH - BALL_RADIUS;
            gs->ball.xspd = -gs->ball.xspd;

            if (ball->xspd < 0) {
                ball->xspd -= 0.02;
                ball->yspd -= 0.02;
            } else {
                ball->xspd += 0.02;
                ball->yspd += 0.02;
            }
        }
    } else if (ball->xspd < 0) {
        if (ball->x + ball->xspd > BALL_RADIUS) {
            gs->ball.x += gs->ball.xspd;
        } else {
            gs->ball.x = BALL_RADIUS;
            gs->ball.xspd = -gs->ball.xspd;

            if (ball->xspd < 0) {
                ball->xspd -= 0.02;
                ball->yspd -= 0.02;
            } else {
                ball->xspd += 0.05;
                ball->yspd += 0.05;
            }
        }
    }

    if (ball->yspd > 0) {
        if (ball->y + ball->yspd < WINHEIGHT - BALL_RADIUS) {
            gs->ball.y += gs->ball.yspd;
        } else {
            gs->ball.y = WINHEIGHT - BALL_RADIUS;
            gs->ball.yspd = -gs->ball.yspd;
        }
    } else if (ball->yspd < 0) {
        if (ball->y + ball->yspd > 0) {
            gs->ball.y += gs->ball.yspd;
        } else {
            gs->ball.y = 0;
            gs->ball.yspd = -gs->ball.yspd;
        }
    }
}

void update_game_bricks(void) {
    Ball* ball = &gs->ball;
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

    for (int y = 0; y < LAYERS; y++) {
        for (int x = 0; x < NUM_BRICKS; x++) {
            Brick* brick = &gs->bricks[y][x];

            if (!brick->active) {
                continue;
            }

            if (CheckCollisionCircleRec(ball_pos, BALL_RADIUS, brick->rec)) {
                brick->active = false;

                bool ball_between_brick_x =
                    ball_pos.x + BALL_RADIUS > brick->rec.x &&
                    ball_pos.x - BALL_RADIUS < brick->rec.x + brick->rec.width;
                bool ball_between_brick_y =
                    ball_pos.y + BALL_RADIUS < brick->rec.y &&
                    ball_pos.y - BALL_RADIUS > brick->rec.y + brick->rec.height;

                if (ball_between_brick_x) {
                    if (ball->y + BALL_RADIUS < brick->rec.y) {
                        ball->y = brick->rec.y - BALL_RADIUS;
                    } else if (ball->y - BALL_RADIUS >
                               brick->rec.y + brick->rec.height) {
                        ball->y =
                            brick->rec.y + brick->rec.height + BALL_RADIUS;
                    }

                    ball->yspd = -ball->yspd;
                    ball->y += ball->yspd;
                } else if (ball_between_brick_y) {
                    if (ball->x + BALL_RADIUS > brick->rec.x) {
                        ball->x = brick->rec.x - BALL_RADIUS;
                    } else if (ball->x - BALL_RADIUS <
                               brick->rec.x + brick->rec.width) {
                        ball->x = brick->rec.x + brick->rec.width + BALL_RADIUS;
                    }

                    ball->xspd = -ball->xspd;
                    ball->x += ball->xspd;
                }
                gs->score += brick->value;
                gs->bricks_broken++;
            }
        }
    }
}

void update_dead(void) {
    if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_SPACE)) {
        reset_game();
        s.screen = SCR_GAME;
    }

    if (IsKeyPressed(KEY_T) || IsKeyPressed(KEY_ESCAPE)) {
        reset_titlescreen();
        s.screen = SCR_TITLE;
    }

    if (IsKeyPressed(KEY_Q)) {
        should_close = true;
    }
}

void update_win(void) {
    update_dead(); // update tasks are identical anyway
}

void update_titlescreen(void) {
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        should_close = true;
        return;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_P)) {
        reset_game();
        s.screen = SCR_GAME;
    }

    if (tss->title_anim_stage >= 60 || tss->title_anim_stage <= 0) {
        tss->title_anim_growing = !tss->title_anim_growing;
    }

    if (tss->title_anim_growing) {
        tss->title_anim_stage++;
    } else {
        tss->title_anim_stage--;
    }
}

void update_settings(void) { update_dead(); }

void update_game(void) {
    Paddle* paddle = &gs->paddle;
    Ball* ball = &gs->ball;

    double paddle_speed_offset =
        (double)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd)) / 5;
    gs->paddle_speed = INITIAL_PADDLE_SPEED + paddle_speed_offset;

    if (ball->y + BALL_RADIUS > paddle->rec.y + paddle->rec.height) {
        s.screen = SCR_DEAD;
        return;
    }

    if (gs->score >= maxscore) {
        s.screen = SCR_WIN;
        return;
    }

    if (IsKeyPressed(KEY_K)) {
        s.screen = SCR_DEAD;
        return;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        gs->paused = !gs->paused;
    }

    if (gs->paused) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            gs->paused = false;
        }

        return;
    }

    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        gs->exit_overlay = !gs->exit_overlay;
    }

    if (gs->exit_overlay) {
        if (IsKeyPressed(KEY_N)) {
            gs->exit_overlay = false; // back
        } else if (IsKeyPressed(KEY_Y)) {
            s.screen = SCR_TITLE;
            reset_titlescreen();
            reset_game();
            return;
        }

        return;
    }

    update_game_paddle();
    update_game_ball();
    update_game_bricks();
}

void update(void) {
    gs->gui.draw = (s.screen == SCR_GAME);

    switch (s.screen) {
        case SCR_GAME: {
            update_game();
        } break;
        case SCR_DEAD: {
            update_dead();
        } break;
        case SCR_WIN: {
            update_win();
        } break;
        case SCR_TITLE: {
            update_titlescreen();
        } break;
        case SCR_SETTINGS: {
            update_settings();
        } break;
    }
}

void reset_game(void) {
    int xspd;
    int yspd;

    // TODO: fix disgusting code (add difficulty levels)
    const int speed_decider = rand() % 3;

    if (speed_decider == 0) {
        xspd = 3;
        yspd = 4;
    } else if (speed_decider == 1) {
        xspd = 4;
        yspd = 3;
    } else if (speed_decider == 2) {
        xspd = 3;
        yspd = 3;
    }

    if (rand() % 2 == 0) {
        xspd = -xspd;
    }

    *gs = (GameState){
        .paddle =
            (Paddle){.rec = (Rectangle){PADDLE_DEFAULT_X, PADDLE_DEFAULT_Y,
                                        PADDLE_WIDTH, PADDLE_HEIGHT},
                     .color = ORANGE},
        .ball =
            (Ball){
                     .x = (int)((WINWIDTH / 2) - (BALL_RADIUS / 2)),
                     .y = (int)((WINHEIGHT / 2) - (BALL_RADIUS / 2)),
                     .xspd = xspd,
                     .yspd = yspd,
                     .color = GRAY,
                     },
    };

    const int QUIT_BUTTON_WIDTH = 60;
    const int EXIT_OVERLAY_BUTTON_WIDTH = 80;
    const int EXIT_OVERLAY_BTNS_WIDTH =
        2 * EXIT_OVERLAY_BUTTON_WIDTH + 10; // + padding

    const int EXIT_OVERLAY_BTNS_Y = (WINHEIGHT / 2) + 30; // padding
    const int EXIT_OVERLAY_BUTTONS_BEGIN =
        (int)(WINWIDTH / 2 - EXIT_OVERLAY_BTNS_WIDTH / 2);

    gs->gui = (GameGui){
        .quit_button = (Rectangle){.x = WINWIDTH - 20 - QUIT_BUTTON_WIDTH,
                                   .y = 20,
                                   .width = QUIT_BUTTON_WIDTH,
                                   .height = 19},
        .exit_overlay_no_button =
            (Rectangle){.x = EXIT_OVERLAY_BUTTONS_BEGIN,
                                   .y = EXIT_OVERLAY_BTNS_Y,
                                   .width = EXIT_OVERLAY_BUTTON_WIDTH,
                                   .height = 30},
        .exit_overlay_yes_button = (Rectangle){
                                   .x = EXIT_OVERLAY_BUTTONS_BEGIN + EXIT_OVERLAY_BUTTON_WIDTH + 10,
                                   .y = EXIT_OVERLAY_BTNS_Y,
                                   .width = EXIT_OVERLAY_BUTTON_WIDTH,
                                   .height = 30}
    };

    make_bricks();
}

void reset_win_or_dead_gui(void) {
    const int BUTTON_WIDTH = 120;
    const int BUTTONS_WIDTH = 3 * BUTTON_WIDTH + 20; // + 2*padding
    const int BUTTONS_BEGIN = (int)(WINWIDTH / 2 - BUTTONS_WIDTH / 2);

    s.win_dead_gui = (WinDeadGui){
        .restart_button = (Rectangle){.x = BUTTONS_BEGIN,
                                      .y = WINHEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30},
        .title_button = (Rectangle){.x = BUTTONS_BEGIN + BUTTON_WIDTH + 10,
                                      .y = WINHEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30},
        .quit_button = (Rectangle){.x = BUTTONS_BEGIN + 2 * BUTTON_WIDTH + 20,
                                      .y = WINHEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30},
    };
}

void reset_titlescreen(void) {
    *tss = (TitleScreenState){
        .title_anim_stage = 1,
        .title_anim_growing = true,
    };

    const int BUTTON_WIDTH = 120;
    const int BUTTONS_WIDTH = 2 * BUTTON_WIDTH + 10; // + padding
    const int BUTTONS_BEGIN = (int)(WINWIDTH / 2 - BUTTONS_WIDTH / 2);

    const int TITLESCREEN_TEXT_Y = WINHEIGHT * 0.16; // check draw_titlescreen

    tss->gui = (TitleScreenGui){
        .start_button =
            (Rectangle){
                        .x = BUTTONS_BEGIN,
                        .y = TITLESCREEN_TEXT_Y + 140,
                        .width = BUTTON_WIDTH,
                        .height = 30,
                        },
        .quit_button = (Rectangle){
                        .x = BUTTONS_BEGIN + BUTTON_WIDTH + 10,
                        .y = TITLESCREEN_TEXT_Y + 140,
                        .width = BUTTON_WIDTH,
                        .height = 30,
                        }
    };
}

void reset_all(void) {
    srand(time(NULL));

    s = (State){
        .screen = SCR_TITLE,
    };

    reset_game();
    reset_titlescreen();
    reset_win_or_dead_gui();
}

int main(int argc, char** argv) {
    argc--;
    argv++;

    if (argc > 0) {
        if (!strcmp(argv[0], "--version")) {
            printf("brickout version " VERSION "\n");
            exit(EXIT_SUCCESS);
        } else if (!strcmp(argv[0], "--help")) {
            printf(HELP);
            exit(EXIT_SUCCESS);
        }
    }

    InitWindow(WINWIDTH, WINHEIGHT, "shitty brick-out clone");
    SetTargetFPS((int)(60 / (1 / SPEED)));
    srand(time(NULL));
    SetExitKey(KEY_NULL);

    for (int i = 1; i <= LAYERS; i++) {
        maxscore += NUM_BRICKS * i;
    }

    LoadRayguiStyle();

    reset_all();

    while (!should_close) {
        if (WindowShouldClose() || should_close)
            should_close = true;

        update();
        BeginDrawing();
        ClearBackground(color(BG_COLOR));
        draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
