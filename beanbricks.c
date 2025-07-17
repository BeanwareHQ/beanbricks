/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 */
#include "titlescreen.h"
#define _POSIX_C_SOURCE 200809L
#define RAYGUI_IMPLEMENTATION

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <sys/cdefs.h>
#include <time.h>

#include <raylib.h>

#include "3rdparty/include/a_string.h"
#include "3rdparty/include/raygui.h"

#include "beanbricks.h"
#include "common.h"
#include "theme.h"

#include "config.h"
#include "game.h"
#include "leaderboard.h"
#include "settings.h"

#include "assets/themes.h"

// color is a hex code, rgb
Color color(i32 color) {
    // copied from somewhere
    u8 r = color >> 16 & 0xFF;
    u8 g = color >> 8 & 0xFF;
    u8 b = color & 0xFF;

    return (Color){r, g, b, 0xFF};
}

// FIXME: refactor into game state instead
u32 maxscore;

// The global config singleton (sorry no singleton pattern. this is C.)
Config cfg;

// The global theme spec
ThemeSpec theme;

// The global game state (sorry rustaceans)
State s;

/* TCC DOES NOT LIKE DECLARATIONS HERE*/
// Reference to s.game
GameState* gs;

// Reference to s.game.bricks
Bricks* bricks;

// Reference to s.title_screen
TitleScreenState* tss;

// the leaderboard
Leaderboard lb;

void load_config(void);
void load_theme(void);
void draw(void);
void update(void);
void reset_all(void);
void init(void);
void deinit(void);

// game related functions

void load_config(void) {
    // FIXME: shitcode

    const char* filepath = "./config.json";
    a_string contents = a_string_read_file(filepath);
    cfg = config_from_json(&contents);
    a_string_free(&contents);

    info("loaded configuration at \"%s\"", filepath);
}

void load_theme(void) {
    // TODO: dynamic theme application
    theme = THEMESPEC_TBL[cfg.theme];

    switch (theme.theme) {
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
        default:
            break;
    }
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
        } else if (!strcmp(argv[0], "--license")) {
            printf(LICENSE);
            exit(EXIT_SUCCESS);
        } else {
            puts("no valid arguments");
            exit(EXIT_SUCCESS);
        }
    }
}

void init(void) {
    // set up globals
    gs = &s.game;
    bricks = &gs->bricks;
    tss = &s.title_screen;

    // actual setup
    SetTraceLogLevel(LOG_ERROR);
    info("initializing beanbricks version " VERSION);

    load_config();

    // TEST DATA (will replace later)
    lb = leaderboard_new(NULL);

    info("initializing raylib window (%dx%d)", cfg.win_width, cfg.win_height);
    InitWindow(WIN_WIDTH, WIN_HEIGHT, "beanbricks");
    SetTargetFPS((i32)(60 / SPEED));
    srand(time(NULL));
    SetExitKey(KEY_NULL);

    for (usize i = 1; i <= LAYERS; i++) {
        maxscore += NUM_BRICKS * i;
    }

    load_theme();

    reset_all();
}

void deinit(void) {
    free_bricks();
    leaderboard_destroy(&lb);
    CloseWindow();
    info("goodbye");
}

i32 main(i32 argc, char* argv[argc]) {
    handle_args(argc, argv);
    init();

    while (!s.should_close) {
        if (WindowShouldClose() || s.should_close)
            s.should_close = true;

        update();
        BeginDrawing();
        ClearBackground(BG_COLOR);
        draw();
        EndDrawing();
    }

    deinit();
    return 0;
}
