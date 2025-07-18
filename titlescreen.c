/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: titlescreen logic
 */
#define _POSIX_C_SOURCE 200809L

#include <raylib.h>

#include "3rdparty/include/raygui.h"

#include "beanbricks.h"
#include "common.h"
#include "game.h"
#include "titlescreen.h"

void titlescreen_draw(TitleScreenState* tss) {
    const char* title = "Beanbricks";
    i32 title_txtsz;

    if (tss->title_anim_stage == 0) {
        title_txtsz = 90;
    } else {
        title_txtsz = 90 + (i32)(tss->title_anim_stage / 5);
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

    titlescreen_draw_gui(tss);
    leaderboard_draw(&lb);
}

void titlescreen_draw_gui(TitleScreenState* tss) {
    if (GuiButton(tss->gui.start_button,
                  GuiIconText(ICON_PLAYER_PLAY, "[P]lay"))) {
        game_reset();
        s.screen = SCR_GAME;
    }

    if (GuiButton(tss->gui.quit_button, GuiIconText(ICON_EXIT, "[Q]uit"))) {
        s.should_close = true;
        return;
    }

    if (GuiButton(tss->gui.settings_button,
                  GuiIconText(ICON_GEAR, "[S]ettings"))) {
        s.screen = SCR_SETTINGS;
    }
}

void titlescreen_update(TitleScreenState* tss) {
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        s.should_close = true;
        return;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_P)) {
        game_reset();
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

void titlescreen_reset(TitleScreenState* tss) {
    *tss = (TitleScreenState){
        .title_anim_stage = 1,
        .title_anim_growing = true,
    };

    const i32 NBUTTONS = 3;
    const i32 BUTTON_WIDTH = 120;
    const i32 BUTTONS_WIDTH =
        (NBUTTONS - 1) * BUTTON_WIDTH + (NBUTTONS - 1) * 10; // + padding
    const i32 BUTTONS_BEGIN =
        (i32)(WIN_WIDTH / NBUTTONS - BUTTONS_WIDTH / NBUTTONS);

    const i32 TITLESCREEN_TEXT_Y = WIN_HEIGHT * 0.16; // check draw_titlescreen

    tss->gui = (TitleScreenGui){
        .start_button =
            (Rectangle){
                        .x = BUTTONS_BEGIN,
                        .y = TITLESCREEN_TEXT_Y + 140,
                        .width = BUTTON_WIDTH,
                        .height = 30,
                        },
        .quit_button =
            (Rectangle){
                        .x = BUTTONS_BEGIN + BUTTON_WIDTH + 10,
                        .y = TITLESCREEN_TEXT_Y + 140,
                        .width = BUTTON_WIDTH,
                        .height = 30,
                        },
        .settings_button = (Rectangle){
                        .x = BUTTONS_BEGIN + 2 * BUTTON_WIDTH + 20,
                        .y = TITLESCREEN_TEXT_Y + 140,
                        .width = BUTTON_WIDTH,
                        .height = 30,
                        }
    };
}
