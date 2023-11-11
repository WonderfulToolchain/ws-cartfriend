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

#define SLOT_TYPE_SOFT 0
#define SLOT_TYPE_LAUNCHER 1
#define SLOT_TYPE_MULTILINEAR_SOFT 2 /* Tentative */
#define SLOT_TYPE_APPENDED_FILES 3 /* Tentative */
#define SLOT_TYPE_UNUSED 0xFF

#define SETTINGS_VERSION 5

#define SRAM_SLOT_ALL 0xFD
#define SRAM_SLOT_FIRST_BOOT 0xFE
#define SRAM_SLOT_NONE 0xFF

extern bool settings_location_legacy;
typedef struct __attribute__((packed)) {
	uint8_t magic[4];
	uint16_t version; // 6

	uint8_t slot_type[GAME_SLOTS]; // 22
	uint8_t active_sram_slot; // 23
	uint8_t sram_slot_mapping[SRAM_SLOTS]; // 38

	uint8_t color_theme; // 39

	uint8_t slot_name[GAME_SLOTS][24]; // 423

	uint8_t flags1; // 424
	uint8_t language; // 425
} settings_t;

#define SETT_FLAGS1_HIDE_SLOT_IDS 0x01
#define SETT_FLAGS1_DISABLE_BUFFERED_WRITES 0x02
#define SETT_FLAGS1_UNLOCK_IEEP_NEXT_BOOT 0x04
#define SETT_FLAGS1_SERIAL_9600BPS 0x08
#define SETT_FLAGS1_WIDE_SCREEN 0x10
#define SETT_FLAGS1_FORCE_FAST_SRAM 0x20

extern settings_t settings_local;
extern bool settings_changed;
extern const char __far settings_magic[4];

void settings_reset(void);
void settings_load(void);
void settings_refresh(void);
void settings_mark_changed(void);
void settings_save(void);
