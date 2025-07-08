/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: settings screen
 */

#include <raylib.h>

#include "beanbricks.h"
#include "common.h"
#include "titlescreen.h"

void draw_settings(void) {
    const char* txt = "nothing to see here...";
    const i32 txt_width = MeasureText(txt, 20);
    const i32 txt_x = WIN_WIDTH / 2 - txt_width / 2;
    const i32 txt_y = WIN_HEIGHT / 2 - 10;
    DrawText(txt, txt_x, txt_y, 20, TXT_PRIMARY);
}

void update_settings(void) {
    if (IsKeyPressed(KEY_Q)) {
        reset_titlescreen();
        s.screen = SCR_TITLE;
    }
}
