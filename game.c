/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: game logic
 */

#include <math.h>
#include <string.h>

#include "beanbricks.h"
#include "common.h"
#include "game.h"

#include "3rdparty/include/raygui.h"
#include "titlescreen.h"

i32 get_bounce_offset(const Ball* ball) {
    f64 avg = (f64)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd));
    f64 max = fabs(avg) / 5;
    f64 min = -max;
    f64 base = (double)rand() / (double)(RAND_MAX);
    f64 result = min + base * (max - min);

    return result + 0.2;
}

void init_bricks(void) {
    usize nmemb = NUM_BRICKS * LAYERS;
    bricks->data = calloc(nmemb, sizeof(Brick));
    check_alloc(bricks->data);
    bricks->status = calloc(nmemb, sizeof(bool));
    check_alloc(bricks->status);
}

void make_bricks(void) {
    if (bricks->data == NULL) {
        init_bricks();
    } else {
        memset(bricks->data, 0, NUM_BRICKS * LAYERS * sizeof(Brick));
        memset(bricks->status, 0, NUM_BRICKS * LAYERS * sizeof(bool));
    }

    i32 cur_x = BRICK_PADDING + 10;
    i32 cur_y = 60;
    i32 starting_x = cur_x;

    // suppress -Wsign-compare
    for (usize layer = 0; layer < (usize)LAYERS; layer++) {
        for (usize i = 0; i < (usize)NUM_BRICKS; i++) {
            Rectangle rec = {cur_x, cur_y, BRICK_WIDTH, BRICK_HEIGHT};
            bricks->data[NUM_BRICKS * layer + i] =
                (Brick){rec, LAYERS - layer, true};
            cur_x += BRICK_WIDTH + 20;
        }
        cur_x = starting_x;
        cur_y += BRICK_HEIGHT + 15;
    }
}

void free_bricks(void) {
    free(bricks->data);
    free(bricks->status);
}

void draw_game_bricks(void) {
    for (usize y = 0; y < LAYERS; y++) {
        // suppress -Wsign-compare
        for (usize x = 0; x < (usize)NUM_BRICKS; x++) {
            Brick* b = &bricks->data[NUM_BRICKS * y + x];

            if (b->active) {
                DrawRectangleRec(b->rec, color(BRICK_COLORS[b->value - 1]));
            }
        }
    }
}

void draw_game_hud_left(void) {
    char txt[20] = {0};
    snprintf(txt, sizeof(txt), "Score: %d/%d", gs->score, maxscore);
    DrawText(txt, 20, 20, 20, TXT_PRIMARY);

    if (cfg.debug) {
        Color col = LIGHTGRAY;
        col.a = 30;
        i32 debug_txt_width = MeasureText(txt, 20);
        char debug_txt_buf[30] = {0};
        const f64 avg_speed = (double)sqrt(gs->ball.xspd * gs->ball.xspd +
                                           gs->ball.yspd * gs->ball.yspd);
        snprintf(debug_txt_buf, sizeof(debug_txt_buf),
                 "Speed: %0.4f (%0.3f,%0.3f)", avg_speed, gs->ball.xspd,
                 gs->ball.yspd);
        debug_txt_width = MeasureText(debug_txt_buf, 10);
        DrawRectangle(0, 38, debug_txt_width + 30, 10, col);
        DrawText(debug_txt_buf, 20, 40, 10, TXT_SECONDARY);
        debug_txt_width = MeasureText(debug_txt_buf, 10);
        snprintf(debug_txt_buf, sizeof(debug_txt_buf),
                 "Position: (%0.3f, %0.3f)", gs->ball.x, gs->ball.y);
        DrawRectangle(0, 48, debug_txt_width + 30, 10, col);
        DrawText(debug_txt_buf, 20, 50, 10, TXT_SECONDARY);
        debug_txt_width = MeasureText(debug_txt_buf, 10);
        snprintf(debug_txt_buf, sizeof(debug_txt_buf), "Paddle: (%0.3f, %0.3f)",
                 gs->paddle.rec.x, gs->paddle.rec.y);
        DrawRectangle(0, 58, debug_txt_width + 30, 12, col);
        DrawText(debug_txt_buf, 20, 60, 10, TXT_SECONDARY);
    }
}

