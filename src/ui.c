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
#include "driver.h"
#include "input.h"
#include "lang.h"
#include "ui.h"
#include "../res/font_default.h"
#include "nanoprintf.h"
#include "util.h"
#include "ws/display.h"

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
    ws_display_set_shade_lut(SHADE_LUT(0, 2, 4, 6, 8, 10, 12, 15));
    outportw(0x20, 7 << 4);
    outportw(0x22, 7);
    outportw(0x24, 5 << 4 | 2);
    outportw(0x26, 2 << 4 | 5);
    outportw(0x30, 7 << 4);
    outportw(0x32, 4 << 4);

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
    ui_puts(true, 28 - strlen(lang_keys[LK_NAME]), 17, 2, lang_keys[LK_NAME]);
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
    ws_screen_put(screen, prefix | chr, x++, y);
}

void ui_fill_line(uint8_t y, uint8_t color) {
    uint16_t prefix = SCR_ENTRY_PALETTE(color);
    uint16_t *screen = SCREEN1 + (y << 5);
    for (uint8_t i = 0; i < 28; i++) {
        *(screen++) = prefix;
    }
}

void ui_puts(bool alt_screen, uint8_t x, uint8_t y, uint8_t color, const char __far* buf) {
    uint16_t prefix = SCR_ENTRY_PALETTE(color);
    uint16_t *screen = alt_screen ? SCREEN2 : SCREEN1;
    while (*buf != '\0') {
        ws_screen_put(screen, prefix | ((uint8_t) *(buf++)), x++, y);
        if (x == 28) return;
    }
}

void ui_puts_centered(bool alt_screen, uint8_t y, uint8_t color, const char __far* buf) {
    uint8_t x = 14 - (strlen(buf) >> 1);
    ui_puts(alt_screen, x, y, color, buf);
}

void ui_printf(bool alt_screen, uint8_t x, uint8_t y, uint8_t color, const char __far* format, ...) {
    char buf[33];
    va_list val;
    va_start(val, format);
    npf_vsnprintf(buf, sizeof(buf), format, val);
    va_end(val);
    ui_puts(alt_screen, x, y, color, buf);
}

void ui_printf_centered(bool alt_screen, uint8_t y, uint8_t color, const char __far* format, ...) {
    char buf[33];
    va_list val;
    va_start(val, format);
    npf_vsnprintf(buf, sizeof(buf), format, val);
    va_end(val);
    uint8_t x = 14 - (strlen(buf) >> 1);
    ui_puts(alt_screen, x, y, color, buf);
}

void ui_printf_right(bool alt_screen, uint8_t x, uint8_t y, uint8_t color, const char __far* format, ...) {
    char buf[33];
    va_list val;
    va_start(val, format);
    int len = npf_vsnprintf(buf, sizeof(buf), format, val);
    va_end(val);
    ui_puts(alt_screen, x + 1 - len, y, color, buf);
}

// Tabs

static uint16_t __far ui_tabs_to_lks[] = {
    LK_UI_HEADER_BROWSE,
    LK_UI_HEADER_TOOLS,
    LK_UI_HEADER_SETTINGS,
    LK_UI_HEADER_ABOUT,
    LK_TOTAL
};
uint8_t ui_current_tab;

static bool ui_hack_show_browse_tab(void) {
    return driver_supports_slots();
}

void ui_set_current_tab(uint8_t tab) {
    ui_current_tab = tab;
    if (tab == 0 && !ui_hack_show_browse_tab()) {
        ui_current_tab = 1;
        tab = 1;
    }

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
        ui_putc(true, 26, 0, 0, 2);
        ui_putc(true, 27, 0, UI_GLYPH_ARROW_RIGHT, 2);
    }

    if (ui_current_tab > 1 || (ui_current_tab == 1 && ui_hack_show_browse_tab())) {
        ui_putc(true, 25, 0, 0, 2);
        ui_putc(true, 26, 0, UI_GLYPH_ARROW_LEFT, 2);
        if (finished) {
            ui_putc(true, 27, 0, 0, 2);
        }
    }
}

