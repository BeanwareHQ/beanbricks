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
#include <raylib.h>
#include <string.h>

#include "3rdparty/include/a_string.h"
#include "beanbricks.h"
#include "common.h"
#include "game.h"

#include "3rdparty/include/raygui.h"
#include "leaderboard.h"
#include "title.h"

static i32 get_bounce_offset(const Ball* ball) {
    f64 avg = (f64)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd));
    f64 max = fabs(avg) / 5;
    f64 min = -max;
    f64 base = (double)rand() / (double)(RAND_MAX);
    f64 result = min + base * (max - min);

    return result + 0.2;
}

Bricks s_game_bricks_init(void) {
    Bricks res = {0};

    usize nmemb = NUM_BRICKS * LAYERS;
    res.data = calloc(nmemb, sizeof(Brick));
    check_alloc(res.data);
    res.status = calloc(nmemb, sizeof(bool));
    check_alloc(res.status);

    return res;
}

void s_game_bricks_populate(Bricks* b) {
    if (b->data == NULL) {
        *b = s_game_bricks_init();
    }

    memset(b->data, 0, NUM_BRICKS * LAYERS * sizeof(Brick));
    memset(b->status, 0, NUM_BRICKS * LAYERS * sizeof(bool));

    i32 cur_x = BRICK_PADDING + 10;
    i32 cur_y = 60;
    i32 starting_x = cur_x;

    // suppress -Wsign-compare
    for (usize layer = 0; layer < (usize)LAYERS; layer++) {
        for (usize i = 0; i < (usize)NUM_BRICKS; i++) {
            Rectangle rec = {cur_x, cur_y, BRICK_WIDTH, BRICK_HEIGHT};
            b->data[NUM_BRICKS * layer + i] =
                (Brick){rec, LAYERS - layer, true};
            cur_x += BRICK_WIDTH + 20;
        }
        cur_x = starting_x;
        cur_y += BRICK_HEIGHT + 15;
    }
}

void s_game_bricks_deinit(Bricks* b) {
    free(b->data);
    free(b->status);
}

void s_game_draw_bricks(GameState* s) {
    for (usize y = 0; y < LAYERS; y++) {
        // suppress -Wsign-compare
        for (usize x = 0; x < (usize)NUM_BRICKS; x++) {
            Brick* b = &s->bricks.data[NUM_BRICKS * y + x];

            if (b->active) {
                DrawRectangleRec(b->rec, color(BRICK_COLORS[b->value - 1]));
            }
        }
    }
}

void s_game_draw_hud_left(GameState* s) {
    char txt[20] = {0};
    snprintf(txt, sizeof(txt), "Score: %d/%d", s->score, maxscore);
    DrawText(txt, 20, 20, 20, TXT_PRIMARY);

    if (cfg.debug) {
        Color col = LIGHTGRAY;
        col.a = 30;
        i32 debug_txt_width = MeasureText(txt, 20);
        char debug_txt_buf[30] = {0};
        const f64 avg_speed = (double)sqrt(s->ball.xspd * s->ball.xspd +
                                           s->ball.yspd * s->ball.yspd);
        snprintf(debug_txt_buf, sizeof(debug_txt_buf),
                 "Speed: %0.4f (%0.3f,%0.3f)", avg_speed, s->ball.xspd,
                 s->ball.yspd);
        debug_txt_width = MeasureText(debug_txt_buf, 10);
        DrawRectangle(0, 38, debug_txt_width + 30, 10, col);
        DrawText(debug_txt_buf, 20, 40, 10, TXT_SECONDARY);
        debug_txt_width = MeasureText(debug_txt_buf, 10);
        snprintf(debug_txt_buf, sizeof(debug_txt_buf),
                 "Position: (%0.3f, %0.3f)", s->ball.x, s->ball.y);
        DrawRectangle(0, 48, debug_txt_width + 30, 10, col);
        DrawText(debug_txt_buf, 20, 50, 10, TXT_SECONDARY);
        debug_txt_width = MeasureText(debug_txt_buf, 10);
        snprintf(debug_txt_buf, sizeof(debug_txt_buf), "Paddle: (%0.3f, %0.3f)",
                 s->paddle.rec.x, s->paddle.rec.y);
        DrawRectangle(0, 58, debug_txt_width + 30, 12, col);
        DrawText(debug_txt_buf, 20, 60, 10, TXT_SECONDARY);
    }
}

