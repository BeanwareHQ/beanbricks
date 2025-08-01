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

#ifndef _CONFIG_H
#define _CONFIG_H

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
 * Returns the corresponding `Theme` given an a_string.
 * Returns -1 upon error.
 *
 * @param astr src string
 * @param t destination
 */
Theme theme_from_string(const a_string* astr);

/*
 * Returns the corresponding `Theme` given a C string.
 * Returns -1 upon error.
 *
 * @param astr src string
 * @param t destination
 */
Theme theme_from_cstring(const char* cstr);

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
    // Enable/Disable Debugging
    bool debug;
} Config;

a_string config_to_json(const Config* cfg);
Config config_from_json(const a_string* str);
Config config_from_json_cstr(const char* str);

#define DEFAULT_CONFIG                                                         \
    ((Config){                                                                 \
        .theme = THEME_CTP_MACCHIATO,                                          \
        .win_width = 600,                                                      \
        .win_height = 800,                                                     \
        .paddle_width = 100,                                                   \
        .paddle_height = 20,                                                   \
        .initial_paddle_speed = 6,                                             \
        .layers = 5,                                                           \
        .brick_width = 50,                                                     \
        .brick_height = 12,                                                    \
        .ball_radius = 10,                                                     \
        .debug = false,                                                        \
    })

#endif
