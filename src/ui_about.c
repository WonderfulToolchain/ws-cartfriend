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
#include "lang.h"
#include "ui.h"
#include "util.h"

static const uint8_t __far fancy_pattern[] = {
    32, 176, 177, 178, 177, 176,
};

static void draw_pattern(uint8_t i) {
    for (uint8_t x = 0; x < 32; x++) {
        uint8_t p = fancy_pattern[(x + i) % sizeof(fancy_pattern)];
        ui_putc(false, x,  6, p, 0);
        if (x > 0) {
            ui_putc(false, x-1, 7, p, 0);
            if (x > 1) {
                ui_putc(false, x-2, 8, p, 0);
                if (x > 2) {
                    ui_putc(false, x-3, 9, p, 0);
                }
            }
        }
    }
}

extern uint8_t fm_initial_slot[7];

void ui_about(void) {
    ui_puts_centered(false, 2, 0, lang_keys[LK_NAME]);
    // ui_puts_centered(false, 3, lang_keys[LK_RELEASE_DATE], 0);
    /* ui_printf_centered(false, 3, 0, lang_keys[LK_UI_LOADED_FROM],
        (int) fm_initial_slot[0],
        (int) fm_initial_slot[1],
        (int) fm_initial_slot[2],
        (int) fm_initial_slot[3]); */
    ui_puts_centered(false, 12, 0, lang_keys[LK_UI_ABOUT_URL_LINE1]);
    ui_puts_centered(false, 13, 0, lang_keys[LK_UI_ABOUT_URL_LINE2]);

    uint16_t i = 0;
    draw_pattern(i);
    while (ui_poll_events()) {
        wait_for_vblank();
        i++;
        if ((i & 7) == 0) {
            draw_pattern(i >> 3);
        }
    }
}