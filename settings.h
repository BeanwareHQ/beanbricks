#pragma once

// colors
#include <raylib.h>

// --- DO NOT TOUCH BELOW ---
typedef enum {
    THEME_DEFAULT = 0,
    THEME_DARK = 1,
    CTP_MOCHA = 2,
    CTP_MACCHIATO = 3,
    CTP_FRAPPE = 4,
    CTP_LATTE = 5,
} Theme;

#define THEME_DEFAULT 0 // Default light mode
#define THEME_DARK    1 // Default dark mode
#define CTP_MOCHA     2 // Catppuccin Mocha
#define CTP_MACCHIATO 3 // Catppiccin Macchiato
#define CTP_FRAPPE    4 // Catppuccin Frappe
#define CTP_LATTE     5 // Catppuccin Latte
// --- DO NOT TOUCH ABOVE ---

// --- Configuration Options---
#define WINWIDTH      1024 // Window Width
#define WINHEIGHT     768  // Window Height
#define PADDLE_WIDTH  100  // Paddle Width
#define PADDLE_HEIGHT 20   // Paddle Height
#define PADDLE_SPEED  5    // Paddle Speed
#define LAYERS        7    // Layers of bricks (above 7 is undefined behavior)

#define BRICK_WIDTH  50 // Width of a single brick
#define BRICK_HEIGHT 10 // Height of a single brick

#define BALL_RADIUS 10 // Ball Radius

#define THEME CTP_MOCHA // Theme (choose options above)
