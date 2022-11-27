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
#include "driver.h"
#include "input.h"
#include "lang.h"
#include "ui.h"
#include "util.h"

static void ui_browse_menu_draw_line(uint8_t entry_id, uint8_t y, uint8_t color) {
    if (entry_id < 16) {
        ui_printf(false, 1, y, color, lang_keys[LK_UI_BROWSE_SLOT], entry_id + 1);
    }
}

static uint8_t iterate_carts(uint8_t *menu_list, uint8_t i) {
    uint8_t buffer[16];

    ui_step_work_indicator();
    driver_unlock();
    for (uint8_t slot = 0; slot <= 15; slot++) {
        ui_step_work_indicator();
        if (driver_get_launch_slot() == slot) continue;

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
    uint8_t menu_list[257];
    uint8_t i = 0;

    i = iterate_carts(menu_list, i);
    menu_list[i++] = MENU_ENTRY_END;

    uint8_t result = ui_menu_select(menu_list, ui_browse_menu_draw_line);
    if (result < 16) {
        driver_unlock();
        driver_launch_slot(0, result, 0xFF);
    }
}