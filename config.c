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

// TODO: better errors

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

Theme theme_from_string(const a_string* astr) {
    return theme_from_cstring(astr->data);
}

Theme theme_from_cstring(const char* cstr) {
    if (!strcmp(cstr, "default")) {
        return THEME_DEFAULT;
    } else if (!strcmp(cstr, "dark")) {
        return THEME_DARK;
    } else if (!strcmp(cstr, "catppuccin_latte")) {
        return THEME_CTP_LATTE;
    } else if (!strcmp(cstr, "catppuccin_frappe")) {
        return THEME_CTP_FRAPPE;
    } else if (!strcmp(cstr, "catppuccin_macchiato")) {
        return THEME_CTP_MACCHIATO;
    } else if (!strcmp(cstr, "catppuccin_mocha")) {
        return THEME_CTP_MOCHA;
    } else {
        return -1;
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
        panic("creating JSON object from config failed.");
    } else {
        a_string res = astr(res_str);
        free(res_str);
        return res;
    }
}

Config config_from_json(const a_string* str) {
    return config_from_json_cstr(str->data);
}

Config config_from_json_cstr(const char* str) {
    Config res = {0};
    const char* err;

    cJSON* json = cJSON_Parse(str);
    if (json == NULL) {
        err = cJSON_GetErrorPtr();
        panic("parsing JSON into Config failed: %s", err);
    }

    cJSON* theme_cjson = cJSON_GetObjectItemCaseSensitive(json, "theme");
    if (!(cJSON_IsString(theme_cjson) && theme_cjson->valuestring != NULL)) {
        panic("expected type string for field \"theme\", got something else");
    }
    Theme theme = theme_from_cstring(theme_cjson->valuestring);
    if (theme == -1) {
        panic("invalid theme name %s", theme_cjson->valuestring);
    }
    res.theme = theme;

    cJSON* win_width_cjson = cJSON_GetObjectItemCaseSensitive(json, "winWidth");
    if (!cJSON_IsNumber(win_width_cjson)) {
        panic(
            "expected type number for field \"win_width\", got something else");
    }
    res.win_width = (u16)win_width_cjson->valueint;

    cJSON* win_height_cjson =
        cJSON_GetObjectItemCaseSensitive(json, "winHeight");
    if (!cJSON_IsNumber(win_height_cjson)) {
        panic("expected type number for field \"win_height\", got something "
              "else");
    }
    res.win_height = (u16)win_height_cjson->valueint;

    cJSON* paddle_width_cjson =
        cJSON_GetObjectItemCaseSensitive(json, "paddleWidth");
    if (!cJSON_IsNumber(paddle_width_cjson)) {
        panic("expected type number for field \"paddle_width\", got something "
              "else");
    }
    res.paddle_width = (u16)paddle_width_cjson->valueint;

    cJSON* paddle_height_cjson =
        cJSON_GetObjectItemCaseSensitive(json, "paddleHeight");
    if (!cJSON_IsNumber(paddle_height_cjson)) {
        panic("expected type number for field \"paddle_height\", got something "
              "else");
    }
    res.paddle_height = (u16)paddle_height_cjson->valueint;

    cJSON* initial_paddle_speed_cjson =
        cJSON_GetObjectItemCaseSensitive(json, "initialPaddleSpeed");
    if (!cJSON_IsNumber(initial_paddle_speed_cjson)) {
        panic("expected type number for field \"initial_paddle_speed\", got "
              "something else");
    }
    res.initial_paddle_speed = (u16)initial_paddle_speed_cjson->valueint;

    cJSON* layers_cjson = cJSON_GetObjectItemCaseSensitive(json, "layers");
    if (!cJSON_IsNumber(layers_cjson)) {
        panic("expected type number for field \"layers\", got something else");
    }
    res.layers = (u16)layers_cjson->valueint;

    cJSON* brick_width_cjson =
        cJSON_GetObjectItemCaseSensitive(json, "brickWidth");
    if (!cJSON_IsNumber(brick_width_cjson)) {
        panic("expected type number for field \"brick_width\", got something "
              "else");
    }
    res.brick_width = (u16)brick_width_cjson->valueint;

    cJSON* brick_height_cjson =
        cJSON_GetObjectItemCaseSensitive(json, "brickHeight");
    if (!cJSON_IsNumber(brick_height_cjson)) {
        panic("expected type number for field \"brick_height\", got something "
              "else");
    }
    res.brick_height = (u16)brick_height_cjson->valueint;

    cJSON* ball_radius_cjson =
        cJSON_GetObjectItemCaseSensitive(json, "ballRadius");
    if (!cJSON_IsNumber(ball_radius_cjson)) {
        panic("expected type number for field \"ball_radius\", got something "
              "else");
    }
    res.ball_radius = (u16)ball_radius_cjson->valueint;

    cJSON_Delete(json);
    return res;
}
