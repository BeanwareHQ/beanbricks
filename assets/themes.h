/*
 * beanbricks.c: a questionable breakout clone in C and Raylib.
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2024-2025.
 *
 * This source code form is wholly licensed under the MIT/Expat license. View
 * the full license text in the root of the project.
 *
 * INFO: wrapper for raygui themes to supress errors
 */

#pragma once

#include "catppuccinlattesapphire.h"
#include "catppuccinmochamauve.h"
#include "catppuccinfrappesapphire.h"
#include "catppuccinmacchiatosapphire.h"

extern void GuiLoadStyleCatppuccinLatteSapphire(void);
extern void GuiLoadStyleCatppuccinFrappeSapphire(void);
extern void GuiLoadStyleCatppuccinMacchiatoSapphire(void);
extern void GuiLoadStyleCatppuccinMochaMauve(void);
