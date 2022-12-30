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
#include <ws.h>
#include "input.h"
#include "lang.h"
#include "settings.h"
#include "ui.h"
#include "util.h"
#include "ws/display.h"

extern bool ui_dialog_open;

#define OSK_EN_WIDTH 12
#define OSK_EN_HEIGHT 4

static const uint8_t __far osk_en1[OSK_EN_HEIGHT * OSK_EN_WIDTH] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '/',';', '\'',
     25, 'z', 'x', 'c', 'v', 'b', 'n', 'm', '`', ',', '.', ' '
};
static const uint8_t __far osk_en2[OSK_EN_HEIGHT * OSK_EN_WIDTH] = {
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '?',':', '"',
     24, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '~', '<', '>', ' '
};

#define OSK_IEEP_WIDTH 11
#define OSK_IEEP_HEIGHT 4

static const uint8_t __far osk_ieep[OSK_IEEP_HEIGHT * OSK_IEEP_WIDTH] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+', 
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '-',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '?', '.',
    ' ', 'Z', 'X', 'C', 'V', 'B', 'N', 'M',  3,  13,  ' '
};

typedef struct {
    uint16_t flags;
    char *buf;
    const uint8_t __far* layout;
    uint8_t buf_width;
    uint8_t width, height;
    uint8_t xt, xb, y;
    uint8_t osk_x, osk_y;
} ui_osk_state_t;

#define OSK_DRAW_TEXT 0x01
#define OSK_DRAW_BUTTONS 0x02
#define OSK_DRAW_ALL 0xFF

void ui_osk_draw(ui_osk_state_t *osk, uint8_t what) {
    if (what & OSK_DRAW_TEXT) {
        char *buf_ptr = osk->buf;
        bool first_null = false;
        for (uint8_t i = 0; i < osk->buf_width; i++) {
            if (*buf_ptr == '\0') {
                uint8_t c = ' ';
                if (first_null == false) {
                    first_null = true;
                    c = 219;
                }  
                ui_putc(true, osk->xt + i, osk->y + 1, c, UI_PAL_DIALOG);
            } else {
                ui_putc(true, osk->xt + i, osk->y + 1, *(buf_ptr++), UI_PAL_DIALOG);
            }
        }
    }
    
    if (what & OSK_DRAW_BUTTONS) {
        const uint8_t __far* layout_ptr = osk->layout;
        for (uint8_t oy = 0; oy < osk->height; oy++) {
            for (uint8_t ox = 0; ox < osk->width; ox++, layout_ptr++) {
                bool active = (ox == osk->osk_x) && (oy == osk->osk_y);
                uint16_t c = (uint8_t) (*layout_ptr);
                if (c == 32) c = UI_GLYPH_SPACE_BAR_ICON;
                ui_putc(true, osk->xb + ox * 2, osk->y + 3 + oy * 2, c, active ? UI_PAL_DIALOGI : UI_PAL_DIALOG);
            }
        }
        {
            bool active = osk->osk_x == 0 && osk->osk_y == 0xFF;
            ui_puts(true, osk->xb, osk->y + 3 + osk->height * 2, active ? UI_PAL_DIALOGI : UI_PAL_DIALOG, lang_keys[LK_DIALOG_OK]);
        }
        {
            bool active = osk->osk_x == 1 && osk->osk_y == 0xFF;
            ui_puts(true, osk->xb + (osk->width * 2 - 1) - strlen(lang_keys[LK_DIALOG_CANCEL]),
                osk->y + 3 + osk->height * 2, active ? UI_PAL_DIALOGI : UI_PAL_DIALOG, lang_keys[LK_DIALOG_CANCEL]);
        }
    }
}

