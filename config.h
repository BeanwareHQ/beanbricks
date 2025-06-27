/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: A basic config system with cJSON.
 */

#pragma once

#include "3rdparty/include/asv.h"

typedef enum {
    THEME_DEFAULT = 0,
    THEME_DARK = 1,
    THEME_CTP_MOCHA = 2,
    THEME_CTP_MACCHIATO = 3,
    THEME_CTP_FRAPPE = 4,
    THEME_CTP_LATTE = 5,
    THEME_PINK = 6,
} Theme;

a_string theme_to_string(Theme* t);
Theme theme_from_string(const a_string* astr);
Theme theme_from_cstring(const char* cstr);
bool theme_cmp_to_string(Theme* t, const a_string* other);
bool theme_cmp_to_cstring(Theme* t, const char* other);

typedef struct {
    // Theme
    Theme theme;
    // Window Width
    u16 win_width;
    // Window Height
    u16 win_height;
    // Paddle Width
    u16 paddle_width;
    // Paddle Height
    u16 paddle_height;
    // Initial Paddle Speed
    u16 initial_paddle_speed;
    // Layers
    u16 layers;
    // Brick Width
    u16 brick_width;
    // Brick Height
    u16 brick_height;
    // Ball Radius
    u16 ball_radius;
} Config;
