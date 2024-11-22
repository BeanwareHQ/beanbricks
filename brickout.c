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
#include <time.h>

#include "settings.h"

#define VERSION "0.1.0-pre"

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
#define BG_COLOR            0x50505a
#define BALL_COLOR          0xc8c8c8
#define TXT_PRIMARY_COLOR   0xffffff
#define TXT_SECONDARY_COLOR 0xf5f5f5
#elif THEME == CTP_MOCHA
// mocha
const int BRICK_COLORS[] = {
    [1] = 0xf38ba8, [2] = 0xfab387, [3] = 0xf9e2af, [4] = 0xa6e3a1,
    [5] = 0x74c7ec, [6] = 0x89b4fa, [7] = 0xb4befe,
};
#define BG_COLOR            0x1e1e2e
#define BALL_COLOR          0x9399b2
#define TXT_PRIMARY_COLOR   0xcdd6f4
#define TXT_SECONDARY_COLOR 0xbac2de
#elif THEME == CTP_MACCHIATO
// macchiato
const int BRICK_COLORS[] = {
    [1] = 0xed8796, [2] = 0xf5a97f, [3] = 0xeed49f, [4] = 0xa6da95,
    [5] = 0x7dc4e4, [6] = 0x8aadf4, [7] = 0xb7bdf8,
};
#define BG_COLOR            0x24273a
#define BALL_COLOR          0x939ab7
#define TXT_PRIMARY_COLOR   0xcad3f5
#define TXT_SECONDARY_COLOR 0xb8c0e0
#elif THEME == CTP_FRAPPE
// frappe
const int BRICK_COLORS[] = {
    [1] = 0xe78284, [2] = 0xef9f76, [3] = 0xe5c890, [4] = 0xa6d189,
    [5] = 0x85c1dc, [6] = 0x8caaee, [7] = 0xbabbf1};
#define BG_COLOR            0x303446
#define BALL_COLOR          0x949cbb
#define TXT_PRIMARY_COLOR   0xc6d0f5
#define TXT_SECONDARY_COLOR 0xb5bfe2
#elif THEME == CTP_LATTE
// frappe
const int BRICK_COLORS[] = {
    [1] = 0xd20f39, [2] = 0xfe640b, [3] = 0xdf8e1d, [4] = 0x40a02b,
    [5] = 0x04a5e5, [6] = 0x1e66f5, [7] = 0x7287fd,
};
#define BG_COLOR            0xeff1f5
#define BALL_COLOR          0x6c6f85
#define TXT_PRIMARY_COLOR   0x4c4f69
#define TXT_SECONDARY_COLOR 0x4c4f69
#else
const int BRICK_COLORS[] = {
    [1] = 0xe62937, [2] = 0xffa100, [3] = 0xffcb00, [4] = 0x00e4e0,
    [5] = 0x0079f1, [6] = 0xc87aff, [7] = 0x873cbe,
};
#define BG_COLOR            0xf5f5f5
#define BALL_COLOR          0x828282
#define TXT_PRIMARY_COLOR   0x000000
#define TXT_SECONDARY_COLOR 0x505050
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
    Paddle paddle;
    Ball ball;
    int score;
    int paddle_speed;
    bool paused;
    bool exit_overlay;
    Brick bricks[LAYERS][NUM_BRICKS];
} GameState;

typedef struct {
    int title_anim_stage;
    bool title_anim_growing;
} TitleScreenState;

typedef struct {
    GameState game;
    TitleScreenState title_screen;
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

// Reset the state of the game
void reset_state(void);
void reset_game(void);
void reset_title(void);

int get_bounce_offset(const Ball* ball) {
    double avg =
        (double)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd));
    double max = fabs(avg) / 5;
    double min = -max;
    double base = (double)rand() / (double)(RAND_MAX);
    double result = min + base * (max - min);

    return result + 0.5;
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

void draw_game_score(void) {
    char txt[20] = {0};
    snprintf(txt, 20, "Score: %d/%d", gs->score, maxscore);
    DrawText(txt, 20, 20, 20, color(TXT_PRIMARY_COLOR));
}

