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

#ifndef _LEADERBOARD_H
#define _LEADERBOARD_H

#include "3rdparty/include/a_string.h"
#include "common.h"
#include <time.h>

typedef struct LeaderboardEntry {
    a_string name;
    time_t time;
    u32 score;
    u32 total_score;
    u32 rows;

    // internal use data
    bool _hovered;

    struct LeaderboardEntry* next; // owned on the heap
} LeaderboardEntry;

typedef struct {
    FILE* fp; // null if not loaded from file
    LeaderboardEntry* head;
} Leaderboard;

/**
 * Creates a new leaderboard.
 *
 * @param file the filename the leaderboard should be read from. NULL = not
 * reading from file
 * @return a new leaderboard with all entries loaded in. head may be null if the
 * file is empty.
 *
 */
Leaderboard leaderboard_new(const char* file);

/**
 * Saves a leaderboard to `fp` and closes it if `fp` is not NULL.
 *
 * @param lb the leaderboard to be closed.
 *
 */
void leaderboard_close(Leaderboard* lb);

/**
 * Destroys a leaderboard. This will not save and close the file pointer. Please
 * call `leaderboard_close()` first.
 *
 * @param lb the leaderboard to be destroyed.
 *
 */
void leaderboard_destroy(Leaderboard* lb);
void leaderboard_print(Leaderboard* lb);
void leaderboard_draw(Leaderboard* lb);
void leaderboard_update(Leaderboard* lb);
LeaderboardEntry* leaderboard_end(Leaderboard* lb);
usize leaderboard_length(Leaderboard* lb);
void leaderboard_add_entry(Leaderboard* lb, LeaderboardEntry* entry);

// ownership of the a_string will be taken
LeaderboardEntry* leaderboard_entry_new(a_string name, time_t time, u32 score,
                                        u32 total_score, u32 rows);
LeaderboardEntry* leaderboard_entry_from_line(const char* line);
void leaderboard_entry_destroy(LeaderboardEntry* e);
void leaderboard_entry_print(LeaderboardEntry* e);
void leaderboard_entry_draw(LeaderboardEntry* e, usize index, i32 y);
void leaderboard_entry_draw_tooltip(LeaderboardEntry* e);
void leaderboard_entry_update(LeaderboardEntry* e);

#endif
