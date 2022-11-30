#pragma once
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
#include "../res/lang.h"

#define UI_PAL_MAIN    0
#define UI_PAL_MAINI   1
#define UI_PAL_BAR     2
#define UI_PAL_BARI    3
#define UI_PAL_DIALOG  8
#define UI_PAL_DIALOGI 9
#define UI_PAL_PBAR    10

extern const char __far* const __far* lang_keys;

void ui_init(void);
void ui_show(void);
void ui_hide(void);
void ui_reset_main_screen(void);
void ui_scroll(int8_t offset);
void ui_fill_line(uint8_t y, uint8_t color);
void ui_putc(bool alt_screen, uint8_t x, uint8_t y, uint16_t chr, uint8_t color);
void ui_puts(bool alt_screen, uint8_t x, uint8_t y, uint8_t color, const char __far* buf);
void ui_puts_centered(bool alt_screen, uint8_t y, uint8_t color, const char __far* buf);
__attribute__((format(printf, 5, 6))) void ui_printf(bool alt_screen, uint8_t x, uint8_t y, uint8_t color, const char __far* format, ...);
__attribute__((format(printf, 4, 5))) void ui_printf_centered(bool alt_screen, uint8_t y, uint8_t color, const char __far* format, ...);
__attribute__((format(printf, 5, 6))) void ui_printf_right(bool alt_screen, uint8_t x, uint8_t y, uint8_t color, const char __far* format, ...);

#define UI_THEME_COUNT 2
void ui_update_theme(uint8_t current_theme);

#define UI_GLYPH_ARROW_RIGHT 16
#define UI_GLYPH_ARROW_LEFT 17
#define UI_GLYPH_TRIANGLE_UR 169
#define UI_GLYPH_TRIANGLE_UL 170
#define UI_GLYPH_HORIZONTAL_PBAR 183

// Tabs

extern uint8_t ui_current_tab;
void ui_set_current_tab(uint8_t tab);
bool ui_poll_events(void);

#define UI_TAB_BROWSE 0
#define UI_TAB_TOOLS 1
#define UI_TAB_SETTINGS 2
#define UI_TAB_ABOUT 3
#define UI_TAB_TOTAL 4

// Work indicator

void ui_step_work_indicator(void);
void ui_clear_work_indicator(void);

// Menu system

#define MENU_ENTRY_DIVIDER 254
#define MENU_ENTRY_END 255

typedef void (*ui_menu_build_line_func)(uint8_t entry_id, char *buf, int buf_len, char *buf_right, int buf_right_len);

typedef struct {
    uint8_t *list;
    ui_menu_build_line_func build_line_func;
    uint16_t flags;

    // auto-generated
    uint8_t height;
    uint8_t pos;
    uint8_t y;
    uint8_t y_max;
} ui_menu_state_t;

#define MENU_SEND_LEFT_RIGHT 0x0001
#define MENU_B_AS_BACK       0x0002

#define MENU_ACTION_LEFT  0x0100
#define MENU_ACTION_RIGHT 0x0200

void ui_menu_init(ui_menu_state_t *menu);
uint16_t ui_menu_select(ui_menu_state_t *menu);

// Progress bars

typedef struct {
    uint16_t step, step_max;
    uint8_t x, y, width;
} ui_pbar_state_t;

void ui_pbar_init(ui_pbar_state_t *state);
void ui_pbar_draw(ui_pbar_state_t *state);

// Dialogs

uint8_t ui_dialog_run(uint16_t flags, uint8_t initial_option, uint16_t lk_question, uint16_t lk_options);

// Tab implementations

void ui_about(void); // ui_about.c
void ui_browse(void); // ui_browse.c
void ui_settings(void); // ui_settings.c
void ui_tools(void); // ui_tools.c