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

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "beanbricks.h"

void s_settings_gui_draw(SettingsState* s);
void s_settings_draw(SettingsState* s);
void s_settings_update(SettingsState* s);

SettingsGui s_settings_gui_new(void);
Screen s_settings_new(void);
void s_settings_deinit(SettingsState* s);

#endif
