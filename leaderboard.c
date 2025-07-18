/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: leaderboard code
 */
#define _POSIX_C_COURSE 200809L

#include <ctype.h>
#include <raylib.h>
#include <string.h>

#include "beanbricks.h"
#include "common.h"
#include "leaderboard.h"

Leaderboard leaderboard_new(const char* file) {
    if (file != NULL)
        panic("not implemented");

    // not written as (Leaderboard){0}; for clarity
    return (Leaderboard){
        .fp = NULL,
        .head = NULL,
    };
}

void leaderboard_deinit(Leaderboard* lb) {
    if (lb->fp != NULL)
        panic("not implemented");

    LeaderboardEntry* curr = lb->head;
    while (curr != NULL) {
        LeaderboardEntry* next = curr->next;
        leaderboard_entry_deinit(curr);
        curr = next;
    }

    *lb = (Leaderboard){0};
}

void leaderboard_print(Leaderboard* lb) {
    LeaderboardEntry* curr = lb->head;
    usize index = 0;

    while (curr != NULL) {
        eprintf("%zu | ", index);
        leaderboard_entry_print(curr);
        curr = curr->next;
        index++;
    }
}
void leaderboard_draw(Leaderboard* lb) {
    LeaderboardEntry* curr = lb->head;
    usize index = 0;
    usize y = 350;
    LeaderboardEntry* hovered = NULL;

    if (curr == NULL) {
        // leaderboard is empty

        char* txt = "leaderboard empty... play a game to get started!";
        i32 txtsz = 20;
        i32 txt_width = MeasureText(txt, txtsz);
        DrawText(txt, (i32)(WIN_WIDTH / 2 - txt_width / 2), y, txtsz,
                 TXT_SECONDARY);
        return;
    }

    while (index < 10 && curr != NULL) {
        leaderboard_entry_draw(curr, index,
                               y + index * LEADERBOARD_ENTRY_HEIGHT);
        if (curr->_hovered)
            hovered = curr;
        curr = curr->next;
        index++;
    }

    // handle tooltips
    if (hovered != NULL) {
        leaderboard_entry_draw_tooltip(hovered);
    }
}

void leaderboard_update(Leaderboard* lb) {
    LeaderboardEntry* curr = lb->head;
    usize index = 0;
    usize y = 350;
    usize x = (WIN_WIDTH / 2) - (LEADERBOARD_ENTRY_WIDTH / 2);

    Rectangle lb_entry_rec = {
        .x = x,
        .y = y,
        .width = LEADERBOARD_ENTRY_WIDTH,
        .height = LEADERBOARD_ENTRY_HEIGHT,
    };

    // this update function runs every frame, so mouse data does
    // not change in the loop.

    Vector2 mouse_pos = GetMousePosition();
    Rectangle mouse_rec = {
        .x = mouse_pos.x,
        .y = mouse_pos.y,
        .width = 1,
        .height = 1,
    };

    while (index < 10 && curr != NULL) {
        bool mouse_over_entry = CheckCollisionRecs(lb_entry_rec, mouse_rec);
        // im a lazy piece of shit
        if (mouse_over_entry && !curr->_hovered)
            curr->_hovered = true;
        else if (!mouse_over_entry && curr->_hovered)
            curr->_hovered = false;

        leaderboard_entry_update(curr);
        curr = curr->next;
        index++;
        lb_entry_rec.y = y + index * LEADERBOARD_ENTRY_HEIGHT;
    }
}

LeaderboardEntry* leaderboard_end(Leaderboard* lb) {
    if (lb->head == NULL)
        return NULL;

    LeaderboardEntry* curr = NULL;
    curr = lb->head;
    while (curr->next != NULL)
        curr = curr->next;
    return curr;
}
usize leaderboard_length(Leaderboard* lb);

void leaderboard_add_entry(Leaderboard* lb, LeaderboardEntry* entry) {

    LeaderboardEntry* end = leaderboard_end(lb);
    if (end == NULL) {
        lb->head = entry;
    } else {
        end->next = entry;
    }
}

LeaderboardEntry* leaderboard_entry_new(a_string name, time_t time, u32 score,
                                        u32 total_score, u32 rows) {
    LeaderboardEntry* res = calloc(1, sizeof(LeaderboardEntry));
    check_alloc(res);
    *res = (LeaderboardEntry){
        .name = name,
        .time = time,
        .score = score,
        .total_score = total_score,
        .rows = rows,
        .next = NULL,
        ._hovered = false,
    };

    return res;
}

