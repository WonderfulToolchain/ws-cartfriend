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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <stdint.h>
#include "config.h"

#define SLOT_TYPE_UNKNOWN 0
#define SETTINGS_VERSION_MAJOR 0
#define SETTINGS_VERSION_MINOR 0

typedef struct __attribute__((packed)) {
	uint8_t magic[4];
	uint8_t ver_major;
	uint8_t ver_minor; // 6
	uint8_t slot_type[GAME_SLOTS]; // 21
	uint8_t sram_slot_mapping[15]; // 36
} settings_t;

extern settings_t settings_local;
extern const char __far settings_magic[4];

#endif
