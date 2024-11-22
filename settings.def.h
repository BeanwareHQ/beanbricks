#pragma once

// colors
#include <raylib.h>

// --- DO NOT TOUCH BELOW ---
typedef enum {
    THEME_DEFAULT = 0,
    THEME_DARK = 1,
    THEME_CTP_MOCHA = 2,
    THEME_CTP_MACCHIATO = 3,
    THEME_CTP_FRAPPE = 4,
    THEME_CTP_LATTE = 5,
} Theme;

#define THEME_DEFAULT       0 // Default light mode
#define THEME_DARK          1 // Default dark mode
#define THEME_CTP_MOCHA     2 // Catppuccin Mocha
#define THEME_CTP_MACCHIATO 3 // Catppiccin Macchiato
#define THEME_CTP_FRAPPE    4 // Catppuccin Frappe
#define THEME_CTP_LATTE     5 // Catppuccin Latte
// --- DO NOT TOUCH ABOVE ---

// --- Configuration Options---
#define WINWIDTH             600 // Window Width
#define WINHEIGHT            800 // Window Height
#define PADDLE_WIDTH         100 // Paddle Width
#define PADDLE_HEIGHT        20  // Paddle Height
#define INITIAL_PADDLE_SPEED 6   // Paddle Speed
#define LAYERS               5 // Layers of bricks (above 7 is undefined behavior)

#define BRICK_WIDTH  50 // Width of a single brick
#define BRICK_HEIGHT 10 // Height of a single brick

#define BALL_RADIUS 10 // Ball Radius

/*
#define DEBUG_INFO // Optional, uncomment to enable
*/

#define SPEED                                                                  \
    1 // Global speed multiplier. A decimal makes the game slower, 1=60FPS. 0 is
      // undefined.

#define THEME THEME_DEFAULT // Theme (choose options above)
