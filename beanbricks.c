/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <math.h>
#include <sys/cdefs.h>
#include <time.h>

#include <raylib.h>

#include "3rdparty/include/a_string.h"
#include "assets/themes.h"
#include "common.h"
#include "config.h"
#include "theme.h"

#define RAYGUI_IMPLEMENTATION
#include "3rdparty/include/raygui.h"

#define VERSION "0.2.0-pre"

#define HELP                                                                   \
    "\033[1mbeanbricks: a questionable breakout clone in C and "               \
    "raylib.\033[0m\n\n"                                                       \
    "Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.\n"                 \
    "This program and all source code in the project directory including "     \
    "this file is licensed under the MIT/Expat license; unless otherwise "     \
    "stated.\n"                                                                \
    "View the full text of the license in the root of the project, or pass "   \
    "--license."                                                               \
    "usage: beanbricks [flags]\n"                                              \
    "running the program with no args will launch the game.\n\n"               \
    "options:\n"                                                               \
    "    --help: show this help screen\n"                                      \
    "    --version: show the version of the program\n"

#define WIN_WIDTH            (cfg.win_width)
#define WIN_HEIGHT           (cfg.win_height)
#define PADDLE_WIDTH         (cfg.paddle_width)
#define PADDLE_HEIGHT        (cfg.paddle_height)
#define INITIAL_PADDLE_SPEED (cfg.initial_paddle_speed)
#define LAYERS               (cfg.layers)
#define BRICK_WIDTH          (cfg.brick_width)
#define BRICK_HEIGHT         (cfg.brick_height)
#define BALL_RADIUS          (cfg.ball_radius)

#define BG_COLOR            (color(theme->bg_color))
#define DARK_SURFACE_COLOR  (color(theme->dark_surface_color))
#define LIGHT_SURFACE_COLOR (color(theme->light_surface_color))
#define BALL_COLOR          (color(theme->ball_color))
#define TXT_PRIMARY         (color(theme->txt_primary))
#define TXT_SECONDARY       (color(theme->txt_secondary))
#define BRICK_COLORS        (theme->brick_colors)

// TODO: refactor
#define LEADERBOARD_ENTRY_HEIGHT 30  // height of one leaderboard item
#define LEADERBOARD_ENTRY_WIDTH  400 // width of one leaderboard item
#define PADDLE_DEFAULT_X         (i32)((cfg.win_width / 2) - (cfg.paddle_width / 2))
#define PADDLE_DEFAULT_Y         (i32)(cfg.win_height - 75)
#define NUM_BRICKS               (i32)(WIN_WIDTH / (BRICK_WIDTH + 20))
#define BRICK_PADDING            (i32)((WIN_WIDTH - NUM_BRICKS * (BRICK_WIDTH + 20)) / 2)
#define SPEED                    1

// color is a hex code, rgb
Color color(i32 color) {
    // copied from somewhere
    u8 r = color >> 16 & 0xFF;
    u8 g = color >> 8 & 0xFF;
    u8 b = color & 0xFF;

    return (Color){r, g, b, 0xFF};
}

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
    ThemeSpec theme;
} State;

typedef struct LeaderboardEntry {
    a_string name;
    time_t time;
    u32 score;
    u32 total_score;
    u32 rows;

    // internal use data
    bool _hovered;

    struct LeaderboardEntry* next; // owned on the heap
} LeaderboardEntry;

typedef struct {
    FILE* fp; // null if not loaded from file
    LeaderboardEntry* head;
} Leaderboard;

bool should_close = false;
static u32 maxscore;

// The global config singleton (sorry no singleton pattern. this is C.)
static Config cfg;

// The global game state (sorry rustaceans)
static State s;

// Reference to s.game
static GameState* const gs = &s.game;

// Reference to s.game.bricks
static Bricks* const bricks = &gs->bricks;

// Reference to s.title_screen
static TitleScreenState* const tss = &s.title_screen;

// Reference to s.theme
static ThemeSpec* const theme = &s.theme;

// the leaderboard
static Leaderboard lb = {0};

/**
 * Creates a new leaderboard.
 *
 * @param file the filename the leaderboard should be read from. NULL = not
 * reading from file
 * @return a new leaderboard with all entries loaded in. head may be null if the
 * file is empty.
 *
 */
Leaderboard leaderboard_new(const char* file);

/**
 * Saves a leaderboard to `fp` and closes it if `fp` is not NULL.
 *
 * @param lb the leaderboard to be closed.
 *
 */
