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
#include "settings.h"
#include "ui.h"
#include "../res/font_default.h"
#include "nanoprintf.h"
#include "util.h"
#include "ws/display.h"
#include "ws/system.h"

#define SCREEN1 ((uint16_t*) 0x1800)
#define SCREEN2 ((uint16_t*) 0x3800)

static uint8_t scroll_y;
const char __far* const __far* lang_keys;

static const uint16_t __far theme_colorways[UI_THEME_COUNT][6] = {
    { // Light
        RGB(0, 0, 0),
        RGB(4, 4, 4),
        RGB(6, 6, 6),
        RGB(8, 8, 8),
        RGB(11, 11, 11),
        RGB(15, 15, 15)
    },
    { // Dark
        RGB(15, 15, 15),
        RGB(11, 11, 11),
        RGB(8, 8, 8),
        RGB(6, 6, 6),
        RGB(4, 4, 4),
        RGB(0, 0, 0)
    }
};

static bool ui_dialog_open;

void ui_update_theme(uint8_t current_theme) {
    wait_for_vblank();
    if (ws_system_color_active()) {
        const uint16_t __far* colorway = theme_colorways[current_theme & 7];
        MEM_COLOR_PALETTE(0)[0] = colorway[ui_dialog_open ? 3 : 5];
        MEM_COLOR_PALETTE(0)[1] = colorway[ui_dialog_open ? 2 : 0];
        MEM_COLOR_PALETTE(1)[0] = colorway[ui_dialog_open ? 2 : 0];
        MEM_COLOR_PALETTE(1)[1] = colorway[ui_dialog_open ? 3 : 5];
        MEM_COLOR_PALETTE(2)[0] = colorway[4];
        MEM_COLOR_PALETTE(2)[1] = colorway[1];
        MEM_COLOR_PALETTE(3)[0] = colorway[1];
        MEM_COLOR_PALETTE(3)[1] = colorway[4];
        MEM_COLOR_PALETTE(8)[0] = colorway[5];
        MEM_COLOR_PALETTE(8)[1] = colorway[0];
        MEM_COLOR_PALETTE(9)[0] = colorway[0];
        MEM_COLOR_PALETTE(9)[1] = colorway[5];
        MEM_COLOR_PALETTE(10)[0] = colorway[5];
        MEM_COLOR_PALETTE(10)[1] = RGB(15, 0, 0);
    } else {
        if (current_theme & 0x80) {
            // dark mode
            ws_display_set_shade_lut(SHADE_LUT(15, 13, 11, 8, 6, 4, 2, 0));
        } else {
            // light mode
            ws_display_set_shade_lut(SHADE_LUT(0, 2, 4, 6, 8, 11, 13, 15));
        }
        if (ui_dialog_open) {
            outportw(0x20, 4 << 4 | 3);
            outportw(0x22, 3 << 4 | 4);
        } else {
            outportw(0x20, 7 << 4);
            outportw(0x22, 7);
        }
        outportw(0x24, 5 << 4 | 2);
        outportw(0x26, 2 << 4 | 5);
        outportw(0x30, 7 << 4);
        outportw(0x32, 7);
        outportw(0x34, 4 << 4);
    }
}

void ui_reset_alt_screen(void) {
    for (uint16_t i = 0; i < 32*16; i++) {
        SCREEN2[i + 32] = SCR_ENTRY_PALETTE(7);
    }
}

void ui_init(void) {
    lang_keys = lang_keys_en;

    outportw(IO_DISPLAY_CTRL, 0);
    outportb(IO_SPR_FIRST, 0);
    outportb(IO_SPR_COUNT, 0);

    outportb(IO_SCR2_SCRL_X, 0);
    outportb(IO_SCR2_SCRL_Y, 0);

    // set palettes (mono)
    if (ws_system_is_color()) {
        ws_mode_set(WS_MODE_COLOR);
    }
    ui_dialog_open = false;
    ui_update_theme(0);

    // install font @ 0x2000
    const uint8_t __far *font_src = _font_default_bin;
    uint16_t *font_dst = (uint16_t*) 0x2000;
    for (uint16_t i = 0; i < _font_default_bin_size; i++, font_src++, font_dst++) {
        *font_dst = *font_src;
    }

    ui_reset_main_screen();
    ui_reset_alt_screen();

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
    outportb(IO_SCR1_SCRL_X, 4);
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
    for (uint8_t i = 0; i < 29; i++) {
        *(screen++) = prefix;
    }
}

void ui_puts(bool alt_screen, uint8_t x, uint8_t y, uint8_t color, const char __far* buf) {
    uint16_t prefix = SCR_ENTRY_PALETTE(color);
    uint16_t *screen = alt_screen ? SCREEN2 : SCREEN1;
    while (*buf != '\0') {
        ws_screen_put(screen, prefix | ((uint8_t) *(buf++)), x++, y);
        if (x == 29) return;
    }
}