void draw_game_hud_right(void) {
    char buf[10] = {0};

    const i32 BAR_WIDTH = 150;
    i32 BAR_X = WIN_WIDTH - BAR_WIDTH - 20;

    if (gs->gui.draw) {
        BAR_X -= gs->gui.quit_button.width;
        BAR_X -= 10;
    }

    const f64 FRAC_BROKEN = (f64)gs->bricks_broken / (LAYERS * NUM_BRICKS);
    const f64 PERCENT_BROKEN = FRAC_BROKEN * 100;
    Color bar_color;

    snprintf(buf, sizeof(buf), "%.01lf%%", PERCENT_BROKEN);
    const i32 TEXT_WIDTH = MeasureText(buf, 20);

    if (PERCENT_BROKEN < 25) {
        bar_color = color(BRICK_COLORS[0]); // red
    } else if (PERCENT_BROKEN < 60) {
        bar_color = color(BRICK_COLORS[1]); // yellow
    } else if (PERCENT_BROKEN < 80) {
        bar_color = color(BRICK_COLORS[3]); // green
    } else {
        bar_color = color(BRICK_COLORS[4]); // blue
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
        (i32)((BAR_WIDTH - 4) * FRAC_BROKEN),
        14, // 18 - 4
    };

    DrawRectangleRec(border, TXT_PRIMARY);
    DrawRectangleRec(background, BG_COLOR);
    DrawRectangleRec(filling, bar_color);
    DrawText(buf, BAR_X - TEXT_WIDTH - 10, 20, 20, TXT_PRIMARY);
}

void draw_game_gui(void) {
    if (GuiButton(gs->gui.quit_button, GuiIconText(ICON_EXIT, "Quit"))) {
        gs->exit_overlay = true;
    }
}

void draw_game(void) {
    DrawRectangleRec(gs->paddle.rec, gs->paddle.color);
    DrawCircle(gs->ball.x, gs->ball.y, BALL_RADIUS, BALL_COLOR);
    draw_game_bricks();
    draw_game_hud_left();
    draw_game_hud_right();

    if (gs->paused) {
        Rectangle darken = (Rectangle){0, 0, WIN_WIDTH, WIN_HEIGHT};
        DrawRectangleRec(darken, (Color){100, 100, 100, 100});

        const char* pause = "paused";
        const i32 pause_txtsz = 60;
        i32 pause_width = MeasureText(pause, pause_txtsz);
        i32 pause_posx = (WIN_WIDTH / 2) - pause_width / 2;
        i32 pause_posy = (WIN_HEIGHT / 2) - pause_txtsz / 2;

        DrawText(pause, pause_posx, pause_posy, pause_txtsz, TXT_PRIMARY);
    }

    if (gs->exit_overlay) {
        Rectangle darken = (Rectangle){0, 0, WIN_WIDTH, WIN_HEIGHT};
        DrawRectangleRec(darken, (Color){100, 100, 100, 100});

        const char* exit = "exit?";
        const i32 exit_txtsz = 60;
        i32 exit_width = MeasureText(exit, exit_txtsz);
        i32 exit_posx = (WIN_WIDTH / 2) - exit_width / 2;
        i32 exit_posy =
            (WIN_HEIGHT / 2) - 60; // exit_txtsz + padding + buttons = 120

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

        DrawText(exit, exit_posx, exit_posy, exit_txtsz, TXT_PRIMARY);
    }

    if (gs->exit_overlay || gs->paused)
        return;

    draw_game_gui();
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
        s.should_close = true;
    }
}

void draw_dead(void) {
    const char* death_txt = "Game over!";
    const i32 death_txtsz = 100;

    i32 death_width = MeasureText(death_txt, death_txtsz);

    i32 death_posx = (WIN_WIDTH / 2) - death_width / 2;
    i32 death_posy = (WIN_HEIGHT / 2) - death_txtsz / 2;

    DrawText(death_txt, death_posx, death_posy, death_txtsz, TXT_PRIMARY);

    draw_game_hud_left();
    draw_game_hud_right();
    draw_win_or_dead_gui();
}

