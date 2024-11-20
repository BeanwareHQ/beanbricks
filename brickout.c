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

// color is a hex code, rgb
inline Color color(int color) {
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
const Color BRICK_COLORS[] = {
    [1] = RED,  [2] = ORANGE, [3] = GOLD,   [4] = GREEN,
    [5] = BLUE, [6] = PURPLE, [7] = VIOLET,
};
#define BG_COLOR                                                               \
    (Color) { 30, 30, 35, 255 }
#define BALL_COLOR          LIGHTGRAY
#define TXT_PRIMARY_COLOR   RAYWHITE
#define TXT_SECONDARY_COLOR LIGHTGRAY
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
const Color BRICK_COLORS[] = {
    [1] = RED,  [2] = ORANGE, [3] = GOLD,   [4] = GREEN,
    [5] = BLUE, [6] = PURPLE, [7] = VIOLET,
};
#define BG_COLOR   RAYWHITE
#define BALL_COLOR GRAY
#define TXT_PRIMARY_COLOR
#define TXT_PRIMARY_COLOR
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
    SCR_WON = 2,
} Screen;

typedef struct {
    Paddle paddle;
    Ball ball;
    int score;
    bool dead;
    Brick bricks[LAYERS][NUM_BRICKS];
} State;

static int maxscore;
void reset_state(State* s);

int make_bounce_offset(const Ball* ball) {
    double avg = (double)(ball->xspd + ball->yspd) / 2;
    double max = fabs(avg) / 2;
    double min = -max;
    double base = (double)rand() / (double)(RAND_MAX);
    double result = min + base * (max - min);

    // printf("bounce offset: %lf\n", result);

    return result + 0.5;
}

void make_bricks(State* s) {
    int cur_x = BRICK_PADDING + 10;
    int cur_y = 60;
    int starting_x = cur_x;

    for (int layer = 0; layer < LAYERS; layer++) {
        for (int i = 0; i < NUM_BRICKS; i++) {
            Rectangle rec = {cur_x, cur_y, BRICK_WIDTH, BRICK_HEIGHT};
            s->bricks[layer][i] = (Brick){rec, LAYERS - layer, true};
            cur_x += BRICK_WIDTH + 20;
        }
        cur_x = starting_x;
        cur_y += BRICK_HEIGHT + 15;
    }
}

void draw_bricks(State* s) {
    for (int y = 0; y < LAYERS; y++) {
        for (int x = 0; x < NUM_BRICKS; x++) {
            Brick* b = &s->bricks[y][x];

            if (b->active) {
                DrawRectangleRec(b->rec, color(BRICK_COLORS[b->value]));
            }
        }
    }
}

void draw(State* s) {
    if (s->dead) {
        const char* death_txt = "Game over!";
        const char* reset_txt = "Press <r> to restart";
        const int death_txtsz = 100;
        const int reset_txtsz = 20;

        int death_width = MeasureText(death_txt, death_txtsz);
        int reset_width = MeasureText(reset_txt, reset_txtsz);

        int death_posx = (WINWIDTH / 2) - death_width / 2;
        int death_posy = (WINHEIGHT / 2) - death_txtsz;
        int reset_posx = (WINWIDTH / 2) - reset_width / 2;

        DrawText(death_txt, death_posx, death_posy, death_txtsz,
                 color(TXT_PRIMARY_COLOR));
        DrawText(reset_txt, reset_posx, WINHEIGHT - reset_txtsz - 20, 20,
                 color(TXT_SECONDARY_COLOR));
    } else {
        DrawRectangleRec(s->paddle.rec, s->paddle.color);
        draw_bricks(s);
        DrawCircle(s->ball.x, s->ball.y, BALL_RADIUS, color(BALL_COLOR));

        char txt[10] = {0};
        snprintf(txt, 10, "Score: %d", s->score);
        DrawText(txt, 20, 20, 20, color(TXT_PRIMARY_COLOR));
    }
}

