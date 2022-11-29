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

#include <string.h>
#include <ws.h>
#include "config.h"
#include "driver.h"
#include "input.h"
#include "lang.h"
#include "nanoprintf.h"
#include "settings.h"
#include "sram.h"
#include "ui.h"
#include "util.h"

static void ui_browse_menu_build_line(uint8_t entry_id, char *buf, int buf_len, char *buf_right, int buf_right_len) {
    if (entry_id < 16) {
        npf_snprintf(buf, buf_len, lang_keys[LK_UI_BROWSE_SLOT], entry_id + 1);
    }
}

static uint8_t iterate_carts(uint8_t *menu_list, uint8_t i) {
    uint8_t buffer[16];

    ui_step_work_indicator();
    driver_unlock();
    for (uint8_t slot = 0; slot <= 15; slot++) {
        ui_step_work_indicator();
        if (driver_get_launch_slot() == slot) {
            // if booted from RAM/SRAM, don't skip launch slot
            if (_CS >= 0x2000) {
                continue;
            }
        }

        memset(buffer, 0, sizeof(buffer));
        if (driver_read_slot(buffer, slot, 0xFF, 0xFFF0, 16)) {
            if (buffer[0] == 0xEA) {
                menu_list[i++] = slot;
            } 
        }
    }
    driver_lock();

    return i;
}

void ui_browse(void) {
    uint8_t menu_list[65];
    uint8_t i = 0;

    i = iterate_carts(menu_list, i);
    menu_list[i++] = MENU_ENTRY_END;

    ui_menu_state_t menu = {
        .list = menu_list,
        .build_line_func = ui_browse_menu_build_line,
        .flags = 0
    };
    ui_menu_init(&menu);

    uint16_t result = ui_menu_select(&menu);
    if (result < 16) {
        ui_reset_main_screen();

        // does the game use SRAM?
        bool sram_used = false;
        driver_read_slot(menu_list, result, 0xFF, 0xFFF0, 16);
        sram_used = menu_list[0x0B] != 0;

        if (sram_used) {
            // figure out SRAM slots
            i = 0;
            for (uint8_t k = 0; k < SRAM_SLOTS; k++) {
                if (settings_local.sram_slot_mapping[k] == result) {
                    menu_list[i++] = k;
                }
            }
            uint8_t sram_slot = 0xFF;
            if (i > 1) {
                menu_list[i++] = MENU_ENTRY_END;
                menu.flags = MENU_B_AS_BACK;
                ui_menu_init(&menu);
                uint16_t result_sram = ui_menu_select(&menu);
                if (result_sram == MENU_ENTRY_END) {
                    return;
                } else {
                    sram_slot = menu_list[result_sram & 0xFF];
                }
            } else if (i == 1) {
                sram_slot = menu_list[0];
            }
        
            sram_switch_to_slot(sram_slot);
        }

        input_wait_clear();
		wait_for_vblank();
		wait_for_vblank();

        launch_slot(result, 0xFF);
    }
}