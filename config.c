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
#include <stdlib.h>
#define _POSIX_C_SOURCE 200809L

#include <string.h>

#include "3rdparty/include/a_string.h"
#include "3rdparty/include/cJSON.h"

#include "common.h"
#include "config.h"

#define MAX_THEME_NAME_LEN 32

static const char* THEME_TBL[] = {
    [THEME_DEFAULT] = "default",
    [THEME_DARK] = "dark",
    [THEME_CTP_LATTE] = "catppuccin_latte",
    [THEME_CTP_FRAPPE] = "catppuccin_frappe",
    [THEME_CTP_MACCHIATO] = "catppuccin_macchiato",
    [THEME_CTP_MOCHA] = "catppuccin_mocha",
};

a_string theme_to_string(const Theme* t) {
    // teknologia
    return astr(THEME_TBL[*t]);
}

void theme_to_cstring(const Theme* t, char* buf, usize buf_len) {
    if (buf_len < MAX_THEME_NAME_LEN)
        panic("expected at least %d chars of space in the buffer",
              MAX_THEME_NAME_LEN);

    const char* res = THEME_TBL[*t];
    strncpy(buf, res, buf_len);
}

void theme_from_string(const a_string* astr, Theme* dest) {
    theme_from_cstring(astr->data, dest);
}

void theme_from_cstring(const char* cstr, Theme* dest) {
    if (!strcmp(cstr, "default")) {
        *dest = THEME_DEFAULT;
    } else if (!strcmp(cstr, "dark")) {
        *dest = THEME_DARK;
    } else if (!strcmp(cstr, "catppuccin_latte")) {
        *dest = THEME_CTP_LATTE;
    } else if (!strcmp(cstr, "catppuccin_frappe")) {
        *dest = THEME_CTP_FRAPPE;
    } else if (!strcmp(cstr, "catppuccin_macchiato")) {
        *dest = THEME_CTP_MACCHIATO;
    } else if (!strcmp(cstr, "catppuccin_mocha")) {
        *dest = THEME_CTP_MOCHA;
    } else {
        dest = NULL;
    }
}

bool theme_cmp_to_string(const Theme* t, const a_string* other) {
    return theme_cmp_to_cstring(t, other->data);
}

bool theme_cmp_to_cstring(const Theme* t, const char* other) {
    return !strcmp(other, THEME_TBL[*t]);
}

a_string config_to_json(const Config* cfg) {
    char* res_str = NULL;

    cJSON* res = cJSON_CreateObject();
    if (res == NULL) {
        goto end;
    }

    char theme_buf[32];
    theme_to_cstring(&cfg->theme, theme_buf, sizeof(theme_buf));
    cJSON* theme = cJSON_CreateString(theme_buf);
    if (theme == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "theme", theme);

    cJSON* win_width = cJSON_CreateNumber(cfg->win_width);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "winWidth", win_width);

    cJSON* win_height = cJSON_CreateNumber(cfg->win_height);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "winHeight", win_height);

    cJSON* paddle_width = cJSON_CreateNumber(cfg->paddle_width);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "paddleWidth", paddle_width);

    cJSON* paddle_height = cJSON_CreateNumber(cfg->paddle_height);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "paddleHeight", paddle_height);

    cJSON* initial_paddle_speed = cJSON_CreateNumber(cfg->initial_paddle_speed);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "initialPaddleSpeed", initial_paddle_speed);

    cJSON* layers = cJSON_CreateNumber(cfg->layers);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "layers", layers);

    cJSON* brick_width = cJSON_CreateNumber(cfg->brick_width);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "brickWidth", brick_width);

    cJSON* brick_height = cJSON_CreateNumber(cfg->brick_height);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "brickHeight", brick_height);

    cJSON* ball_radius = cJSON_CreateNumber(cfg->ball_radius);
    if (win_width == NULL) {
        goto end;
    }
    cJSON_AddItemToObject(res, "ballRadius", ball_radius);

    res_str = cJSON_Print(res);

end:
    cJSON_Delete(res);
    if (res_str == NULL) {
        return a_string_new_uninitialized();
    } else {
        return astr(res_str);
    }
}

Config config_from_json(const a_string* str) {
    return config_from_json_cstr(str->data);
}

Config config_from_json_cstr(const char* str) {}