void update(State* s) {
    if (s->dead) {
        if (IsKeyPressed(KEY_R)) {
            s->dead = false;
            reset_state(s);
        }
    } else {
        Paddle* paddle = &s->paddle;
        Ball* ball = &s->ball;

        // funny raylib CheckCollisionCircleRec shit
        Vector2 ball_pos = (Vector2){ball->x, ball->y};

        if (ball->y + BALL_RADIUS > paddle->rec.y + paddle->rec.height) {
            s->dead = true;
            return;
        }

        if (s->score >= maxscore) {
            s->dead = true;
            return;
        }

        // paddle update logic
        if (IsKeyDown(KEY_LEFT)) {
            if (paddle->rec.x - PADDLE_SPEED >= 0) {
                paddle->rec.x -= PADDLE_SPEED;
            } else {
                paddle->rec.x = 0;
            }
        } else if (IsKeyDown(KEY_RIGHT)) {
            if (paddle->rec.x + PADDLE_SPEED <= WINWIDTH - PADDLE_WIDTH) {
                paddle->rec.x += PADDLE_SPEED;
            } else {
                paddle->rec.x = WINWIDTH - PADDLE_WIDTH;
            }
        }

        bool ball_between_paddle_x =
            ball->x > paddle->rec.x &&
            ball->x < paddle->rec.x + paddle->rec.width;
        bool ball_between_paddle_y =
            ball->y < paddle->rec.y &&
            ball->y > paddle->rec.y + paddle->rec.height;

        if (CheckCollisionCircleRec(ball_pos, BALL_RADIUS, paddle->rec)) {
            ball->y = paddle->rec.y - BALL_RADIUS;
            ball->yspd = -ball->yspd;
            if (ball->yspd < 0) {
                ball->yspd -= make_bounce_offset(ball);
            } else {
                ball->yspd += make_bounce_offset(ball);
            }

            bool ball_and_paddle_direction_opposite =
                ball->xspd < 0 && IsKeyDown(KEY_RIGHT) ||
                ball->xspd > 0 && IsKeyDown(KEY_LEFT);

            if (ball_and_paddle_direction_opposite) {
                ball->xspd = -ball->xspd;

                if (ball->xspd < 0) {
                    ball->xspd -= make_bounce_offset(ball);
                } else {
                    ball->xspd += make_bounce_offset(ball);
                }
            }
        }

        // ball update logic
        if (ball->xspd > 0) {
            if (ball->x + ball->xspd < WINWIDTH - BALL_RADIUS) {
                s->ball.x += s->ball.xspd;
            } else {
                s->ball.x = WINWIDTH - BALL_RADIUS;
                s->ball.xspd = -s->ball.xspd + make_bounce_offset(ball);
            }
        } else if (ball->xspd < 0) {
            if (ball->x + ball->xspd > 0) {
                s->ball.x += s->ball.xspd;
            } else {
                s->ball.x = 0;
                s->ball.xspd = -s->ball.xspd + make_bounce_offset(ball);
            }
        }

        if (ball->yspd > 0) {
            if (ball->y + ball->yspd < WINHEIGHT - BALL_RADIUS) {
                s->ball.y += s->ball.yspd;
            } else {
                s->ball.y = WINHEIGHT - BALL_RADIUS;
                s->ball.yspd = -s->ball.yspd + make_bounce_offset(ball);
            }
        } else if (ball->yspd < 0) {
            if (ball->y + ball->yspd > 0) {
                s->ball.y += s->ball.yspd;
            } else {
                s->ball.y = 0;
                s->ball.yspd = -s->ball.yspd + make_bounce_offset(ball);
            }
        }

        for (int y = 0; y < LAYERS; y++) {
            for (int x = 0; x < NUM_BRICKS; x++) {
                Brick* brick = &s->bricks[y][x];

                if (!brick->active) {
                    continue;
                }

                if (CheckCollisionCircleRec(ball_pos, BALL_RADIUS,
                                            brick->rec)) {
                    bool ball_between_brick_x =
                        ball_pos.x > brick->rec.x &&
                        ball_pos.x < brick->rec.x + brick->rec.width;
                    bool ball_between_brick_y =
                        ball_pos.y < brick->rec.y &&
                        ball_pos.y > brick->rec.y + brick->rec.height;

                    // ball above or below
                    if (ball_between_brick_x) {
                        if (ball->y + BALL_RADIUS < brick->rec.y) {
                            ball->y = brick->rec.y - BALL_RADIUS;
                        } else if (ball->y - BALL_RADIUS >
                                   brick->rec.y + brick->rec.height) {
                            ball->y =
                                brick->rec.y + brick->rec.height + BALL_RADIUS;
                        }

                        ball->yspd = -ball->yspd;
                    }

                    // ball left or right
                    if (ball_between_brick_y) {
                        if (ball->x + BALL_RADIUS < brick->rec.x) {
                            ball->x = brick->rec.x - BALL_RADIUS;
                        } else if (ball->x - BALL_RADIUS >
                                   brick->rec.x + brick->rec.width) {
                            ball->x =
                                brick->rec.x + brick->rec.width + BALL_RADIUS;
                        }

                        ball->xspd = -ball->xspd;
                    }

                    s->score += brick->value;
                    brick->active = false;
                }
            }
        }
    }
}

void reset_state(State* s) {
    srand(time(NULL));
    const int max_xspd = 4;
    const int min_xspd = 2;
    const int max_yspd = 6;
    const int min_yspd = 3;
    int xspd = rand() % (max_xspd - min_xspd + 1) + min_xspd;
    int yspd = rand() % (max_yspd - min_yspd - 2) + min_yspd;

    if (rand() % 2 == 0) {
        xspd = -xspd;
    }

    *s = (State){
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
        .dead = false,
    };

    make_bricks(s);
}

int main(void) {
    InitWindow(WINWIDTH, WINHEIGHT, "shitty brick-out clone");
    SetTargetFPS(60);
    srand(time(NULL));

    for (int i = 1; i <= LAYERS; i++) {
        maxscore += NUM_BRICKS * i;
    }

    State s;
    reset_state(&s);

    while (!WindowShouldClose()) {
        update(&s);
        BeginDrawing();
        ClearBackground(color(BG_COLOR));
        draw(&s);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
