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
#include "ui.h"
#include "../res/font_default.h"
#include "nanoprintf.h"
#include "util.h"

#define SCREEN1 ((uint16_t*) 0x1800)
#define SCREEN2 ((uint16_t*) 0x3800)

static uint8_t scroll_y;
const char __far* const __far* lang_keys;

void ui_init(void) {
    lang_keys = lang_keys_en;

    outportw(IO_DISPLAY_CTRL, 0);
    outportb(IO_SPR_FIRST, 0);
    outportb(IO_SPR_COUNT, 0);

    outportb(IO_SCR2_SCRL_X, 0);
    outportb(IO_SCR2_SCRL_Y, 0);

    // set palettes (mono)
    video_shade_lut_set(GRAY_LUT(0, 2, 4, 6, 8, 10, 12, 15));
    outportw(0x20, 7 << 4);
    outportw(0x22, 7);
    outportw(0x24, 5 << 4 | 2);
    outportw(0x26, 2 << 4 | 5);
    outportw(0x30, 7 << 4);
    outportw(0x32, 7);
    outportw(0x34, 5 << 4 | 2);
    outportw(0x36, 2 << 4 | 5);

    // install font @ 0x2000
    const uint8_t __far *font_src = _font_default_bin;
    uint16_t *font_dst = (uint16_t*) 0x2000;
    for (uint16_t i = 0; i < _font_default_bin_size; i++, font_src++, font_dst++) {
        *font_dst = *font_src;
    }

    ui_reset_main_screen();

    for (uint16_t i = 0; i < 32*16; i++) {
        SCREEN2[i + 32] = SCR_ENTRY_PALETTE(7);
    }
    for (uint16_t i = 0; i < 28; i++) {
        SCREEN2[i] = SCR_ENTRY_PALETTE(2);
        SCREEN2[i + (17 << 5)] = SCR_ENTRY_PALETTE(2);
    }

    outportb(IO_SCR_BASE, SCR1_BASE((uint16_t) SCREEN1) | SCR2_BASE((uint16_t) SCREEN2));
    ui_puts(true, 28 - strlen(lang_keys[LK_NAME]), 17, lang_keys[LK_NAME], 2);
}

void ui_show(void) {
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE | DISPLAY_SCR2_ENABLE);
}

void ui_hide(void) {
    outportw(IO_DISPLAY_CTRL, 0);
}

void ui_reset_main_screen(void) {
    scroll_y = 0;
    outportb(IO_SCR1_SCRL_X, 0);
    outportb(IO_SCR1_SCRL_Y, 248);
    memset(SCREEN1, 0, 0x800);
}

void ui_scroll(int8_t offset) {
    scroll_y = (scroll_y + offset) & 31;
    outportb(IO_SCR1_SCRL_Y, (scroll_y - 1) << 3);
}

void ui_putc(bool alt_screen, uint8_t x, uint8_t y, uint16_t chr, uint8_t color) {
    uint16_t prefix = SCR_ENTRY_PALETTE(color);
    uint16_t *screen = alt_screen ? SCREEN2 : SCREEN1;
    video_screen_put(screen, prefix | chr, x++, y);
}

void ui_fill_line(uint8_t y, uint8_t color) {
    uint16_t prefix = SCR_ENTRY_PALETTE(color);
    uint16_t *screen = SCREEN1 + (y << 5);
    for (uint8_t i = 0; i < 28; i++) {
        *(screen++) = prefix;
    }
}

void ui_puts(bool alt_screen, uint8_t x, uint8_t y, const char __far* buf, uint8_t color) {
    uint16_t prefix = SCR_ENTRY_PALETTE(color);
    uint16_t *screen = alt_screen ? SCREEN2 : SCREEN1;
    while (*buf != '\0') {
        video_screen_put(screen, prefix | *(buf++), x++, y);
        if (x == 28) return;
    }
}

void ui_puts_centered(bool alt_screen, uint8_t y, const char __far* buf, uint8_t color) {
    uint8_t x = 14 - (strlen(buf) >> 1);
    ui_puts(alt_screen, x, y, buf, color);
}

void ui_printf(bool alt_screen, uint8_t x, uint8_t y, uint8_t color, const char __far* format, ...) {
    char buf[33];
    va_list val;
    va_start(val, format);
    npf_vsnprintf(buf, sizeof(buf), format, val);
    va_end(val);
    ui_puts(alt_screen, x, y, buf, color);
}

void ui_printf_centered(bool alt_screen, uint8_t y, uint8_t color, const char __far* format, ...) {
    char buf[33];
    va_list val;
    va_start(val, format);
    npf_vsnprintf(buf, sizeof(buf), format, val);
    va_end(val);
    uint8_t x = 14 - (strlen(buf) >> 1);
    ui_puts(alt_screen, x, y, buf, color);
}

// Tabs