void leaderboard_close(Leaderboard* lb);

/**
 * Destroys a leaderboard. This will not save and close the file pointer. Please
 * call `leaderboard_close()` first.
 *
 * @param lb the leaderboard to be destroyed.
 *
 */
void leaderboard_destroy(Leaderboard* lb);
void leaderboard_print(Leaderboard* lb);
void leaderboard_draw(Leaderboard* lb);
void leaderboard_update(Leaderboard* lb);
LeaderboardEntry* leaderboard_end(Leaderboard* lb);
usize leaderboard_length(Leaderboard* lb);
void leaderboard_add_entry(Leaderboard* lb, LeaderboardEntry* entry);

// ownership of the a_string will be taken
LeaderboardEntry* leaderboard_entry_new(a_string name, time_t time, u32 score,
                                        u32 total_score, u32 rows);
LeaderboardEntry* leaderboard_entry_from_line(const char* line);
void leaderboard_entry_destroy(LeaderboardEntry* e);
void leaderboard_entry_print(LeaderboardEntry* e);
void leaderboard_entry_draw(LeaderboardEntry* e, usize index, i32 y);
void leaderboard_entry_draw_tooltip(LeaderboardEntry* e);
void leaderboard_entry_update(LeaderboardEntry* e);

// get an offset for the ball when bouncing on certain surfaces.
i32 get_bounce_offset(const Ball* ball);

// Populates the bricks.
void init_bricks(void);
void make_bricks(void);
void free_bricks(void);

void load_config(void);
void load_theme(void);

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

void init(void);
void deinit(void);

Leaderboard leaderboard_new(const char* file) {
    if (file != NULL)
        panic("not implemented");

    // not written as (Leaderboard){0}; for clarity
    return (Leaderboard){
        .fp = NULL,
        .head = NULL,
    };
}

void leaderboard_destroy(Leaderboard* lb) {
    if (lb->fp != NULL)
        panic("not implemented");

    LeaderboardEntry* curr = lb->head;
    while (curr != NULL) {
        LeaderboardEntry* next = curr->next;
        leaderboard_entry_destroy(curr);
        curr = next;
    }

    *lb = (Leaderboard){0};
}

void leaderboard_print(Leaderboard* lb) {
    LeaderboardEntry* curr = lb->head;
    usize index = 0;

    while (curr != NULL) {
        eprintf("%zu | ", index);
        leaderboard_entry_print(curr);
        curr = curr->next;
        index++;
    }
}
void leaderboard_draw(Leaderboard* lb) {
    LeaderboardEntry* curr = lb->head;
    usize index = 0;
    usize y = 350;
    LeaderboardEntry* hovered = NULL;

    if (curr == NULL) {
        // leaderboard is empty

        char* txt = "leaderboard empty... play a game to get started!";
        i32 txtsz = 20;
        i32 txt_width = MeasureText(txt, txtsz);
        DrawText(txt, (i32)(WIN_WIDTH / 2 - txt_width / 2), y, txtsz,
                 TXT_SECONDARY);
        return;
    }

    while (index < 10 && curr != NULL) {
        leaderboard_entry_draw(curr, index,
                               y + index * LEADERBOARD_ENTRY_HEIGHT);
        if (curr->_hovered)
            hovered = curr;
        curr = curr->next;
        index++;
    }

    // handle tooltips
    if (hovered != NULL) {
        leaderboard_entry_draw_tooltip(hovered);
    }
}

void leaderboard_update(Leaderboard* lb) {
    LeaderboardEntry* curr = lb->head;
    usize index = 0;
    usize y = 350;
    usize x = (WIN_WIDTH / 2) - (LEADERBOARD_ENTRY_WIDTH / 2);

    Rectangle lb_entry_rec = {
        .x = x,
        .y = y,
        .width = LEADERBOARD_ENTRY_WIDTH,
        .height = LEADERBOARD_ENTRY_HEIGHT,
    };

    // this update function runs every frame, so mouse data does
    // not change in the loop.

    Vector2 mouse_pos = GetMousePosition();
    Rectangle mouse_rec = {
        .x = mouse_pos.x,
        .y = mouse_pos.y,
        .width = 1,
        .height = 1,
    };

    while (index < 10 && curr != NULL) {
        bool mouse_over_entry = CheckCollisionRecs(lb_entry_rec, mouse_rec);
        // im a lazy piece of shit
        if (mouse_over_entry && !curr->_hovered)
            curr->_hovered = true;
        else if (!mouse_over_entry && curr->_hovered)
            curr->_hovered = false;

        leaderboard_entry_update(curr);
        curr = curr->next;
        index++;
        lb_entry_rec.y = y + index * LEADERBOARD_ENTRY_HEIGHT;
    }
}

