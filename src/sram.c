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
#include <wonderful.h>
#include <ws.h>
#include "config.h"
#include "driver.h"
#include "error.h"
#include "lang.h"
#include "settings.h"
#include "sram.h"
#include "ui.h"
#include "util.h"

#ifdef USE_SLOT_SYSTEM
#define USE_PARTIAL_WRITES

static inline uint8_t sram_get_bank(uint8_t sram_slot, uint8_t sub_bank) {
    uint8_t slot = 0x80 | (sram_slot << 3);
    uint8_t bank = slot + sub_bank;
    // carve out a settings area between F40000 .. F5FFFF
    // this allows writing a Pocket Challenge V2 bootloader there
    if (!settings_location_legacy && bank >= 0xF4) bank += 2;
    if (bank >= 0xFA) {
        error_critical(ERROR_CODE_SRAM_SLOT_OVERFLOW_UNKNOWN, sram_slot);
    }
    
    return slot;
}

bool sram_copy_to_buffer_check_flash(void* restrict s1, uint16_t offset);
bool sram_copy_from_bank1(uint16_t offset, uint16_t words);

static void sram_backup_restore_slot(uint8_t sram_slot, bool is_restore) {
    uint8_t driver_slot = driver_get_launch_slot();
    uint8_t buffer[256];
    ui_pbar_state_t pbar = {
        .x = 0,
        .y = 13,
        .width = 27
    };
    ui_pbar_init(&pbar);

    ui_reset_main_screen();
    if (settings_location_legacy) {
        ui_puts_centered(false, 2, 0, lang_keys[LK_UI_MSG_MIGRATING]);
    } else {
        ui_puts_centered(false, 2, 0, lang_keys[is_restore ? LK_UI_MSG_RESTORE_SRAM : LK_UI_MSG_BACKUP_SRAM]);
    }

    if (_CS >= 0x2000) {
        if (is_restore) {
            pbar.step_max = 256;
            for (uint16_t i = 0; i < 256; i++) {
                pbar.step = i;
                ui_pbar_draw(&pbar);
                ui_step_work_indicator();

                if (!(i & 31)) {
                    outportb(IO_BANK_RAM, i >> 5);
                    uint8_t bank = sram_get_bank(sram_slot, i >> 5);
                    outportb(IO_BANK_ROM1, bank);
                    asm volatile("" ::: "memory");
                }
                uint16_t offset = (i << 11);

                // ROM -> SRAM
                sram_copy_from_bank1(offset, 2048 >> 1);
                // memcpy(MK_FP(0x1000, offset), MK_FP(0x3000, offset), 2048);
            }
        } else {
            // for backup, erase slots first
            for (uint8_t i = 0; i < 8; i++) {
                ui_step_work_indicator();
                uint8_t bank = sram_get_bank(sram_slot, i);
                driver_erase_bank(0, driver_slot, bank);
            }

            pbar.step_max = 2048;
            for (uint16_t i = 0; i < 2048; i++) {
                pbar.step = i;
                if (!(i & 7)) {
                    ui_pbar_draw(&pbar);
                    if (!(i & 255)) {
                        outportb(IO_BANK_RAM, i >> 8);
                        asm volatile("" ::: "memory");
                    }
                }
                ui_step_work_indicator();

                uint8_t bank = sram_get_bank(sram_slot, i >> 8);
                uint16_t offset = (i << 8);

#ifdef USE_PARTIAL_WRITES
                if (sram_copy_to_buffer_check_flash(buffer, offset)) {
                    driver_write_slot(buffer, driver_slot, bank, offset, sizeof(buffer));
                }
#else
                uint8_t __far* sram_buffer = MK_FP(0x1000, offset);
                memcpy(buffer, sram_buffer, 256);
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
        .x = 0,
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
        pbar.step_max = 8 * SRAM_SLOTS;
        ui_pbar_init(&pbar);

        uint8_t driver_slot = driver_get_launch_slot();

        for (uint8_t i = 0; i < pbar.step_max; i++) {
            pbar.step = i;
            ui_pbar_draw(&pbar);
            ui_step_work_indicator();

            uint8_t rom_slot = sram_get_bank(i >> 3, i & 7);
            driver_erase_bank(0, driver_slot, rom_slot);
        }
    } else {
        pbar.step_max = 8;
        ui_pbar_init(&pbar);

        uint8_t driver_slot = driver_get_launch_slot();

        for (uint8_t i = 0; i < 8; i++) {
            pbar.step = i;
            ui_pbar_draw(&pbar);
            ui_step_work_indicator();

            uint8_t rom_slot = sram_get_bank(sram_slot, i);
            driver_erase_bank(0, driver_slot, rom_slot);
        }
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
        sram_backup_restore_slot(settings_local.active_sram_slot, false);
        settings_local.active_sram_slot = SRAM_SLOT_NONE;
        settings_mark_changed();
    }

    if (sram_slot < SRAM_SLOTS) {
        sram_backup_restore_slot(sram_slot, true);
        settings_local.active_sram_slot = sram_slot;
        settings_mark_changed();
    }
}
#else
void sram_switch_to_slot(uint8_t sram_slot) {
    // stub
}
#endif