LeaderboardEntry* leaderboard_entry_from_line(const char* line) {
    /*
     * The line should look something like the following:
     *
     * NAME              TIME  SCORE TOTAL_SCORE ROWS
     * "the user's name" 70    90    180         3
     *
     */

    const usize line_len = strlen(line);
    usize curr = 0;

    if (line_len < 2)
        return NULL;

    // chop off all the garbage
    while (line[curr] != '"')
        curr++;

    // find the end delim
    usize name_begin = curr + 1;
    while (line[curr] != '"')
        curr++;

    // now the end char is "
    usize name_end = curr - 1;
    usize name_len = name_end - name_begin;

    a_string name_buf = a_string_with_capacity(name_len + 1);
    a_string_copy_cstr(&name_buf, (char*)line + name_begin);

    // chop off whitespaces
    curr++; // skip past "
    while (isspace(line[curr]))
        curr++;

    usize nums_len = strlen((char*)line + curr);
    a_string nums = a_string_with_capacity(nums_len + 1);
    a_string_copy_cstr(&nums, (char*)line + curr);

    char* curr_tok = NULL;
    char* strtol_end = NULL;

    curr_tok = strtok(nums.data, " ");
    if (curr_tok == NULL)
        panic("expected time in line `%s`", line);

    // parse time
    time_t time = (time_t)strtol(curr_tok, &strtol_end, 10);
    if (*strtol_end != '\0')
        panic("couldnt parse line `%s` at position %zu", line, curr);

    curr_tok = strtok(NULL, " ");
    if (curr_tok == NULL)
        panic("expected score in line `%s`", line);
    // parse score
    u32 score = strtol(curr_tok, &strtol_end, 10);
    if (*strtol_end != '\0')
        panic("couldnt parse line `%s` at position %zu", line, curr);

    // parse total_score
    if (curr_tok == NULL)
        panic("expected total_score in line `%s`", line);
    u32 total_score = strtol(curr_tok, &strtol_end, 10);
    if (*strtol_end != '\0')
        panic("couldnt parse line `%s` at position %zu", line, curr);

    // parse rows
    if (curr_tok == NULL)
        panic("expected rows in line `%s`", line);
    u32 rows = strtol(curr_tok, &strtol_end, 10);
    if (*strtol_end != '\0')
        panic("couldnt parse line `%s` at position %zu", line, curr);

    LeaderboardEntry* res =
        leaderboard_entry_new(name_buf, time, score, total_score, rows);

    a_string_free(&nums);
    a_string_free(&name_buf);

    return res;
}

void leaderboard_entry_deinit(LeaderboardEntry* e) {
    a_string_free(&e->name);
    // free(e);
}

void leaderboard_entry_print(LeaderboardEntry* e) {
    eprintf("name: `%s`, time: %lu, score: %d, total_score: %d, rows: %d\n",
            e->name.data, e->time, e->score, e->total_score, e->rows);
}

void leaderboard_entry_draw(LeaderboardEntry* e, usize index, i32 y) {
    const i32 BEGIN = WIN_WIDTH / 2 - LEADERBOARD_ENTRY_WIDTH / 2;
    const i32 RANKING_BOX_WIDTH = MeasureText("00", 20) + 10; // +padding

    const Rectangle box = {
        .x = BEGIN,
        .y = y,
        .width = LEADERBOARD_ENTRY_WIDTH,
        .height = LEADERBOARD_ENTRY_HEIGHT,
    };

    const Rectangle ranking_rec = {
        .x = BEGIN,
        .y = y,
        .width = RANKING_BOX_WIDTH,
        .height = LEADERBOARD_ENTRY_HEIGHT, // 10 + 20 + 10
    };

    char buf[5] = {0};
    snprintf(buf, sizeof(buf), "%02zu", index + 1);
    const i32 RANKING_TEXT_WIDTH = MeasureText(buf, 20);

    DrawRectangleRec(box, DARK_SURFACE_COLOR);
    DrawRectangleRec(ranking_rec, LIGHT_SURFACE_COLOR);

    DrawText(buf, BEGIN + (RANKING_BOX_WIDTH / 2 - RANKING_TEXT_WIDTH / 2),
             y + 5, 20, TXT_PRIMARY);

    DrawText(e->name.data, BEGIN + RANKING_BOX_WIDTH + 10, y + 5, 20,
             TXT_SECONDARY);
}

void leaderboard_entry_draw_tooltip(LeaderboardEntry* e) {
    Vector2 mouse_pos = GetMousePosition();

    // rows:
    // -----
    //
    // name: <name>
    // score: <score>/<total_score>
    // time: <time>
    // rows: <rows>

    char rows[4][50] = {0};
    usize max_row_len = sizeof(rows[0]);

    snprintf(rows[0], max_row_len, "name: %s", e->name.data);
    snprintf(rows[1], max_row_len, "score: %d/%d", e->score, e->total_score);
    snprintf(rows[2], max_row_len, "time: %d", (i32)e->time);
    snprintf(rows[3], max_row_len, "rows: %d", e->rows);

    // draw relative to the mouse position
    i32 txt_x = mouse_pos.x + 7; // border + padding
    i32 txt_y = mouse_pos.y + 7;

    i32 max_width = MeasureText(rows[0], 20);
    for (usize i = 0; i < LENGTH(rows); i++) {
        i32 width = MeasureText(rows[i], 20);

        if (width > max_width) {
            max_width = width;
        }
    }

    Rectangle bounds = {
        .x = mouse_pos.x,
        .y = mouse_pos.y,
        .width = max_width + 14,                                // + 2*padding
        .height = (txt_y + 35 + 25 * LENGTH(rows)) - txt_y + 7, // padding
    };

    Rectangle contents = bounds;
    contents.x += 2;
    contents.y += 2;
    contents.width -= 4;
    contents.height -= 4;

    DrawRectangleRec(bounds, color(BRICK_COLORS[5]));
    DrawRectangleRec(contents, DARK_SURFACE_COLOR);

    DrawText("Stats", txt_x, txt_y, 30, TXT_PRIMARY);
    txt_y += 35;

    for (usize i = 0; i < LENGTH(rows); i++) {
        DrawText(rows[i], txt_x, txt_y, 20, TXT_SECONDARY);
        txt_y += 25;
    }
}

void leaderboard_entry_update(LeaderboardEntry* e) {
    // suppress warning
    e = e;
    return;
}
