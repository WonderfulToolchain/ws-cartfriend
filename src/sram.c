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
#include "lang.h"
#include "settings.h"
#include "sram.h"
#include "ui.h"
#include "util.h"
#include "ws/cartridge.h"

static uint8_t sram_get_slot(uint8_t sram_slot) {
    return 0x80 | (sram_slot << 3);
}

void sram_backup_restore_slot(uint8_t sram_slot, bool is_restore) {
    uint8_t rom_slot = sram_get_slot(sram_slot);
    uint8_t driver_slot = driver_get_launch_slot();
    uint8_t buffer[256];
    ui_pbar_state_t pbar = {
        .step_max = 2048,
        .x = 1,
        .y = 13,
        .width = 26
    };
    ui_pbar_init(&pbar);

    ui_reset_main_screen();
    ui_puts_centered(false, 2, 0, lang_keys[is_restore ? LK_DIALOG_RESTORE_SRAM : LK_DIALOG_BACKUP_SRAM]);

    if (!is_restore) {
        // for backup, erase slots first
        for (uint8_t i = 0; i < 8; i++) {
            driver_erase_bank(0, driver_slot, rom_slot + i);
        }
    }

    for (uint16_t i = 0; i < 2048; i++) {
        pbar.step = i;
        ui_pbar_draw(&pbar);
        ui_step_work_indicator();

        ws_bank_ram_set(i >> 8);
        uint8_t bank = (i >> 8) | rom_slot;
        uint16_t offset = (i << 8);
        uint8_t __far* sram_buffer = MK_FP(0x1000, offset);

        if (_CS < 0x2000) {
            // if booting from SRAM, fake this step
            wait_for_vblank();
            wait_for_vblank();
        } else {
            if (is_restore) {
                // ROM -> SRAM
                driver_read_slot(buffer, driver_slot, bank, offset, sizeof(buffer));
                memcpy(sram_buffer, buffer, 256);
            } else /* is_backup */ {
                // SRAM -> ROM
                memcpy(buffer, sram_buffer, 256);
                uint16_t skip_len = 0;
                while (skip_len < 256) {
                    if (buffer[skip_len] != 0xFF) {
                        break;
                    } else {
                        skip_len++;
                    }
                }
                if (skip_len < 256) {
                    driver_write_slot(buffer, driver_slot, bank, offset + skip_len, sizeof(buffer) - skip_len);
                }
            }
        }
    }

    ui_clear_work_indicator();
}

void sram_erase(uint8_t sram_slot) {
    ui_reset_main_screen();
    ui_puts_centered(false, 2, 0, lang_keys[LK_DIALOG_ERASE_SRAM]);

    ui_pbar_state_t pbar = {
        .x = 1,
        .y = 13,
        .width = 26
    };

    if (sram_slot == settings_local.active_sram_slot || sram_slot == SRAM_SLOT_NONE) {
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

            uint8_t rom_slot = sram_get_slot(i >> 3);
            driver_erase_bank(0, driver_slot, rom_slot + (i & 7));
        }
    } else {
        pbar.step_max = 8;
        ui_pbar_init(&pbar);

        uint8_t rom_slot = sram_get_slot(sram_slot);
        uint8_t driver_slot = driver_get_launch_slot();

        for (uint8_t i = 0; i < 8; i++) {
            pbar.step = i;
            ui_pbar_draw(&pbar);
            ui_step_work_indicator();

            driver_erase_bank(0, driver_slot, rom_slot + i);
        }
    }
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

    if (settings_local.active_sram_slot != SRAM_SLOT_NONE) {
        sram_backup_restore_slot(settings_local.active_sram_slot, false);
        settings_local.active_sram_slot = SRAM_SLOT_NONE;
        settings_mark_changed();
    }

    if (sram_slot != SRAM_SLOT_NONE) {
        sram_backup_restore_slot(sram_slot, true);
        settings_local.active_sram_slot = sram_slot;
        settings_mark_changed();
    }
}