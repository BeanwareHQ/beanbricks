/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 */
#define _POSIX_C_SOURCE 200809L
#define RAYGUI_IMPLEMENTATION

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/cdefs.h>
#include <time.h>

#include <raylib.h>

#include "3rdparty/include/a_string.h"
#include "3rdparty/include/raygui.h"

#include "beanbricks.h"
#include "common.h"
#include "theme.h"
#include "titlescreen.h"

#include "config.h"
#include "game.h"
#include "leaderboard.h"
#include "settings.h"

#include "assets/themes.h"

// The global config singleton (sorry no singleton pattern. this is C.)
Config cfg;

// The global game state
State state;

// aliases for ease-of-life
static Screen* screen;

void load_config(void);
void load_theme(void);
void draw(void);
void update(void);
void reset_all(void);
void init(void);
void deinit(void);

// color is a hex code, rgb
Color color(i32 color) {
    // copied from somewhere
    u8 r = color >> 16 & 0xFF;
    u8 g = color >> 8 & 0xFF;
    u8 b = color & 0xFF;

    return (Color){r, g, b, 0xFF};
}

static void deinit_current_screen(void) {
    switch (screen->variant) {
        case SCR_GAME: {
            s_game_deinit(&screen->data.game);
        } break;
        case SCR_DEAD: {
            s_dead_deinit(&screen->data.dead);
        } break;
        case SCR_WIN: {
            s_win_deinit(&screen->data.win);
        } break;
        case SCR_TITLE: {
            s_title_deinit(&screen->data.title);
        } break;
        case SCR_SETTINGS: {
            s_settings_deinit(&screen->data.settings);
        } break;
        default:
            break;
    }
}

void switch_screen(ScreenVariant scr) {
    deinit_current_screen();
    switch (scr) {
        case SCR_GAME: {
            *screen = s_game_new();
        } break;
        case SCR_DEAD: {
            *screen = s_dead_new();
        } break;
        case SCR_WIN: {
            *screen = s_win_new();
        } break;
        case SCR_TITLE: {
            *screen = s_title_new();
        } break;
        case SCR_SETTINGS: {
            *screen = s_settings_new();
        } break;
        case SCR_QUIT: {
            screen->variant = SCR_QUIT;
        } break;
        default:
            break;
    }
}

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
    state.theme = THEMESPEC_TBL[cfg.theme];
    // FIXME: refactor into function pointer table
    switch (state.theme.theme) {
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
    switch (screen->variant) {
        case SCR_GAME: {
            s_game_draw(&screen->data.game);
        } break;
        case SCR_DEAD: {
            s_dead_draw(&screen->data.dead);
        } break;
        case SCR_WIN: {
            s_win_draw(&screen->data.win);
        } break;
        case SCR_TITLE: {
            s_title_draw(&screen->data.title);
        } break;
        case SCR_SETTINGS: {
            s_settings_draw(&screen->data.settings);
        } break;
        default:
            break;
    }
}

void update(void) {
    // FIXME: refactor into function pointer table
    switch (screen->variant) {
        case SCR_GAME: {
            s_game_update(&screen->data.game);
        } break;
        case SCR_DEAD: {
            s_dead_update(&screen->data.dead);
        } break;
        case SCR_WIN: {
            s_win_update(&screen->data.win);
        } break;
        case SCR_TITLE: {
            s_title_update(&screen->data.title);
        } break;
        case SCR_SETTINGS: {
            s_settings_update(&screen->data.settings);
        } break;
        default:
            break;
    }
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
    screen = &state.screen;

    // actual setup
    SetTraceLogLevel(LOG_ERROR);
    info("initializing beanbricks version " VERSION);

    load_config();

    info("initializing raylib window (%dx%d)", cfg.win_width, cfg.win_height);
    InitWindow(WIN_WIDTH, WIN_HEIGHT, "beanbricks");
    SetTargetFPS((i32)(60 / SPEED));
    srand(time(NULL));
    SetExitKey(KEY_NULL);

    state = (State){
        .screen.variant = SCR_NONE,
    };

    for (usize i = 1; i <= LAYERS; i++) {
        state.maxscore += NUM_BRICKS * i;
    }

    // TEST DATA (will replace later)
    state.lb = leaderboard_new(NULL);
    load_theme();

    srand(time(NULL));
    switch_screen(SCR_TITLE);
}

void deinit(void) {
    leaderboard_deinit(&state.lb);
    CloseWindow();
    info("goodbye");
}

i32 main(i32 argc, char* argv[argc]) {
    handle_args(argc, argv);
    init();

    while (!WindowShouldClose()) {
        update();

        if (screen->variant == SCR_QUIT)
            break;

        BeginDrawing();
        ClearBackground(BG_COLOR);
        draw();
        EndDrawing();
    }

    deinit();
    return 0;
}
