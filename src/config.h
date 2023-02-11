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

#ifdef TARGET_flash_masta
#define USE_SLOT_SYSTEM
#define FLASH_CONFIG_SIZE 1024
#define GAME_SLOTS 16
#define SRAM_SLOTS 15
#endif

#ifdef TARGET_generic
// TODO: No longer rely on these values.
#define FLASH_CONFIG_SIZE 1024
#define GAME_SLOTS 16
#define SRAM_SLOTS 15
#endif

// #define USE_LOW_BATTERY_WARNING
