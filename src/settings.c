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
#include "lang.h"
#include "settings.h"
#include "ui.h"

// Saving a 1KB settings block across 128 slots allows for wear leveling.
// Given that one block of WSFM flash is rated for 100,000 erases, this should give >12 million
// settings changes over the lifespan of the device.
// (I'd have preferred an SD card slot, but you gotta work with what you gotta work with.)

#define SETTINGS_BANK 0xF8
#define MAX_SETTINGS_SLOT 127

uint8_t settings_slot;
settings_t settings_local;
bool settings_changed;
const char __far settings_magic[4] = {'w', 'f', 'C', 'F'};

void settings_reset(void) {
    memset(((uint8_t*) &settings_local) + sizeof(settings_magic), 0, sizeof(settings_local) - sizeof(settings_magic));
    memcpy(settings_local.magic, settings_magic, sizeof(settings_magic));
    settings_local.version = SETTINGS_VERSION;

    // game/SRAM slots
    uint8_t sram_slot = 0;
    for (uint8_t i = 0; i < GAME_SLOTS; i++) {
        if (i == driver_get_launch_slot()) {
            settings_local.slot_type[i] = SLOT_TYPE_LAUNCHER;
        } else {
            settings_local.slot_type[i] = SLOT_TYPE_SOFT;
            if (sram_slot < GAME_SLOTS) {
                settings_local.sram_slot_mapping[sram_slot++] = i;
            }
        }
        settings_local.slot_name[i][0] = 0;
        settings_local.slot_name[i][1] = 0;
    }
    while (sram_slot < SRAM_SLOTS) {
        settings_local.sram_slot_mapping[sram_slot++] = 0xFF;
    }
    settings_local.active_sram_slot = SRAM_SLOT_FIRST_BOOT;
    settings_local.color_theme = 0x02;

    settings_changed = true;
    settings_slot = MAX_SETTINGS_SLOT;
}

void settings_load(void) {
    settings_changed = false;
    bool settings_found = false;

    if (driver_get_launch_slot() != 0xFF) {
        driver_unlock();

        settings_slot = MAX_SETTINGS_SLOT;
        while (settings_slot <= MAX_SETTINGS_SLOT) {
            memset(&settings_local, 0, 6);
            driver_read_slot(&settings_local, driver_get_launch_slot(), SETTINGS_BANK + (settings_slot >> 6), settings_slot << 10, 6);

            if (!memcmp(settings_magic, &settings_local, 4)) {
                driver_read_slot(((uint8_t*) &settings_local) + 6, driver_get_launch_slot(), SETTINGS_BANK + (settings_slot >> 6), (settings_slot << 10) + 6, sizeof(settings_local) - 6);
                settings_found = true;
                break;
            } else {
                settings_slot--;
            }
        }

        driver_lock();
    }

    if (!settings_found) {
        settings_reset();
    } else {
        // version migration
        if (settings_local.version > SETTINGS_VERSION) {
        	ui_show();
            ui_dialog_run(0, 0, LK_DIALOG_SETTINGS_TOO_NEW, LK_DIALOG_OK);
            settings_reset();
            return;
        }

        if (settings_local.version < 1) {
            settings_local.color_theme = 0;
        }
        if (settings_local.version < 2) {
            for (uint8_t i = 0; i < GAME_SLOTS; i++) {
                settings_local.slot_name[i][0] = 0;
                settings_local.slot_name[i][1] = 0;
            }
        }

        settings_local.version = SETTINGS_VERSION;
    }
}

void settings_mark_changed(void) {
    settings_changed = true;
}

void settings_save(void) {
    if (!settings_changed) return;
    if (driver_get_launch_slot() == 0xFF) return;

    ui_step_work_indicator();

    if (settings_slot >= MAX_SETTINGS_SLOT) {
        driver_erase_bank(0, driver_get_launch_slot(), SETTINGS_BANK);
        settings_slot = 0;
    } else {
        settings_slot++;
    }

    driver_write_slot(&settings_local, driver_get_launch_slot(), SETTINGS_BANK + (settings_slot >> 6), settings_slot << 10, sizeof(settings_local));

    ui_clear_work_indicator();
    settings_changed = false;
}