void draw_game_progressbar(void) {
    const int WIDTH = 150;
    const Rectangle border = {
        WINWIDTH - WIDTH - 20, // - width - padding,
        20,
        WIDTH,
        18,
    };

    const Rectangle background = {
        WINWIDTH - WIDTH - 20 + 2, // padding
        22,                        // 20 + 2
        WIDTH - 4,
        14, // 18 - 4
    };

    const Rectangle filling = {
        WINWIDTH - WIDTH - 20 + 2, // padding
        22,                        // 20 + 2
        (int)(WIDTH * s.game.score / maxscore),
        14, // 18 - 4
    };

    DrawRectangleRec(border, color(TXT_PRIMARY_COLOR));
    DrawRectangleRec(background, color(BG_COLOR));
    DrawRectangleRec(filling, color(BRICK_COLORS[1]));
}

void draw_dead(void) {
    const char* death_txt = "Game over!";
    const char* reset_txt = "Press <r> to restart, <t> for title screen";
    const int death_txtsz = 100;
    const int reset_txtsz = 20;

    int death_width = MeasureText(death_txt, death_txtsz);
    int reset_width = MeasureText(reset_txt, reset_txtsz);

    int death_posx = (WINWIDTH / 2) - death_width / 2;
    int death_posy = (WINHEIGHT / 2) - death_txtsz / 2;
    int reset_posx = (WINWIDTH / 2) - reset_width / 2;

    DrawText(death_txt, death_posx, death_posy, death_txtsz,
             color(TXT_PRIMARY_COLOR));
    DrawText(reset_txt, reset_posx, WINHEIGHT - reset_txtsz - 20, 20,
             color(TXT_SECONDARY_COLOR));
    draw_game_score();
    draw_game_progressbar();
}

void draw_win(void) {
    const char* win_txt = "You won!";
    const char* reset_txt = "Press <r> to restart, <t> for title screen";
    const int win_txtsz = 100;
    const int reset_txtsz = 20;

    int win_width = MeasureText(win_txt, win_txtsz);
    int reset_width = MeasureText(reset_txt, reset_txtsz);

    int win_posx = (WINWIDTH / 2) - win_width / 2;
    int win_posy = (WINHEIGHT / 2) - win_txtsz / 2;
    int reset_posx = (WINWIDTH / 2) - reset_width / 2;

    DrawText(win_txt, win_posx, win_posy, win_txtsz, color(TXT_PRIMARY_COLOR));
    DrawText(reset_txt, reset_posx, WINHEIGHT - reset_txtsz - 20, 20,
             color(TXT_SECONDARY_COLOR));
    draw_game_score();
    draw_game_progressbar();
}

void draw_title(void) {
    const char* title = "Brick-out";
    int title_txtsz;

    if (tss->title_anim_stage == 0) {
        title_txtsz = 100;
    } else {
        title_txtsz = 100 + (int)(tss->title_anim_stage / 3);
    }

    int title_width = MeasureText(title, title_txtsz);
    int title_posx = (WINWIDTH / 2) - title_width / 2;
    int title_posy = (WINHEIGHT / 2) - title_txtsz / 2;

    const char* begin = "press any key to begin";
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
}

void draw_settings(void) { return draw_dead(); }

void draw_game(void) {
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
        int exit_posy = (WINHEIGHT / 2) - exit_txtsz / 2;

        const char* confirm = "<y> for yes, <n> for no";
        const int confirm_txtsz = 20;
        int confirm_width = MeasureText(confirm, confirm_txtsz);
        int confirm_posx = (WINWIDTH / 2) - confirm_width / 2;
        int confirm_posy = WINHEIGHT - confirm_txtsz - 20;

        DrawText(exit, exit_posx, exit_posy, exit_txtsz,
                 color(TXT_PRIMARY_COLOR));
        DrawText(confirm, confirm_posx, confirm_posy, confirm_txtsz,
                 color(TXT_SECONDARY_COLOR));
    }

    DrawRectangleRec(gs->paddle.rec, gs->paddle.color);
    DrawCircle(gs->ball.x, gs->ball.y, BALL_RADIUS, color(BALL_COLOR));
    draw_game_bricks();
    draw_game_score();
    draw_game_progressbar();
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
            draw_title();
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

    bool ball_between_paddle_x =
        ball->x > paddle->rec.x && ball->x < paddle->rec.x + paddle->rec.width;
    bool ball_between_paddle_y =
        ball->y < paddle->rec.y && ball->y > paddle->rec.y + paddle->rec.height;

    if (CheckCollisionCircleRec(ball_pos, BALL_RADIUS, paddle->rec)) {
        ball->y = paddle->rec.y - BALL_RADIUS;
        ball->yspd = -ball->yspd;
        if (ball->yspd < 0) {
            ball->yspd -= get_bounce_offset(ball);
        } else {
            ball->yspd += get_bounce_offset(ball);
        }

        bool ball_and_paddle_direction_opposite =
            ball->xspd < 0 && IsKeyDown(KEY_RIGHT) ||
            ball->xspd > 0 && IsKeyDown(KEY_LEFT);

        if (ball_and_paddle_direction_opposite) {
            ball->xspd = -ball->xspd;

            if (ball->xspd < 0) {
                ball->xspd -= get_bounce_offset(ball);
                ball->xspd -= 0.15;
                ball->yspd -= 0.1;
            } else {
                ball->xspd += get_bounce_offset(ball);
                ball->xspd += 0.15;
                ball->yspd += 0.1;
            }
        } else {
            if (ball->xspd < 0) {
                ball->xspd -= 0.1;
                ball->yspd -= 0.1;
            } else {
                ball->xspd += 0.1;
                ball->yspd += 0.1;
            }
        }
    }
}

