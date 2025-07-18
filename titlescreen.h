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

#ifndef _TITLESCREEN_H
#define _TITLESCREEN_H

#include "beanbricks.h"

void s_title_gui_draw(TitleScreenState* gui);
void s_title_draw(TitleScreenState* s);
void s_title_update(TitleScreenState* s);

TitleScreenGui s_title_gui_new(void);
Screen s_title_new(void);

void s_title_deinit(TitleScreenState* s);

#endif
