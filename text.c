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
#include "3rdparty/include/a_string.h"
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "beanbricks.h"
#include "common.h"
#include "text.h"
#include <raylib.h>

Text text_new(u16 x, u16 y, u16 size, a_string str, Color col) {
    if (!a_string_valid(&str)) {
        panic("passed invalid string to text constructor");
    }

    Text res = {x, y, size, .str = str, .col = col};
    res.width = MeasureText(str.data, size);

    return res;
}

Text text_new_hcentered(u16 y, u16 size, a_string str, Color col) {
    if (!a_string_valid(&str)) {
        panic("passed invalid string to text constructor");
    }

    Text res = {.y = y, .size = size, .str = str, .col = col};
    res.width = MeasureText(str.data, size);
    res.x = (WIN_WIDTH / 2) - res.width / 2;

    return res;
}

Text text_new_vcentered(u16 x, u16 size, a_string str, Color col) {
    if (!a_string_valid(&str)) {
        panic("passed invalid string to text constructor");
    }

    Text res = {.x = x, .size = size, .str = str, .col = col};
    res.width = MeasureText(str.data, size);
    res.y = (WIN_HEIGHT / 2) - size / 2;

    return res;
}

Text text_new_centered(u16 size, a_string str, Color col) {
    if (!a_string_valid(&str)) {
        panic("passed invalid string to text constructor");
    }

    Text res = {.size = size, .str = str, .col = col};
    res.width = MeasureText(str.data, size);
    res.x = (WIN_WIDTH / 2) - res.width / 2;
    res.y = (WIN_HEIGHT / 2) - size / 2;

    return res;
}

void text_draw(Text* txt) {
    DrawText(txt->str.data, txt->x, txt->y, txt->size, txt->col);
}

void text_free(Text* txt) { a_string_free(&txt->str); }