bool ui_poll_events(void) {
    input_update();

    if (input_pressed & KEY_ALEFT) {
        if (ui_current_tab > 0) {
            ui_set_current_tab(ui_current_tab - 1);
            wait_for_vblank();
            return false;
        }
    } else if (input_pressed & KEY_ARIGHT) {
        if (ui_current_tab < (UI_TAB_TOTAL - 1)) {
            ui_set_current_tab(ui_current_tab + 1);
            wait_for_vblank();
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

static int ui_menu_len(uint8_t *menu_list) {
    int i = 0;
    while (*(menu_list++) != MENU_ENTRY_END) {
        i++;
    }
    return i;
}

static void ui_menu_draw_line(ui_menu_state_t *menu, uint8_t pos, uint8_t color) {
    char buf[31];
    char buf_right[31];
    buf[0] = 0; buf_right[0] = 0;

    menu->build_line_func(menu->list[pos], buf, 30, buf_right, 30);
    if (buf[0] != 0) {
        ui_puts(false, 1, pos, color, buf);
    }
    if (buf_right[0] != 0) {
        ui_puts(false, 27 - strlen(buf_right), pos, color, buf_right);
    }
}

static void ui_menu_redraw(ui_menu_state_t *menu) {
    if (menu->height > 0) {
        for (uint8_t i = 0; i < 16; i++) {
            if (i >= menu->height) break;
            ui_menu_draw_line(menu, menu->y + i, 0);
        }
        ui_fill_line(menu->pos, 1);
        ui_menu_draw_line(menu, menu->pos, 1);
    }
}

static void ui_menu_move(ui_menu_state_t *menu, int8_t delta) {
    int new_pos = menu->pos + delta;
    if (new_pos < 0) new_pos = 0;
    else if (new_pos >= menu->height) new_pos = menu->height - 1;
    if (new_pos == menu->pos) return;

    // draw lines
    ui_fill_line(menu->pos, 0);
    ui_menu_draw_line(menu, menu->pos, 0);
    menu->pos = new_pos;
    ui_fill_line(menu->pos, 1);
    ui_menu_draw_line(menu, menu->pos, 1);

    // adjust scroll
    int new_y = menu->pos - 8;
    if (new_y < 0) new_y = 0;
    else if (new_y >= menu->y_max) new_y = menu->y_max;
    int scroll_delta = new_y - menu->y;
    if (scroll_delta != 0) {
        ui_scroll(scroll_delta);
        menu->y = new_y;

        // TODO: optimize
        ui_menu_redraw(menu);
    }
}

void ui_menu_init(ui_menu_state_t *menu) {
    menu->height = ui_menu_len(menu->list);
    menu->pos = 0;
    menu->y = 0;
    menu->y_max = menu->height - 16;
    if (menu->y_max > menu->height) menu->y_max = 0;
}

uint16_t ui_menu_select(ui_menu_state_t *menu) {
    ui_clear_work_indicator();
    ui_menu_redraw(menu);

    while (ui_poll_events()) {
        if (input_pressed & KEY_UP) {
            ui_menu_move(menu, -1);
        }
        if (input_pressed & KEY_DOWN) {
            ui_menu_move(menu, 1);
        }
        wait_for_vblank();
        uint8_t curr_entry = menu->list[menu->pos];
        if (menu->flags & MENU_SEND_LEFT_RIGHT) {
            if (input_pressed & KEY_LEFT) {
                return curr_entry | MENU_ACTION_LEFT;
            }
            if (input_pressed & KEY_RIGHT) {
                return curr_entry | MENU_ACTION_RIGHT;
            }
        }
        if (menu->flags & MENU_B_AS_BACK) {
            if (input_pressed & KEY_B) {
                return MENU_ENTRY_END;
            }
        }
        if (input_pressed & KEY_A) {
            return curr_entry;
        }
    }

    return MENU_ENTRY_END;
}

// Progress bar

void ui_pbar_init(ui_pbar_state_t *state) {
    state->step = 0;
}

// TODO: Optimize (draw only changed steps)
void ui_pbar_draw(ui_pbar_state_t *state) {
    uint16_t step_count = state->width * 8;
    uint16_t step_current = (((uint32_t) state->step) * step_count) / state->step_max;
    uint8_t i = 0;
    uint8_t x = state->x;
    for (i = 8; i <= step_current; i += 8) {
        ui_putc(false, x++, state->y, 219, UI_PAL_PBAR);
    }
    step_current &= 7;
    if (step_current > 0) {
        ui_putc(false, x, state->y, step_current + 255, UI_PAL_PBAR);
    }
}