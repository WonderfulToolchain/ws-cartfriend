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
#include <string.h>
#include "config.h"
#include "driver.h"
#include "settings.h"

settings_t settings_local;
bool settings_changed;
const char __far settings_magic[4] = {'w', 'f', 'C', 'F'};

void settings_clear(void) {
    memset(&settings_local, 0, sizeof(settings_local));
    memcpy(settings_local.magic, settings_magic, sizeof(settings_magic));
    settings_local.ver_major = SETTINGS_VERSION_MAJOR;
    settings_local.ver_minor = SETTINGS_VERSION_MINOR;

    uint8_t sram_slot = 0;
    for (uint8_t i = 0; i < GAME_SLOTS; i++) {
        if (i == driver_get_launch_slot()) {
            settings_local.slot_type[i] = SLOT_TYPE_LAUNCHER;
            settings_local.sram_slot_mapping[i] = 0xFF;
        } else {
            settings_local.slot_type[i] = SLOT_TYPE_GAME;
            settings_local.sram_slot_mapping[i] = sram_slot++;
        }
    }

    settings_changed = true;
}

void settings_load(void) {
    settings_changed = false;

    // TODO
    settings_clear();
}

void settings_save(void) {
    if (!settings_changed) return;
    // TODO
}