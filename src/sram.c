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
#include <string.h>
#include <wonderful-asm.h>
#include <ws.h>
#include "config.h"
#include "driver.h"
#include "error.h"
#include "lang.h"
#include "settings.h"
#include "sram.h"
#include "ui.h"
#include "util.h"
#include "ws/cartridge.h"
#include "ws/hardware.h"

#define USE_PARTIAL_WRITES

static uint8_t sram_get_slot(uint8_t sram_slot) {
    uint8_t slot = 0x80 | (sram_slot << 3);
    if (slot >= 0xF8) {
        error_critical(ERROR_CODE_SRAM_SLOT_OVERFLOW_UNKNOWN, sram_slot);
    }
    return slot;
}

static void sram_backup_restore_slot(uint8_t sram_slot, bool is_restore) {
    uint8_t rom_slot = sram_get_slot(sram_slot);
    uint8_t driver_slot = driver_get_launch_slot();
    uint8_t buffer[256];
    ui_pbar_state_t pbar = {
        .x = 1,
        .y = 13,
        .width = 27
    };
    ui_pbar_init(&pbar);

    ui_reset_main_screen();
    ui_puts_centered(false, 2, 0, lang_keys[is_restore ? LK_UI_MSG_RESTORE_SRAM : LK_UI_MSG_BACKUP_SRAM]);

    if (_CS >= 0x2000) {
        if (is_restore) {
            pbar.step_max = 256;
            for (uint16_t i = 0; i < 256; i++) {
                pbar.step = i;
                ui_pbar_draw(&pbar);
                ui_step_work_indicator();

                ws_bank_ram_set(i >> 5);
                uint8_t bank = (i >> 5) | rom_slot;
                uint16_t offset = (i << 11);
                outportb(IO_BANK_ROM1, bank);

                // ROM -> SRAM
                memcpy(MK_FP(0x1000, offset), MK_FP(0x3000, offset), 2048);
            }
        } else {
            // for backup, erase slots first
            for (uint8_t i = 0; i < 8; i++) {
                ui_step_work_indicator();
                driver_erase_bank(0, driver_slot, rom_slot + i);
            }

            pbar.step_max = 2048;
            for (uint16_t i = 0; i < 2048; i++) {
                pbar.step = i;
                ui_pbar_draw(&pbar);
                ui_step_work_indicator();

                ws_bank_ram_set(i >> 8);
                uint8_t bank = (i >> 8) | rom_slot;
                uint16_t offset = (i << 8);
                uint8_t __far* sram_buffer = MK_FP(0x1000, offset);

                memcpy(buffer, sram_buffer, 256);
#ifdef USE_PARTIAL_WRITES
                uint16_t *buffer16 = (uint16_t*) buffer;
                for (uint8_t i = 0; i < (sizeof(buffer) >> 1); i++) {
                    if (buffer16[i] != 0xFFFF) {
                        driver_write_slot(buffer, driver_slot, bank, offset, sizeof(buffer));
                        break;
                    }
                }
#else
                driver_write_slot(buffer, driver_slot, bank, offset, sizeof(buffer));
#endif
            }
        }
    }

    ui_clear_work_indicator();
    ui_update_indicators();
}

void sram_erase(uint8_t sram_slot) {
    ui_reset_main_screen();
    ui_puts_centered(false, 2, 0, lang_keys[LK_UI_MSG_ERASE_SRAM]);

    ui_pbar_state_t pbar = {
        .x = 1,
        .y = 13,
        .width = 27
    };

    if (sram_slot == SRAM_SLOT_NONE) {
        pbar.step_max = 128;
        ui_pbar_init(&pbar);

        for (uint16_t i = 0; i < 128; i++) { /* 16 * 8 */
            pbar.step = i;
            ui_pbar_draw(&pbar);
            ui_step_work_indicator();

            ws_bank_ram_set(i >> 4);
            uint8_t __far* sram_buffer = MK_FP(0x1000, i << 12);
            memset(sram_buffer, 0xFF, 1 << 12);
        }

        ui_clear_work_indicator();
    } else if (sram_slot == SRAM_SLOT_ALL) {
        driver_unlock();

        pbar.step_max = 8 * SRAM_SLOTS;
        ui_pbar_init(&pbar);

        uint8_t driver_slot = driver_get_launch_slot();

        for (uint8_t i = 0; i < pbar.step_max; i++) {
            pbar.step = i;
            ui_pbar_draw(&pbar);
            ui_step_work_indicator();

            uint8_t rom_slot = sram_get_slot(i >> 3);
            if ((rom_slot | 0x80) < 0xF8) {
                driver_erase_bank(0, driver_slot, rom_slot + (i & 7));
            }
        }

        driver_lock();
    } else {
        driver_unlock();

        pbar.step_max = 8;
        ui_pbar_init(&pbar);

        uint8_t rom_slot = sram_get_slot(sram_slot);
        uint8_t driver_slot = driver_get_launch_slot();

        for (uint8_t i = 0; i < 8; i++) {
            pbar.step = i;
            ui_pbar_draw(&pbar);
            ui_step_work_indicator();

            if ((rom_slot | 0x80) < 0xF8) {
                driver_erase_bank(0, driver_slot, rom_slot + i);
            }
        }

        driver_lock();
    }

    ui_update_indicators();
}

void sram_switch_to_slot(uint8_t sram_slot) {
    if (settings_local.active_sram_slot == sram_slot) return;

    if (settings_local.active_sram_slot == SRAM_SLOT_FIRST_BOOT) {
        // on first boot, assume SRAM contents are intended
        if (sram_slot != SRAM_SLOT_NONE) {
            settings_local.active_sram_slot = sram_slot;
            settings_mark_changed();
        }
        return;
    }

    bool unlocked = false;

    if (sram_slot >= SRAM_SLOTS && sram_slot != SRAM_SLOT_NONE) {
        error_critical(ERROR_CODE_SRAM_SLOT_OVERFLOW_SWITCH, sram_slot);
    }

    if (settings_local.active_sram_slot < SRAM_SLOTS) {
        if (!unlocked) { driver_unlock(); unlocked = true; }
        sram_backup_restore_slot(settings_local.active_sram_slot, false);
        settings_local.active_sram_slot = SRAM_SLOT_NONE;
        settings_mark_changed();
    }

    if (sram_slot < SRAM_SLOTS) {
        if (!unlocked) { driver_unlock(); unlocked = true; }
        sram_backup_restore_slot(sram_slot, true);
        settings_local.active_sram_slot = sram_slot;
        settings_mark_changed();
    }

    if (unlocked) driver_lock();
}
