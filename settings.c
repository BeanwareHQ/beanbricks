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

#include "settings.h"
#include <raylib.h>

#include "beanbricks.h"
#include "title.h"

void s_settings_gui_draw(SettingsState* s) { title_draw(&s->gui.title); }

void s_settings_draw(SettingsState* s) { s_settings_gui_draw(s); }

void s_settings_update(SettingsState* s) {
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        switch_screen(SCR_TITLE);
    }
}

SettingsGui s_settings_gui_new(void) {
    return (SettingsGui){.title = title_new_centered(
                             20, astr("nothing to see here..."), TXT_PRIMARY)};
}

Screen s_settings_new(void) {
    SettingsState s = (SettingsState){
        .gui = s_settings_gui_new(),
    };

    Screen res = {
        .variant = SCR_SETTINGS,
        .data.settings = s,
    };

    return res;
}

void s_settings_deinit(SettingsState* s) { title_deinit(&s->gui.title); }
