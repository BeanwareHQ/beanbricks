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
    Screen screen;
    int score;
    int paddle_speed;
    bool paused;
    Brick bricks[LAYERS][NUM_BRICKS];
} State;

static int maxscore;
void reset_state(State* s);

int get_bounce_offset(const Ball* ball) {
    double avg =
        (double)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd));
    double max = fabs(avg) / 5;
    double min = -max;
    double base = (double)rand() / (double)(RAND_MAX);
    double result = min + base * (max - min);

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

void draw_game_bricks(State* s) {
    for (int y = 0; y < LAYERS; y++) {
        for (int x = 0; x < NUM_BRICKS; x++) {
            Brick* b = &s->bricks[y][x];

            if (b->active) {
                DrawRectangleRec(b->rec, color(BRICK_COLORS[b->value]));
            }
        }
    }
}

void draw_game_score(State* s) {
    char txt[20] = {0};
    snprintf(txt, 20, "Score: %d/%d", s->score, maxscore);
    DrawText(txt, 20, 20, 20, color(TXT_PRIMARY_COLOR));
}

void draw_game(State* s) {
    if (s->paused) {
        Rectangle darken = (Rectangle){0, 0, WINWIDTH, WINHEIGHT};
        DrawRectangleRec(darken, (Color){100, 100, 100, 100});

        const char* pause = "paused";
        const int pause_txtsz = 80;
        int pause_width = MeasureText(pause, pause_txtsz);
        int pause_posx = (WINWIDTH / 2) - pause_width / 2;
        int pause_posy = (WINHEIGHT / 2) - pause_txtsz / 2;

        DrawText(pause, pause_posx, pause_posy, pause_txtsz,
                 color(TXT_PRIMARY_COLOR));
    }

    DrawRectangleRec(s->paddle.rec, s->paddle.color);
    DrawCircle(s->ball.x, s->ball.y, BALL_RADIUS, color(BALL_COLOR));
    draw_game_bricks(s);
    draw_game_score(s);
}

void draw_dead(State* s) {
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
    draw_game_score(s);
}

void draw_win(State* s) {
    const char* win_txt = "You won!";
    const char* reset_txt = "Press <r> to restart";
    const int win_txtsz = 100;
    const int reset_txtsz = 20;

    int win_width = MeasureText(win_txt, win_txtsz);
    int reset_width = MeasureText(reset_txt, reset_txtsz);

    int win_posx = (WINWIDTH / 2) - win_width / 2;
    int win_posy = (WINHEIGHT / 2) - win_txtsz;
    int reset_posx = (WINWIDTH / 2) - reset_width / 2;

    DrawText(win_txt, win_posx, win_posy, win_txtsz, color(TXT_PRIMARY_COLOR));
    DrawText(reset_txt, reset_posx, WINHEIGHT - reset_txtsz - 20, 20,
             color(TXT_SECONDARY_COLOR));
    draw_game_score(s);
}

void draw_title(State* s) {
    const char* title = "Brick-out";
    const int title_txtsz = 120;
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
}

void draw_settings(State* s) { return draw_dead(s); }

void draw(State* s) {
    switch (s->screen) {
        case SCR_GAME: {
            draw_game(s);
        } break;
        case SCR_DEAD: {
            draw_dead(s);
        } break;
        case SCR_WIN: {
            draw_win(s);
        } break;
        case SCR_TITLE: {
            draw_title(s);
        } break;
        case SCR_SETTINGS: {
            draw_settings(s);
        } break;
    }
}