LeaderboardEntry* leaderboard_end(Leaderboard* lb) {
    if (lb->head == NULL)
        return NULL;

    LeaderboardEntry* curr = NULL;
    curr = lb->head;
    while (curr->next != NULL)
        curr = curr->next;
    return curr;
}
usize leaderboard_length(Leaderboard* lb);

void leaderboard_add_entry(Leaderboard* lb, LeaderboardEntry* entry) {

    LeaderboardEntry* end = leaderboard_end(lb);
    if (end == NULL) {
        lb->head = entry;
    } else {
        end->next = entry;
    }
}

LeaderboardEntry* leaderboard_entry_new(a_string name, time_t time, u32 score,
                                        u32 total_score, u32 rows) {
    LeaderboardEntry* res = calloc(1, sizeof(LeaderboardEntry));
    check_alloc(res);
    *res = (LeaderboardEntry){
        .name = name,
        .time = time,
        .score = score,
        .total_score = total_score,
        .rows = rows,
        .next = NULL,
        ._hovered = false,
    };

    return res;
}

LeaderboardEntry* leaderboard_entry_from_line(const char* line) {
    /*
     * The line should look something like the following:
     *
     * NAME              TIME  SCORE TOTAL_SCORE ROWS
     * "the user's name" 70    90    180         3
     *
     */

    const usize line_len = strlen(line);
    usize curr = 0;

    if (line_len < 2)
        return NULL;

    // chop off all the garbage
    while (line[curr] != '"')
        curr++;

    // find the end delim
    usize name_begin = curr + 1;
    while (line[curr] != '"')
        curr++;

    // now the end char is "
    usize name_end = curr - 1;
    usize name_len = name_end - name_begin;

    a_string name_buf = a_string_with_capacity(name_len + 1);
    a_string_copy_cstr(&name_buf, (char*)line + name_begin);

    // chop off whitespaces
    curr++; // skip past "
    while (isspace(line[curr]))
        curr++;

    usize nums_len = strlen((char*)line + curr);
    a_string nums = a_string_with_capacity(nums_len + 1);
    a_string_copy_cstr(&nums, (char*)line + curr);

    char* curr_tok = NULL;
    char* strtol_end = NULL;

    curr_tok = strtok(nums.data, " ");
    if (curr_tok == NULL)
        panic("expected time in line `%s`", line);

    // parse time
    time_t time = (time_t)strtol(curr_tok, &strtol_end, 10);
    if (*strtol_end != '\0')
        panic("couldnt parse line `%s` at position %zu", line, curr);

    curr_tok = strtok(NULL, " ");
    if (curr_tok == NULL)
        panic("expected score in line `%s`", line);
    // parse score
    u32 score = strtol(curr_tok, &strtol_end, 10);
    if (*strtol_end != '\0')
        panic("couldnt parse line `%s` at position %zu", line, curr);

    // parse total_score
    if (curr_tok == NULL)
        panic("expected total_score in line `%s`", line);
    u32 total_score = strtol(curr_tok, &strtol_end, 10);
    if (*strtol_end != '\0')
        panic("couldnt parse line `%s` at position %zu", line, curr);

    // parse rows
    if (curr_tok == NULL)
        panic("expected rows in line `%s`", line);
    u32 rows = strtol(curr_tok, &strtol_end, 10);
    if (*strtol_end != '\0')
        panic("couldnt parse line `%s` at position %zu", line, curr);

    LeaderboardEntry* res =
        leaderboard_entry_new(name_buf, time, score, total_score, rows);

    a_string_free(&nums);
    a_string_free(&name_buf);

    return res;
}

void leaderboard_entry_destroy(LeaderboardEntry* e) {
    a_string_free(&e->name);
    free(e);
}

void leaderboard_entry_print(LeaderboardEntry* e) {
    eprintf("name: `%s`, time: %lu, score: %d, total_score: %d, rows: %d\n",
            e->name.data, e->time, e->score, e->total_score, e->rows);
}

