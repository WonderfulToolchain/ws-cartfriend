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
#include "lang.h"
#include "ui.h"
#include "util.h"

#define MENU_OPT_SAVEMAP 0

static uint16_t __far ui_opt_lks[] = {
    LK_UI_SETTINGS_SAVEMAP,
};
static void ui_opt_menu_draw_line(uint8_t entry_id, uint8_t y, uint8_t color) {
    ui_puts(false, 1, y, color, lang_keys[ui_opt_lks[entry_id]]);
}

void ui_settings(void) {
    uint8_t menu_list[16];
    uint8_t i = 0;
    if (driver_supports_slots()) {
        menu_list[i++] = MENU_OPT_SAVEMAP;
    }
    menu_list[i++] = MENU_ENTRY_END;

    uint8_t result = ui_menu_select(menu_list, ui_opt_menu_draw_line);
}