void update_game_paddle(State* s) {
    Paddle* paddle = &s->paddle;
    Ball* ball = &s->ball;
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

    // paddle update logic
    if (IsKeyDown(KEY_LEFT)) {
        if (paddle->rec.x - s->paddle_speed >= 0) {
            paddle->rec.x -= s->paddle_speed;
        } else {
            paddle->rec.x = 0;
        }
    } else if (IsKeyDown(KEY_RIGHT)) {
        if (paddle->rec.x + s->paddle_speed <= WINWIDTH - PADDLE_WIDTH) {
            paddle->rec.x += s->paddle_speed;
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

void update_game_ball(State* s) {
    Paddle* paddle = &s->paddle;
    Ball* ball = &s->ball;
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

    // ball update logic
    if (ball->xspd > 0) {
        if (ball->x + ball->xspd < WINWIDTH - BALL_RADIUS) {
            s->ball.x += s->ball.xspd;
        } else {
            s->ball.x = WINWIDTH - BALL_RADIUS;
            s->ball.xspd = -s->ball.xspd + get_bounce_offset(ball);

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
            s->ball.x += s->ball.xspd;
        } else {
            s->ball.x = BALL_RADIUS;
            s->ball.xspd = -s->ball.xspd + get_bounce_offset(ball);

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
            s->ball.y += s->ball.yspd;
        } else {
            s->ball.y = WINHEIGHT - BALL_RADIUS;
            s->ball.yspd = -s->ball.yspd + get_bounce_offset(ball);
        }
    } else if (ball->yspd < 0) {
        if (ball->y + ball->yspd > 0) {
            s->ball.y += s->ball.yspd;
        } else {
            s->ball.y = 0;
            s->ball.yspd = -s->ball.yspd + get_bounce_offset(ball);
        }
    }
}

void update_game_bricks(State* s) {
    Paddle* paddle = &s->paddle;
    Ball* ball = &s->ball;
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

    for (int y = 0; y < LAYERS; y++) {
        for (int x = 0; x < NUM_BRICKS; x++) {
            Brick* brick = &s->bricks[y][x];

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

                // ball above or below
                if (ball_between_brick_x) {
                    if (ball->y + BALL_RADIUS < brick->rec.y) {
                        ball->y = brick->rec.y - BALL_RADIUS;
                    } else if (ball->y - BALL_RADIUS <
                               brick->rec.y + brick->rec.height) {
                        ball->y =
                            brick->rec.y + brick->rec.height + BALL_RADIUS;
                    }

                    ball->yspd = -ball->yspd;
                    ball->y += ball->yspd;
                }

                // ball left or right
                if (ball_between_brick_y) {
                    if (ball->x + BALL_RADIUS < brick->rec.x) {
                        ball->x = brick->rec.x - BALL_RADIUS;
                    } else if (ball->x - BALL_RADIUS >
                               brick->rec.x + brick->rec.width) {
                        ball->x = brick->rec.x + brick->rec.width + BALL_RADIUS;
                    }

                    const int half_height = brick->rec.height / 2;
                    const int middle = brick->rec.y + half_height;

                    if (ball->y <= middle) {
                        ball->y =
                            brick->rec.y + brick->rec.height + BALL_RADIUS;
                    } else {
                        ball->y = brick->rec.y - BALL_RADIUS;
                    }

                    ball->xspd = -ball->xspd;
                    ball->x += ball->xspd;
                }

                s->score += brick->value;
            }
        }
    }
}

void update_dead(State* s) {
    if (IsKeyPressed(KEY_R)) {
        reset_state(s);
        s->screen = SCR_GAME;
    }
}

void update_game(State* s) {
    Paddle* paddle = &s->paddle;
    Ball* ball = &s->ball;

    // funny raylib CheckCollisionCircleRec shit
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

    double paddle_speed_offset =
        (double)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd)) / 5;
    s->paddle_speed = INITIAL_PADDLE_SPEED + paddle_speed_offset;

    if (ball->y + BALL_RADIUS > paddle->rec.y + paddle->rec.height) {
        s->screen = SCR_DEAD;
        return;
    }

    if (s->score >= maxscore) {
        s->screen = SCR_WIN;
        return;
    }

    if (IsKeyPressed(KEY_K)) {
        s->screen = SCR_DEAD;
        return;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        s->paused = !s->paused;
    }

    if (s->paused) {
        return;
    }

    update_game_paddle(s);
    update_game_ball(s);
    update_game_bricks(s);
}

void update_title(State* s) {
    if (GetCharPressed() != 0) {
        reset_state(s);
        s->screen = SCR_GAME;
    }
}

void update_win(State* s) { return update_dead(s); }
void update_settings(State* s) { return update_dead(s); }

void update(State* s) {
    switch (s->screen) {
        case SCR_GAME: {
            update_game(s);
        } break;
        case SCR_DEAD: {
            update_dead(s);
        } break;
        case SCR_WIN: {
            update_win(s);
        } break;
        case SCR_TITLE: {
            update_title(s);
        } break;
        case SCR_SETTINGS: {
            update_settings(s);
        } break;
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
        .screen = SCR_TITLE,
    };

    make_bricks(s);
}

int main(void) {
    InitWindow(WINWIDTH, WINHEIGHT, "shitty brick-out clone");
    SetTargetFPS(60 / (1 / SPEED));
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