void s_game_draw_hud_right(GameState* s) {
    char buf[10] = {0};

    const i32 BAR_WIDTH = 150;
    i32 BAR_X = WIN_WIDTH - BAR_WIDTH - 20;

    if (!s->paused) {
        BAR_X -= s->gui.quit_button.width;
        BAR_X -= 10;
    }

    const f64 FRAC_BROKEN = (f64)s->bricks_broken / (LAYERS * NUM_BRICKS);
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

void s_game_gui_draw(GameState* s) {
    if (GuiButton(s->gui.quit_button, GuiIconText(ICON_EXIT, "Quit"))) {
        s->exit_overlay = true;
    }
}

void s_game_draw(GameState* s) {
    DrawRectangleRec(s->paddle.rec, s->paddle.color);
    DrawCircle(s->ball.x, s->ball.y, BALL_RADIUS, BALL_COLOR);

    s_game_draw_bricks(s);
    s_game_draw_hud_left(s);
    s_game_draw_hud_right(s);

    if (s->paused) {
        Rectangle darken = (Rectangle){0, 0, WIN_WIDTH, WIN_HEIGHT};
        DrawRectangleRec(darken, (Color){100, 100, 100, 100});

        const char* pause = "paused";
        const i32 pause_txtsz = 60;
        i32 pause_width = MeasureText(pause, pause_txtsz);
        i32 pause_posx = (WIN_WIDTH / 2) - pause_width / 2;
        i32 pause_posy = (WIN_HEIGHT / 2) - pause_txtsz / 2;

        DrawText(pause, pause_posx, pause_posy, pause_txtsz, TXT_PRIMARY);
    }

    if (s->exit_overlay) {
        Rectangle darken = (Rectangle){0, 0, WIN_WIDTH, WIN_HEIGHT};
        DrawRectangleRec(darken, (Color){100, 100, 100, 100});

        const char* exit = "exit?";
        const i32 exit_txtsz = 60;
        i32 exit_width = MeasureText(exit, exit_txtsz);
        i32 exit_posx = (WIN_WIDTH / 2) - exit_width / 2;
        i32 exit_posy =
            (WIN_HEIGHT / 2) - 60; // exit_txtsz + padding + buttons = 120

        if (GuiButton(s->gui.exit_overlay_yes_button,
                      GuiIconText(ICON_OK_TICK, "[Y]es"))) {
            switch_screen(SCR_TITLE);
            return;
        }

        if (GuiButton(s->gui.exit_overlay_no_button,
                      GuiIconText(ICON_CROSS, "[N]o"))) {
            s->exit_overlay = false; // back
            return;
        }

        DrawText(exit, exit_posx, exit_posy, exit_txtsz, TXT_PRIMARY);
    }

    if (s->exit_overlay || s->paused)
        return;

    s_game_gui_draw(s);
}

void s_game_update_paddle(GameState* s) {
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
        if (paddle->rec.x + s->paddle_speed <= WIN_WIDTH - PADDLE_WIDTH) {
            paddle->rec.x += s->paddle_speed;
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

void s_game_update_ball(GameState* s) {
    Ball* ball = &s->ball;

    // ball update logic
    if (ball->xspd > 0) {
        if (ball->x + ball->xspd < WIN_WIDTH - BALL_RADIUS) {
            s->ball.x += s->ball.xspd;
        } else {
            s->ball.x = WIN_WIDTH - BALL_RADIUS;
            s->ball.xspd = -s->ball.xspd;

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
            s->ball.x += s->ball.xspd;
        } else {
            s->ball.x = BALL_RADIUS;
            s->ball.xspd = -s->ball.xspd;

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
            s->ball.y += s->ball.yspd;
        } else {
            s->ball.y = WIN_HEIGHT - BALL_RADIUS;
            s->ball.yspd = -s->ball.yspd;
        }
    } else if (ball->yspd < 0) {
        if (ball->y + ball->yspd > 0) {
            s->ball.y += s->ball.yspd;
        } else {
            s->ball.y = 0;
            s->ball.yspd = -s->ball.yspd;
        }
    }
}

void s_game_update_bricks(GameState* s) {
    Ball* ball = &s->ball;
    Vector2 ball_pos = (Vector2){ball->x, ball->y};

    for (usize y = 0; y < LAYERS; y++) {
        // suppress -Wsign-compare
        for (usize x = 0; x < (usize)NUM_BRICKS; x++) {
            Brick* brick = &s->bricks.data[y * NUM_BRICKS + x];

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
                s->score += brick->value;
                s->bricks_broken++;
            }
        }
    }
}

void s_game_update(GameState* s) {
    Paddle* paddle = &s->paddle;
    Ball* ball = &s->ball;

    s->elapsed_time++;

    f64 paddle_speed_offset =
        (f64)(sqrt(ball->xspd * ball->xspd + ball->yspd * ball->yspd)) / 5;
    s->paddle_speed = INITIAL_PADDLE_SPEED + paddle_speed_offset;

    if (ball->y + BALL_RADIUS > paddle->rec.y + paddle->rec.height) {
        switch_screen(SCR_DEAD);
        return;
    }

    if (s->score >= maxscore) {
        switch_screen(SCR_WIN);
        return;
    }

    if (cfg.debug) {
        if (IsKeyPressed(KEY_K)) {
            switch_screen(SCR_DEAD);
        }

        if (IsKeyPressed(KEY_W)) {
            switch_screen(SCR_WIN);
            return;
        }
    }

    if (IsKeyPressed(KEY_SPACE)) {
        s->paused = !s->paused;
    }

    if (s->paused) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            s->paused = false;
        }

        return;
    }

    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        s->exit_overlay = !s->exit_overlay;
    }

    if (s->exit_overlay) {
        if (IsKeyPressed(KEY_N)) {
            s->exit_overlay = false; // back
        } else if (IsKeyPressed(KEY_Y)) {
            switch_screen(SCR_TITLE);
            return;
        }

        return;
    }

    s_game_update_paddle(s);
    s_game_update_ball(s);
    s_game_update_bricks(s);
}

