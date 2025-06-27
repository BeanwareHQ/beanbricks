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

#include "3rdparty/include/a_string.h"
#include "common.h"

typedef enum {
    // "default"
    THEME_DEFAULT = 0,
    // "dark"
    THEME_DARK = 1,
    // "catppuccin_latte"
    THEME_CTP_LATTE = 2,
    // "catppuccin_frappe"
    THEME_CTP_FRAPPE = 3,
    // "catppuccin_mocha"
    THEME_CTP_MOCHA = 4,
    // "catppuccin_macchiato"
    THEME_CTP_MACCHIATO = 5,
    // TODO: add this
    // THEME_PINK = 6,
} Theme;

a_string theme_to_string(const Theme* t);
void theme_to_cstring(const Theme* t, char* buf, usize buf_len);

/*
 * Stores the corresponding `Theme` given an a_string into t.
 * Stores `NULL` if it is an invalid theme.
 *
 * @param astr src string
 * @param t destination
 */
void theme_from_string(const a_string* astr, Theme* dest);

/*
 * Stores the corresponding `Theme` given an C string into t.
 * Stores `NULL` if it is an invalid theme.
 *
 * @param cstr src string
 * @param t destination
 */
void theme_from_cstring(const char* cstr, Theme* dest);

bool theme_cmp_to_string(const Theme* t, const a_string* other);
bool theme_cmp_to_cstring(const Theme* t, const char* other);

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

a_string config_to_json(const Config* cfg);
Config config_from_json(const a_string* str);
Config config_from_json_cstr(const char* str);