bool ui_osk_run(uint16_t flags, char *buf, uint8_t buf_width) {
    wait_for_vblank();
    ui_dialog_open = true;
    ui_update_theme(settings_local.color_theme);

    ui_osk_state_t osk = {
        .flags = flags,
        .buf = buf,
        .buf_width = buf_width,
        .osk_x = 0,
        .osk_y = 3
    };

    if ((flags & UI_OSK_LAYOUT_MASK) == UI_OSK_LAYOUT_IEEP) {
        osk.layout = osk_ieep;
        osk.width = OSK_IEEP_WIDTH;
        osk.height = OSK_IEEP_HEIGHT;
    } else {
        osk.layout = osk_en1;
        osk.width = OSK_EN_WIDTH;
        osk.height = OSK_EN_HEIGHT;
    }

    uint8_t width = ((buf_width > (osk.width * 2 - 1)) ? buf_width : (osk.width * 2 - 1)) + 4;
    uint8_t height = ((osk.height * 2) - 1) + 6;
    uint8_t x = 14 - ((width + 1) >> 1);
    osk.xt = (x + ((width - buf_width) >> 1));
    osk.xb = (x + ((width - (osk.width * 2 - 1)) >> 1));
    osk.y = 9 - ((height + 1) >> 1);
    ws_screen_fill(SCREEN2, SCR_ENTRY_PALETTE(UI_PAL_DIALOG), x, osk.y, width, height);

    // pre-draw some elements
    ui_putc(true, osk.xt - 1, osk.y + 1, '[', UI_PAL_DIALOG);
    ui_putc(true, osk.xt + buf_width, osk.y + 1, ']', UI_PAL_DIALOG);

    uint8_t to_draw = OSK_DRAW_ALL;
    bool result = false;
    while (true) {
        ui_osk_draw(&osk, to_draw);
        to_draw = 0;

        wait_for_vblank();
        input_update();
        if (input_pressed & KEY_UP) {
            if (osk.osk_y == 0) {
                osk.osk_x = osk.osk_x >= (osk.width >> 1) ? 1 : 0;
                osk.osk_y = 0xFF;
            } else if (osk.osk_y == 0xFF) {
                osk.osk_x = osk.osk_x ? osk.width - 1 : 0;
                osk.osk_y = osk.height - 1;
            } else {
                osk.osk_y--;
            }
            to_draw |= OSK_DRAW_BUTTONS;
        }
        if (input_pressed & KEY_DOWN) {
            if (osk.osk_y == osk.height - 1) {
                osk.osk_x = osk.osk_x >= (osk.width >> 1) ? 1 : 0;
                osk.osk_y = 0xFF;
            } else if (osk.osk_y == 0xFF) {
                osk.osk_x = osk.osk_x ? osk.width - 1 : 0;
                osk.osk_y = 0;
            } else {
                osk.osk_y++;
            }
            to_draw |= OSK_DRAW_BUTTONS;
        }
        if (input_pressed & KEY_LEFT) {
            osk.osk_x--;
            if (osk.osk_x == 0xFF) {
                osk.osk_x = (osk.osk_y == 0xFF) ? 1 : (osk.width - 1);
            }
            to_draw |= OSK_DRAW_BUTTONS;
        }
        if (input_pressed & KEY_RIGHT) {
            uint8_t xmax = (osk.osk_y == 0xFF) ? 1 : (osk.width - 1);
            if (osk.osk_x == xmax) {
                osk.osk_x = 0;
            } else {
                osk.osk_x++;
            }
            to_draw |= OSK_DRAW_BUTTONS;
        }
        if (input_pressed & KEY_AUP) {
            if (osk.osk_y == 0xFF) {
                osk.osk_x = osk.osk_x ? osk.width - 1 : 0;
            }
            osk.osk_y = 0;
            to_draw |= OSK_DRAW_BUTTONS;
        }
        if (input_pressed & KEY_ADOWN) {
            if (osk.osk_y != 0xFF) {
                osk.osk_x = osk.osk_x >= (osk.width >> 1) ? 1 : 0;
            }
            osk.osk_y = 0xFF;
            to_draw |= OSK_DRAW_BUTTONS;
        }
        if (input_pressed & KEY_ALEFT) {
            osk.osk_x = 0;
            to_draw |= OSK_DRAW_BUTTONS;
        }
        if (input_pressed & KEY_ARIGHT) {
            osk.osk_x = (osk.osk_y == 0xFF) ? 1 : (osk.width - 1);
            to_draw |= OSK_DRAW_BUTTONS;
        }
        if (osk.osk_y == 0xFF) {
            if (input_pressed & KEY_B) {
                osk.osk_x = 1;
                to_draw |= OSK_DRAW_BUTTONS;
            } else if (input_pressed & KEY_A) {
                result = (osk.osk_x == 0);
                break;
            }
        } else {
            uint8_t curr_key = osk.layout[osk.osk_y * osk.width + osk.osk_x];;
            if ((input_pressed & KEY_A)) {
                if (curr_key == 25) {
                    osk.layout = osk_en2;
                    to_draw |= OSK_DRAW_BUTTONS;
                } else if (curr_key == 24) {
                    osk.layout = osk_en1;
                    to_draw |= OSK_DRAW_BUTTONS;
                }
            }
            if (input_pressed & (KEY_A | KEY_B)) {
                // find end of string
                char *ptr = buf;
                uint8_t i = 0;
                for (i = 0; i < buf_width; i++, ptr++) {
                    if ((*ptr) == '\0') break;
                }
                if (i > 0 && (input_pressed & KEY_B)) {
                    ptr--; i--;
                    *ptr = '\0';
                    to_draw |= OSK_DRAW_TEXT;
                }
                if (i < buf_width && (curr_key >= 32) && (curr_key <= 126) && (input_pressed & KEY_A)) {
                    *ptr = curr_key;
                    ptr++, i++;
                    if (i < buf_width) *ptr = '\0';
                    to_draw |= OSK_DRAW_TEXT;
                }
            }
        }
    }

    wait_for_vblank();
    ui_reset_alt_screen();

    input_wait_clear();
    wait_for_vblank();

    ui_dialog_open = false;
    ui_update_theme(settings_local.color_theme);
    return result;
}