void update_game_ball(void) {
    Paddle* paddle = &gs->paddle;
    Ball* ball = &gs->ball;
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

    // ball update logic
    if (ball->xspd > 0) {
        if (ball->x + ball->xspd < WINWIDTH - BALL_RADIUS) {
            gs->ball.x += gs->ball.xspd;
        } else {
            gs->ball.x = WINWIDTH - BALL_RADIUS;
            gs->ball.xspd = -gs->ball.xspd + get_bounce_offset(ball);

            if (ball->xspd < 0) {
                ball->xspd -= 0.05;
                ball->yspd -= 0.05;
            } else {
                ball->xspd += 0.05;
                ball->yspd += 0.05;
            }
        }
    } else if (ball->xspd < 0) {
        if (ball->x + ball->xspd > BALL_RADIUS) {
            gs->ball.x += gs->ball.xspd;
        } else {
            gs->ball.x = BALL_RADIUS;
            gs->ball.xspd = -gs->ball.xspd + get_bounce_offset(ball);

            if (ball->xspd < 0) {
                ball->xspd -= 0.05;
                ball->yspd -= 0.05;
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
            gs->ball.yspd = -gs->ball.yspd + get_bounce_offset(ball);
        }
    } else if (ball->yspd < 0) {
        if (ball->y + ball->yspd > 0) {
            gs->ball.y += gs->ball.yspd;
        } else {
            gs->ball.y = 0;
            gs->ball.yspd = -gs->ball.yspd + get_bounce_offset(ball);
        }
    }
}

void update_game_bricks(void) {
    Paddle* paddle = &gs->paddle;
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
            }
        }
    }
}

void update_dead(void) {
    if (IsKeyPressed(KEY_R)) {
        reset_game();
        s.screen = SCR_GAME;
    }

    if (IsKeyPressed(KEY_T)) {
        reset_title();
        s.screen = SCR_TITLE;
    }
}

void update_win(void) {
    if (IsKeyPressed(KEY_R)) {
        reset_game();
        s.screen = SCR_GAME;
    }

    if (IsKeyPressed(KEY_T)) {
        reset_title();
        s.screen = SCR_TITLE;
    }
}

void update_title(void) {
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        should_close = true;
        return;
    }

    if (GetCharPressed() != 0) {
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

void update_settings(void) { return update_dead(); }

void update_game(void) {
    Paddle* paddle = &gs->paddle;
    Ball* ball = &gs->ball;

    // funny raylib CheckCollisionCircleRec shit
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

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
            reset_title();
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
            update_title();
        } break;
        case SCR_SETTINGS: {
            update_settings();
        } break;
    }
}

void reset_game(void) {
    const int max_xspd = 4;
    const int min_xspd = 2;
    const int max_yspd = 6;
    const int min_yspd = 3;

    int xspd = rand() % (max_xspd - min_xspd + 1) + min_xspd;
    int yspd = rand() % (max_yspd - min_yspd - 2) + min_yspd;

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

    make_bricks();
}

void reset_title(void) {
    *tss = (TitleScreenState){
        .title_anim_stage = 1,
        .title_anim_growing = true,
    };
}

void reset_state(void) {
    srand(time(NULL));

    s = (State){
        .screen = SCR_TITLE,
    };

    reset_game();
    reset_title();
}

int main(void) {
    InitWindow(WINWIDTH, WINHEIGHT, "shitty brick-out clone");
    SetTargetFPS((int)(60 / (1 / SPEED)));
    srand(time(NULL));
    SetExitKey(KEY_NULL);

    for (int i = 1; i <= LAYERS; i++) {
        maxscore += NUM_BRICKS * i;
    }

    State s = {0};
    reset_state();

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
