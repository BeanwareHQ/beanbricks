/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: wrapper for raylib text
 */

#ifndef _TEXT_H
#define _TEXT_H

#include <raylib.h>

#include "3rdparty/include/a_string.h"
#include "common.h"

typedef struct {
    u16 x;
    u16 y;
    u16 size;
    u16 width; // accessible only after initialization
    a_string str;
    Color col;
} Title;

Title title_new(u16 x, u16 y, u16 size, a_string str, Color col);
Title title_new_hcentered(u16 y, u16 size, a_string str, Color col);
Title title_new_vcentered(u16 x, u16 size, a_string str, Color col);
Title title_new_centered(u16 size, a_string str, Color col);

void title_draw(Title* txt);
void title_deinit(Title* txt);

#endif
