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

void titlescreen_draw_gui(TitleScreenState* gui);
void titlescreen_draw(TitleScreenState* tss);
void titlescreen_update(TitleScreenState* tss);
void titlescreen_reset(TitleScreenState* tss);

#endif