static uint16_t __far ui_tabs_to_lks[] = {
    LK_UI_HEADER_BROWSE,
    LK_UI_HEADER_ABOUT,
    LK_TOTAL
};
uint8_t ui_current_tab;

void ui_set_current_tab(uint8_t tab) {
    ui_current_tab = tab;

    uint8_t x = 0;
    bool active = true;
    const char __far* text = lang_keys[ui_tabs_to_lks[tab]];
    bool finished = false;

    while (x < 28) {
        if (text != NULL && ((*text) != 0)) {
            ui_putc(true, x++, 0, *(text++), active ? 3 : 2);
        } else if (finished) {
            ui_putc(true, x++, 0, 0, 2);
        } else if (*text == 0) {
            ui_putc(true, x++, 0,
                active ? UI_GLYPH_TRIANGLE_UR : 0,
                2
            );
            ui_putc(true, x++, 0, 0, 2);

            active = false;
            uint16_t lk = ui_tabs_to_lks[++tab];
            if (lk == LK_TOTAL) {
                finished = true;
            } else {
                text = lang_keys[lk];
            }
        } else {
            finished = true;
        }
    }

    if (!finished) {
        ui_putc(true, 27, 0, UI_GLYPH_ARROW_RIGHT, 2);
    }
}

bool ui_poll_events(void) {
    input_update();

    if (input_pressed & KEY_ALEFT) {
        if (ui_current_tab > 0) {
            ui_set_current_tab(ui_current_tab - 1);
            return false;
        }
    } else if (input_pressed & KEY_ARIGHT) {
        if (ui_current_tab < (UI_TAB_TOTAL - 1)) {
            ui_set_current_tab(ui_current_tab + 1);
            return false;
        }
    }

    return true;
}

// Work indicator

extern uint8_t vbl_ticks;
uint8_t ui_work_indicator;
uint8_t ui_work_indicator_vbl_ticks;
static const uint8_t __far ui_work_table[] = {
    'q', 'd', 'b', 'p'
};

void ui_step_work_indicator(void) {
    if (ui_work_indicator_vbl_ticks == 0xFF || (ui_work_indicator_vbl_ticks != (vbl_ticks >> 2))) {
        ui_putc(true, 0, 17, ui_work_table[ui_work_indicator], 2);
        ui_work_indicator = (ui_work_indicator + 1) & 3;
        ui_work_indicator_vbl_ticks = vbl_ticks >> 2;
    }
}

void ui_clear_work_indicator(void) {
    ui_work_indicator = 0;
    ui_work_indicator_vbl_ticks = 0xFF;
    ui_putc(true, 0, 17, ' ', 2);
}

// Menu system

typedef struct {
    uint8_t *list;
    ui_menu_draw_line_func draw_line_func;
    int height;
    int pos;
    int y;
    int y_max;
} ui_menu_state_t;

static int ui_menu_len(uint8_t *menu_list) {
    int i = 0;
    while (*(menu_list++) != MENU_ENTRY_END) {
        i++;
    }
    return i;
}

static void ui_menu_move(ui_menu_state_t *menu, int8_t delta) {
    int new_pos = menu->pos + delta;
    if (new_pos < 0) new_pos = 0;
    else if (new_pos >= menu->height) new_pos = menu->height - 1;
    if (new_pos == menu->pos) return;

    // draw lines
    ui_fill_line(menu->pos, 0);
    menu->draw_line_func(menu->list[menu->pos], menu->pos, 0);
    menu->pos = new_pos;
    ui_fill_line(menu->pos, 1);
    menu->draw_line_func(menu->list[menu->pos], menu->pos, 1);

    // adjust scroll
    int new_y = menu->pos - 8;
    if (new_y < 0) new_y = 0;
    else if (new_y >= menu->y_max) new_y = menu->y_max;
    ui_scroll(new_y - menu->y);
    menu->y = new_y;
}

uint8_t ui_menu_select(uint8_t *menu_list, ui_menu_draw_line_func draw_line_func) {
    ui_menu_state_t menu;
    menu.list = menu_list;
    menu.draw_line_func = draw_line_func;
    menu.height = ui_menu_len(menu_list);
    menu.pos = 0;
    menu.y = 0;
    menu.y_max = menu.height - 16;
    if (menu.y_max < 0) menu.y_max = 0;
    
    for (uint8_t i = 0; i < 16; i++) {
        if (i >= menu.height) break;
        draw_line_func(menu_list[i], i, 0);
    }
    ui_fill_line(0, 1);
    draw_line_func(menu_list[0], 0, 1);

    while (ui_poll_events()) {
        if (input_pressed & KEY_UP) {
            ui_menu_move(&menu, -1);
        }
        if (input_pressed & KEY_DOWN) {
            ui_menu_move(&menu, 1);
        }
        wait_for_vblank();
        if (input_pressed & KEY_A) {
            return menu_list[menu.pos];
        }
    }

    return MENU_ENTRY_END;
}