GameGui s_game_gui_new(void) {
    const i32 QUIT_BUTTON_WIDTH = 60;
    const i32 EXIT_OVERLAY_BUTTON_WIDTH = 80;
    const i32 EXIT_OVERLAY_BTNS_WIDTH =
        2 * EXIT_OVERLAY_BUTTON_WIDTH + 10; // + padding

    const i32 EXIT_OVERLAY_BTNS_Y = (WIN_HEIGHT / 2) + 30; // padding
    const i32 EXIT_OVERLAY_BUTTONS_BEGIN =
        (i32)(WIN_WIDTH / 2 - EXIT_OVERLAY_BTNS_WIDTH / 2);

    GameGui res = {
        .quit_button = (Rectangle){.x = WIN_WIDTH - 20 - QUIT_BUTTON_WIDTH,
                                   .y = 20,
                                   .width = QUIT_BUTTON_WIDTH,
                                   .height = 19},
        .exit_overlay_no_button =
            (Rectangle){.x = EXIT_OVERLAY_BUTTONS_BEGIN,
                                   .y = EXIT_OVERLAY_BTNS_Y,
                                   .width = EXIT_OVERLAY_BUTTON_WIDTH,
                                   .height = 30},
        .exit_overlay_yes_button =
            (Rectangle){.x = EXIT_OVERLAY_BUTTONS_BEGIN +
                             EXIT_OVERLAY_BUTTON_WIDTH + 10,
                                   .y = EXIT_OVERLAY_BTNS_Y,
                                   .width = EXIT_OVERLAY_BUTTON_WIDTH,
                                   .height = 30},
        .exit_overlay_title =
            title_new_centered(60, astr("exit?"), TXT_PRIMARY),
    };

    return res;
}

Screen s_game_new(void) {
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

    GameState gs = {
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
        .gui = s_game_gui_new(),
        .bricks = s_game_bricks_init(),

        // other fields are numerical, so we won't init
    };
    s_game_bricks_populate(&gs.bricks);

    Screen res = {.variant = SCR_GAME, .data.game = gs};

    return res;
}

void s_game_gui_deinit(GameState* s) {
    title_deinit(&s->gui.exit_overlay_title);
}

void s_game_deinit(GameState* s) {
    s_game_bricks_deinit(&s->bricks);
    s_game_gui_deinit(s);
}

DeadGui s_dead_gui_new(void) {
    const i32 BUTTON_WIDTH = 120;
    const i32 BUTTONS_WIDTH = 3 * BUTTON_WIDTH + 20; // + 2*padding
    const i32 BUTTONS_BEGIN = (i32)(WIN_WIDTH / 2 - BUTTONS_WIDTH / 2);

    return (DeadGui){
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
        .title = title_new_centered(90, astr("Game Over!"), TXT_PRIMARY),
    };
}