void ui_puts_centered(bool alt_screen, uint8_t y, uint8_t color, const char __far* buf) {
    uint8_t x = ((alt_screen ? 28 : 29) - strlen(buf)) >> 1;
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
    uint8_t x = ((alt_screen ? 28 : 29) - strlen(buf)) >> 1;
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

static void ui_menu_draw_line(ui_menu_state_t *menu, uint8_t pos, uint8_t color) {
    if (menu->list[pos] == MENU_ENTRY_DIVIDER) {
        ws_screen_fill(SCREEN1, 196, 0, pos, 29, 1);
        return;
    }

    char buf[31];
    char buf_right[31];
    buf[0] = 0; buf_right[0] = 0;

    menu->build_line_func(menu->list[pos], buf, 30, buf_right, 30);
    if (buf[0] != 0) {
        ui_puts(false, 1, pos, color, buf);
    }
    if (buf_right[0] != 0) {
        ui_puts(false, 28 - strlen(buf_right), pos, color, buf_right);
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
    int last_pos;
    int new_pos = menu->pos;
    do {
        last_pos = new_pos;
        new_pos = new_pos + delta;
        if (new_pos < 0) new_pos = 0;
        else if (new_pos >= menu->height) new_pos = menu->height - 1;
    } while (last_pos != new_pos && (menu->list[new_pos] == MENU_ENTRY_DIVIDER));

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
    menu->height = u8_arraylist_len(menu->list);
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
        ui_putc(false, x, state->y, step_current + UI_GLYPH_HORIZONTAL_PBAR, UI_PAL_PBAR);
    }
}

// Dialog

static uint8_t sep_count_lines(const char __far* s, uint8_t *max_width) {
    uint8_t line_width = 0;
    uint8_t line_count = 1;
    while (*s != '\0') {
        if (*s == '|') {
            if (line_width > *max_width) {
                *max_width = line_width;
            }
            line_width = 0;
            line_count++;
        } else {
            line_width++;
        }
        s++;
    }
    if (line_width > *max_width) {
        *max_width = line_width;
    }
    return line_count;
}

static void sep_draw(const char __far* s, uint8_t x, uint8_t y, uint8_t highlight_line, bool left_aligned) {
    char buf[29];
    highlight_line += y;

    uint8_t i = 0;
    while (*s != '\0') {
        if (*s == '|') {
            buf[i] = 0;
            if (left_aligned) {
                ui_puts(true, x, y, y == highlight_line ? 9 : 8, buf);
            } else {
                ui_puts_centered(true, y, y == highlight_line ? 9 : 8, buf);
            }
            i = 0;
            y++;
        } else {
            buf[i++] = *s;
        }
        s++;
    }

    buf[i] = 0;
    if (left_aligned) {
        ui_puts(true, x, y, y == highlight_line ? 9 : 8, buf);
    } else {
        ui_puts_centered(true, y, y == highlight_line ? 9 : 8, buf);
    }
}

uint8_t ui_dialog_run(uint16_t flags, uint8_t initial_option, uint16_t lk_question, uint16_t lk_options) {
    wait_for_vblank();
    ui_dialog_open = true;
    ui_update_theme(settings_local.color_theme);

    uint8_t max_line_width = 0;
    uint8_t line_count = sep_count_lines(lang_keys[lk_question], &max_line_width);
    uint8_t option_count = sep_count_lines(lang_keys[lk_options], &max_line_width);

    uint8_t width = ((max_line_width + 2) + 1) & 0xFE;
    uint8_t height = ((line_count + option_count + 3) + 1) & 0xFE;
    uint8_t x = 14 - ((width + 1) >> 1);
    uint8_t y = 9 - ((height + 1) >> 1);
    ws_screen_fill(SCREEN2, SCR_ENTRY_PALETTE(UI_PAL_DIALOG), x, y, width, height);
    uint8_t selected_option = initial_option;

    // draw text
    sep_draw(lang_keys[lk_question], x + 1, y + 1, 0xFF, true);
    while (true) {
        sep_draw(lang_keys[lk_options], x + 1, y + height - option_count - 1, selected_option, false);

        wait_for_vblank();
        input_update();
        if (input_pressed & KEY_UP) {
            if (selected_option > 0) {
                selected_option--;
            }
        } else if (input_pressed & KEY_DOWN) {
            if (selected_option < (option_count - 1)) {
                selected_option++;
            }
        } else if (input_pressed & KEY_A) {
            break;
        } else if (input_pressed & KEY_B) {
            selected_option = 0xFF;
            break;
        }
    }

    wait_for_vblank();
    ui_dialog_open = false;
    ui_update_theme(settings_local.color_theme);
    ui_reset_alt_screen();

    return selected_option;
}