void leaderboard_entry_draw(LeaderboardEntry* e, usize index, i32 y) {
    const i32 BEGIN = WIN_WIDTH / 2 - LEADERBOARD_ENTRY_WIDTH / 2;
    const i32 RANKING_BOX_WIDTH = MeasureText("00", 20) + 10; // +padding

    const Rectangle box = {
        .x = BEGIN,
        .y = y,
        .width = LEADERBOARD_ENTRY_WIDTH,
        .height = LEADERBOARD_ENTRY_HEIGHT,
    };

    const Rectangle ranking_rec = {
        .x = BEGIN,
        .y = y,
        .width = RANKING_BOX_WIDTH,
        .height = LEADERBOARD_ENTRY_HEIGHT, // 10 + 20 + 10
    };

    char buf[5] = {0};
    snprintf(buf, sizeof(buf), "%02zu", index + 1);
    const i32 RANKING_TEXT_WIDTH = MeasureText(buf, 20);

    DrawRectangleRec(box, DARK_SURFACE_COLOR);
    DrawRectangleRec(ranking_rec, LIGHT_SURFACE_COLOR);

    DrawText(buf, BEGIN + (RANKING_BOX_WIDTH / 2 - RANKING_TEXT_WIDTH / 2),
             y + 5, 20, TXT_PRIMARY);

    DrawText(e->name.data, BEGIN + RANKING_BOX_WIDTH + 10, y + 5, 20,
             TXT_SECONDARY);
}

void leaderboard_entry_draw_tooltip(LeaderboardEntry* e) {
    Vector2 mouse_pos = GetMousePosition();

    // rows:
    // -----
    //
    // name: <name>
    // score: <score>/<total_score>
    // time: <time>
    // rows: <rows>

    char rows[4][50] = {0};
    usize max_row_len = sizeof(rows[0]);

    snprintf(rows[0], max_row_len, "name: %s", e->name.data);
    snprintf(rows[1], max_row_len, "score: %d/%d", e->score, e->total_score);
    snprintf(rows[2], max_row_len, "time: %d", (i32)e->time);
    snprintf(rows[3], max_row_len, "rows: %d", e->rows);

    // draw relative to the mouse position
    i32 txt_x = mouse_pos.x + 7; // border + padding
    i32 txt_y = mouse_pos.y + 7;

    i32 max_width = MeasureText(rows[0], 20);
    for (usize i = 0; i < LENGTH(rows); i++) {
        i32 width = MeasureText(rows[i], 20);

        if (width > max_width) {
            max_width = width;
        }
    }

    Rectangle bounds = {
        .x = mouse_pos.x,
        .y = mouse_pos.y,
        .width = max_width + 14,                                // + 2*padding
        .height = (txt_y + 35 + 25 * LENGTH(rows)) - txt_y + 7, // padding
    };

    Rectangle contents = bounds;
    contents.x += 2;
    contents.y += 2;
    contents.width -= 4;
    contents.height -= 4;

    DrawRectangleRec(bounds, color(BRICK_COLORS[5]));
    DrawRectangleRec(contents, DARK_SURFACE_COLOR);

    DrawText("Stats", txt_x, txt_y, 30, TXT_PRIMARY);
    txt_y += 35;

    for (usize i = 0; i < LENGTH(rows); i++) {
        DrawText(rows[i], txt_x, txt_y, 20, TXT_SECONDARY);
        txt_y += 25;
    }
}

void leaderboard_entry_update(LeaderboardEntry* e) { return; }