void draw_win(void) {
    const char* win_txt = "You won!";
    const i32 win_txtsz = 100;

    i32 win_width = MeasureText(win_txt, win_txtsz);

    i32 win_posx = (WIN_WIDTH / 2) - win_width / 2;
    i32 win_posy = (WIN_HEIGHT / 2) - win_txtsz / 2;

    DrawText(win_txt, win_posx, win_posy, win_txtsz, TXT_PRIMARY);

    draw_game_hud_left();
    draw_game_hud_right();
    draw_win_or_dead_gui();
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
        if (paddle->rec.x + gs->paddle_speed <= WIN_WIDTH - PADDLE_WIDTH) {
            paddle->rec.x += gs->paddle_speed;
        } else {
            paddle->rec.x = WIN_WIDTH - PADDLE_WIDTH;
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
        if (ball->x + ball->xspd < WIN_WIDTH - BALL_RADIUS) {
            gs->ball.x += gs->ball.xspd;
        } else {
            gs->ball.x = WIN_WIDTH - BALL_RADIUS;
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
        if (ball->y + ball->yspd < WIN_HEIGHT - BALL_RADIUS) {
            gs->ball.y += gs->ball.yspd;
        } else {
            gs->ball.y = WIN_HEIGHT - BALL_RADIUS;
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

    for (usize y = 0; y < LAYERS; y++) {
        // suppress -Wsign-compare
        for (usize x = 0; x < (usize)NUM_BRICKS; x++) {
            Brick* brick = &bricks->data[y * NUM_BRICKS + x];

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
        s.should_close = true;
    }
}

void update_win(void) {
    update_dead(); // update tasks are identical anyway
}

void update_game(void) {
    Paddle* paddle = &gs->paddle;
    Ball* ball = &gs->ball;

    f64 paddle_speed_offset =
        (f64)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd)) / 5;
    gs->paddle_speed = INITIAL_PADDLE_SPEED + paddle_speed_offset;

    if (ball->y + BALL_RADIUS > paddle->rec.y + paddle->rec.height) {
        s.screen = SCR_DEAD;
        return;
    }

    if (gs->score >= maxscore) {
        s.screen = SCR_WIN;
        LeaderboardEntry* e = leaderboard_entry_new(
            astr("default name"), 0, gs->score, maxscore, LAYERS);
        leaderboard_add_entry(&lb, e);
        return;
    }

    if (IsKeyPressed(KEY_K)) {
        s.screen = SCR_DEAD;
        LeaderboardEntry* e = leaderboard_entry_new(
            astr("default name"), 0, gs->score, maxscore, LAYERS);
        leaderboard_add_entry(&lb, e);
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
            reset_titlescreen();
            reset_game();
            s.screen = SCR_TITLE;
            return;
        }

        return;
    }

    update_game_paddle();
    update_game_ball();
    update_game_bricks();
}

void reset_game(void) {
    i32 xspd;
    i32 yspd;

    // FIXME: disgusting code (add difficulty levels)
    const i32 speed_decider = rand() % 3;

    if (speed_decider == 0) {
        xspd = 3;
        yspd = 4;
    } else if (speed_decider == 1) {
        xspd = 4;
        yspd = 3;
    } else {
        xspd = 3;
        yspd = 3;
    }

    if (rand() % 2 == 0) {
        xspd = -xspd;
    }

    // FIXME: dont free and immediately realloc
    free_bricks();

    *gs = (GameState){
        .paddle =
            (Paddle){.rec = (Rectangle){PADDLE_DEFAULT_X, PADDLE_DEFAULT_Y,
                                        PADDLE_WIDTH, PADDLE_HEIGHT},
                     .color = ORANGE},
        .ball =
            (Ball){
                     .x = (i32)((WIN_WIDTH / 2) - (BALL_RADIUS / 2)),
                     .y = (i32)((WIN_HEIGHT / 2) - (BALL_RADIUS / 2)),
                     .xspd = xspd,
                     .yspd = yspd,
                     .color = GRAY,
                     },
    };

    const i32 QUIT_BUTTON_WIDTH = 60;
    const i32 EXIT_OVERLAY_BUTTON_WIDTH = 80;
    const i32 EXIT_OVERLAY_BTNS_WIDTH =
        2 * EXIT_OVERLAY_BUTTON_WIDTH + 10; // + padding

    const i32 EXIT_OVERLAY_BTNS_Y = (WIN_HEIGHT / 2) + 30; // padding
    const i32 EXIT_OVERLAY_BUTTONS_BEGIN =
        (i32)(WIN_WIDTH / 2 - EXIT_OVERLAY_BTNS_WIDTH / 2);

    gs->gui = (GameGui){
        .quit_button = (Rectangle){.x = WIN_WIDTH - 20 - QUIT_BUTTON_WIDTH,
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
    const i32 BUTTON_WIDTH = 120;
    const i32 BUTTONS_WIDTH = 3 * BUTTON_WIDTH + 20; // + 2*padding
    const i32 BUTTONS_BEGIN = (i32)(WIN_WIDTH / 2 - BUTTONS_WIDTH / 2);

    s.win_dead_gui = (WinDeadGui){
        .restart_button = (Rectangle){.x = BUTTONS_BEGIN,
                                      .y = WIN_HEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30},
        .title_button = (Rectangle){.x = BUTTONS_BEGIN + BUTTON_WIDTH + 10,
                                      .y = WIN_HEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30},
        .quit_button = (Rectangle){.x = BUTTONS_BEGIN + 2 * BUTTON_WIDTH + 20,
                                      .y = WIN_HEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30},
    };
}