Screen s_dead_new(void) {
    Screen res = {
        .variant = SCR_DEAD,
    };

    res.data.dead = (DeadScreenState){
        .gui = s_dead_gui_new(),
    };

    return res;
}

void s_dead_gui_draw(DeadScreenState* s) {
    bool should_restart = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_R) ||
                          GuiButton(s->gui.restart_button,
                                    GuiIconText(ICON_REPEAT_FILL, "[R]estart"));
    bool should_go_titlescreen =
        IsKeyPressed(KEY_T) || IsKeyPressed(KEY_ESCAPE) ||
        GuiButton(s->gui.title_button,
                  GuiIconText(ICON_HOUSE, "[T]itle Screen"));
    bool should_quit =
        IsKeyPressed(KEY_Q) ||
        GuiButton(s->gui.quit_button, GuiIconText(ICON_EXIT, "[Q]uit"));

    if (should_restart) {
        switch_screen(SCR_GAME);
    } else if (should_go_titlescreen) {
        switch_screen(SCR_TITLE);
    } else if (should_quit) {
        switch_screen(SCR_QUIT);
    }
}

void s_dead_draw(DeadScreenState* s) {
    title_draw(&s->gui.title);
    s_dead_gui_draw(s);
}

void s_dead_update(DeadScreenState* s) {}

void s_dead_deinit(DeadScreenState* s) { title_deinit(&s->gui.title); }

WinGui s_win_gui_new(void) {
    const i32 BUTTON_WIDTH = 120;
    const i32 BUTTONS_WIDTH = 3 * BUTTON_WIDTH + 20; // + 2*padding
    const i32 BUTTONS_BEGIN = (i32)(WIN_WIDTH / 2 - BUTTONS_WIDTH / 2);
    const i32 TEXT_INPUT_WIDTH = 250;

    Title title = title_new_hcentered((u16)(WIN_HEIGHT * 0.2), 90,
                                      astr("You won!"), TXT_PRIMARY);

    WinGui res = {
        .restart_button = (Rectangle){.x = BUTTONS_BEGIN,
                                      .y = WIN_HEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30 },
        .title_button = (Rectangle){.x = BUTTONS_BEGIN + BUTTON_WIDTH + 10,
                                      .y = WIN_HEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30 },
        .quit_button = (Rectangle){.x = BUTTONS_BEGIN + 2 * BUTTON_WIDTH + 20,
                                      .y = WIN_HEIGHT - 40,
                                      .width = BUTTON_WIDTH,
                                      .height = 30 },
        .text_input = (Rectangle){.x = (i32)(TEXT_INPUT_WIDTH / 2),
                                      .y = title.y + title.size + 10,
                                      .width = TEXT_INPUT_WIDTH,
                                      .height = 250},
        .title = title
    };

    return res;
}

Screen s_win_new(void) {
    Screen res = {
        .variant = SCR_WIN,
    };

    LeaderboardEntry entry = {
        .name = a_string_with_capacity(512),
    };

    res.data.win = (WinScreenState){.gui = s_win_gui_new(), .entry = entry};

    return res;
}

void s_win_gui_draw(WinScreenState* wss) {
    // name entry
    const i32 TEXT_INPUT_WIDTH = 250;
    /* FIXME: later
    GuiTextInputBox(text_input_bounds, "Enter display name for leaderboard...",
                    "leave empty to stay anonymous", "", , 255, NULL);
    */

    if (GuiButton(wss->gui.restart_button,
                  GuiIconText(ICON_REPEAT_FILL, "[R]estart"))) {
        switch_screen(SCR_GAME);
    } else if (GuiButton(wss->gui.title_button,
                         GuiIconText(ICON_HOUSE, "[T]itle Screen"))) {
        switch_screen(SCR_TITLE);
    } else if (GuiButton(wss->gui.quit_button,
                         GuiIconText(ICON_EXIT, "[Q]uit"))) {
        switch_screen(SCR_QUIT);
    }
}

void s_win_draw(WinScreenState* s) {
    title_draw(&s->gui.title);

    s_win_gui_draw(s);
}

void s_win_update(WinScreenState* s) {}

void s_win_deinit(WinScreenState* s) {
    title_deinit(&s->gui.title);
    leaderboard_entry_deinit(&s->entry);
}
