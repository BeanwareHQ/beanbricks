/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: theme definitions
 */

// pragma doesnt seem to work?
#ifndef _THEME_H
#define _THEME_H

#include "common.h"
#include "config.h"

// TODO: better naming?
typedef struct {
    Theme theme;
    i32 bg_color;
    i32 dark_surface_color;
    i32 light_surface_color;
    i32 ball_color;
    i32 txt_primary;
    i32 txt_secondary;
    i32 brick_colors[7];
} ThemeSpec;

const ThemeSpec THEMESPEC_DEFAULT = {
    .theme = THEME_DEFAULT,
    .bg_color = 0xf5f5f5,
    .dark_surface_color = 0xc3c3c3,
    .light_surface_color = 0xa0a0a0,
    .ball_color = 0x828282,
    .txt_primary = 0x101010,
    .txt_secondary = 0x525252,
    .brick_colors =
        {
                       [0] = 0xe62937,
                       [1] = 0xffa100,
                       [2] = 0xffcb00,
                       [3] = 0x00e4e0,
                       [4] = 0x0079f1,
                       [5] = 0xc87aff,
                       [6] = 0x873cbe,
                       },
};

const ThemeSpec THEMESPEC_DARK = {
    .theme = THEME_DEFAULT,
    .bg_color = 0x1d1d1e,
    .dark_surface_color = 0x353539,
    .light_surface_color = 0x404042,
    .ball_color = 0xc8c8c8,
    .txt_primary = 0xf8f8f8,
    .txt_secondary = 0xf2f2f2,
    .brick_colors = {
                     [0] = 0xe62937,
                     [1] = 0xffa100,
                     [2] = 0xffcb00,
                     [3] = 0x00e4e0,
                     [4] = 0x0079f1,
                     [5] = 0xc87aff,
                     [6] = 0x873cbe,
                     }
};

const ThemeSpec THEMESPEC_CTP_LATTE = {
    .theme = THEME_CTP_LATTE,
    .bg_color = 0xeff1f5,
    .dark_surface_color = 0xccd0da,
    .light_surface_color = 0xbcc0cc,
    .ball_color = 0x6c6f85,
    .txt_primary = 0x4c4f69,
    .txt_secondary = 0x5c5f77,
    .brick_colors = {
                     [0] = 0xd20f39,
                     [1] = 0xfe640b,
                     [2] = 0xdf8e1d,
                     [3] = 0x40a02b,
                     [4] = 0x04a5e5,
                     [5] = 0x1e66f5,
                     [6] = 0x7287fd,
                     }
};

const ThemeSpec THEMESPEC_CTP_FRAPPE = {
    .theme = THEME_CTP_FRAPPE,
    .bg_color = 0x303446,
    .dark_surface_color = 0x414559,
    .light_surface_color = 0x51576d,
    .ball_color = 0x949cbb,
    .txt_primary = 0xc6d0f5,
    .txt_secondary = 0xb5bfe2,
    .brick_colors = {[0] = 0xe78284,
                     [1] = 0xef9f76,
                     [2] = 0xe5c890,
                     [3] = 0xa6d189,
                     [4] = 0x85c1dc,
                     [5] = 0x8caaee,
                     [6] = 0xbabbf1}
};

const ThemeSpec THEMESPEC_CTP_MACCHIATO = {
    .theme = THEME_CTP_MACCHIATO,
    .bg_color = 0x24273a,
    .dark_surface_color = 0x363a4f,
    .light_surface_color = 0x494d64,
    .ball_color = 0x939ab7,
    .txt_primary = 0xcad3f5,
    .txt_secondary = 0xb8c0e0,
    .brick_colors =
        {
                       [0] = 0xed8796,
                       [1] = 0xf5a97f,
                       [2] = 0xeed49f,
                       [3] = 0xa6da95,
                       [4] = 0x7dc4e4,
                       [5] = 0x8aadf4,
                       [6] = 0xb7bdf8,
                       },
};

const ThemeSpec THEMESPEC_CTP_MOCHA = {
    .theme = THEME_CTP_MOCHA,
    .bg_color = 0x1e1e2e,
    .dark_surface_color = 0x313244,
    .light_surface_color = 0x45475a,
    .ball_color = 0x9399b2,
    .txt_primary = 0xcdd6f4,
    .txt_secondary = 0xbac2de,
    .brick_colors = {
                     [0] = 0xf38ba8,
                     [1] = 0xfab387,
                     [2] = 0xf9e2af,
                     [3] = 0xa6e3a1,
                     [4] = 0x74c7ec,
                     [5] = 0x89b4fa,
                     [6] = 0xb4befe,
                     }
};

const ThemeSpec THEMESPEC_TBL[] = {
    [THEME_DEFAULT] = THEMESPEC_DEFAULT,
    [THEME_DARK] = THEMESPEC_DARK,
    [THEME_CTP_LATTE] = THEMESPEC_CTP_LATTE,
    [THEME_CTP_FRAPPE] = THEMESPEC_CTP_FRAPPE,
    [THEME_CTP_MACCHIATO] = THEMESPEC_CTP_MACCHIATO,
    [THEME_CTP_MOCHA] = THEMESPEC_CTP_MOCHA,
};
#endif