// game related functions

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

    for (usize layer = 0; layer < LAYERS; layer++) {
        for (usize i = 0; i < NUM_BRICKS; i++) {
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

void load_config(void) {
    // TODO: dynamic config loading
    cfg = DEFAULT_CONFIG;
}

void load_theme(void) {
    // TODO: dynamic theme application
    s.theme = THEMESPEC_TBL[cfg.theme];
    switch (theme->theme) {
        case THEME_CTP_LATTE: {
            GuiLoadStyleCatppuccinLatteSapphire();
        } break;
        case THEME_CTP_FRAPPE: {
            GuiLoadStyleCatppuccinFrappeSapphire();
        } break;
        case THEME_CTP_MACCHIATO: {
            GuiLoadStyleCatppuccinMacchiatoSapphire();
        } break;
        case THEME_CTP_MOCHA: {
            GuiLoadStyleCatppuccinMochaMauve();
        } break;
        default: {
            // nothing for you
        } break;
    }
}

void draw_game_bricks(void) {
    for (usize y = 0; y < LAYERS; y++) {
        for (usize x = 0; x < NUM_BRICKS; x++) {
            Brick* b = &bricks->data[NUM_BRICKS * y + x];

            if (b->active) {
                DrawRectangleRec(b->rec, color(BRICK_COLORS[b->value]));
            }
        }
    }
}

void draw_game_hud_left(void) {
    char txt[20] = {0};
    snprintf(txt, sizeof(txt), "Score: %d/%d", gs->score, maxscore);
    DrawText(txt, 20, 20, 20, TXT_PRIMARY);

#ifdef DEBUG_INFO
    const i32 txt_width = MeasureText(txt, 20);
    char spd[30] = {0};
    const f64 avg_speed = (double)sqrt(gs->ball.xspd * gs->ball.xspd +
                                       gs->ball.yspd * gs->ball.yspd);
    snprintf(spd, sizeof(spd), "Speed: %0.4f (%0.3f,%0.3f)", avg_speed,
             gs->ball.xspd, gs->ball.yspd);

    DrawText(spd, 20 + txt_width + 10, 20, 20, TXT_SECONDARY);
#endif
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

void draw_titlescreen(void) {
    const char* title = "Beanbricks";
    i32 title_txtsz;

    if (tss->title_anim_stage == 0) {
        title_txtsz = 100;
    } else {
        title_txtsz = 100 + (i32)(tss->title_anim_stage / 5);
    }

    i32 title_width = MeasureText(title, title_txtsz);
    i32 title_posx = (WIN_WIDTH / 2) - title_width / 2;
    i32 title_posy = WIN_HEIGHT * 0.16;

    const char* begin = "or press enter to begin";
    const i32 begin_txtsz = 20;
    i32 begin_width = MeasureText(begin, begin_txtsz);
    i32 begin_posx = (WIN_WIDTH / 2) - begin_width / 2;
    i32 begin_posy = WIN_HEIGHT - begin_txtsz - 20;

    DrawText(title, title_posx, title_posy, title_txtsz, TXT_PRIMARY);
    DrawText(begin, begin_posx, begin_posy, begin_txtsz, TXT_SECONDARY);

    DrawText("Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025", 20, 20,
             10, TXT_SECONDARY);
    DrawText("version " VERSION, 20, 34, 10, TXT_SECONDARY);

    draw_titlescreen_gui();
    leaderboard_draw(&lb);
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
        for (usize x = 0; x < NUM_BRICKS; x++) {
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

    leaderboard_update(&lb);
}

void update_settings(void) { update_dead(); }

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
    i32 xspd;
    i32 yspd;

    // TODO: fix disgusting code (add difficulty levels)
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

void reset_titlescreen(void) {
    *tss = (TitleScreenState){
        .title_anim_stage = 1,
        .title_anim_growing = true,
    };

    const i32 BUTTON_WIDTH = 120;
    const i32 BUTTONS_WIDTH = 2 * BUTTON_WIDTH + 10; // + padding
    const i32 BUTTONS_BEGIN = (i32)(WIN_WIDTH / 2 - BUTTONS_WIDTH / 2);

    const i32 TITLESCREEN_TEXT_Y = WIN_HEIGHT * 0.16; // check draw_titlescreen

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

void handle_args(i32 argc, char* argv[argc]) {
    argc--;
    argv++;

    if (argc > 0) {
        if (!strcmp(argv[0], "--version")) {
            printf("beanbricks version " VERSION "\n");
            exit(EXIT_SUCCESS);
        } else if (!strcmp(argv[0], "--help")) {
            printf(HELP);
            exit(EXIT_SUCCESS);
        }
    }
}

void init(void) {
    load_config();
    load_theme();

    // TEST DATA (will replace later)
    lb = leaderboard_new(NULL);

    InitWindow(WIN_WIDTH, WIN_HEIGHT, "beanbricks");
    SetTargetFPS((i32)(60 / SPEED));
    srand(time(NULL));
    SetExitKey(KEY_NULL);

    for (usize i = 1; i <= LAYERS; i++) {
        maxscore += NUM_BRICKS * i;
    }

    reset_all();
}

void deinit(void) {
    free_bricks();
    leaderboard_destroy(&lb);
    CloseWindow();
}

i32 main(i32 argc, char* argv[argc]) {
    handle_args(argc, argv);
    init();

    while (!should_close) {
        if (WindowShouldClose() || should_close)
            should_close = true;

        update();
        BeginDrawing();
        ClearBackground(BG_COLOR);
        draw();
        EndDrawing();
    }

    deinit();
    return 0;
}
