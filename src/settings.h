#pragma once
/**
 * Copyright (c) 2022 Adrian Siekierka
 *
 * CartFriend is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * CartFriend is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with CartFriend. If not, see <https://www.gnu.org/licenses/>. 
 */

#include <stdbool.h>
#include <stdint.h>
#include "config.h"

#define SLOT_TYPE_GAME 0
#define SLOT_TYPE_LAUNCHER 1

#define SETTINGS_VERSION_MAJOR 0
#define SETTINGS_VERSION_MINOR 0

typedef struct __attribute__((packed)) {
	uint8_t magic[4];
	uint8_t ver_major;
	uint8_t ver_minor; // 6
	uint8_t language; // 7
	uint8_t slot_type[GAME_SLOTS]; // 23
	uint8_t sram_slot_mapping[GAME_SLOTS]; // 38
} settings_t;

extern settings_t settings_local;
extern bool settings_changed;
extern const char __far settings_magic[4];

void settings_clear(void);
void settings_load(void);
void settings_save(void);