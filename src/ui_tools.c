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
#include <wonderful-asm.h>
#include <ws.h>
#include "driver.h"
#include "lang.h"
#include "ui.h"
#include "util.h"
#include "xmodem.h"

#define MENU_TOOL_SRAMCODE_XM 0

static uint16_t __far ui_tool_lks[] = {
    LK_UI_TOOLS_SRAMCODE_XM
};
static void ui_tool_menu_draw_line(uint8_t entry_id, uint8_t y, uint8_t color) {
    ui_puts(false, 1, y, color, lang_keys[ui_tool_lks[entry_id]]);
}

static void ui_tool_sramcode_xm() {
    ui_reset_main_screen();
    ui_puts_centered(false, 2, 0, lang_keys[LK_UI_XMODEM_RECEIVE]);
    ui_puts_centered(false, 3, 0, lang_keys[LK_UI_XMODEM_PRESS_B_TO_CANCEL]);

    uint8_t __far* sram_ptr = MK_FP(0x1000, 0x0000);
    uint16_t sram_incrs = 0;
    bool active = true;

    xmodem_open(SERIAL_BAUD_38400);
    if (xmodem_recv_start() == XMODEM_OK) {
        ui_fill_line(13, 0);
        ui_puts_centered(false, 13, 0, lang_keys[LK_UI_XMODEM_IN_PROGRESS]);

        while (active) {
            uint8_t result = xmodem_recv_block(sram_ptr);
            switch (result) {
                case XMODEM_COMPLETE:
                    launch_sram();
                    break;
                case XMODEM_SELF_CANCEL:
                    active = false;
                    break;
                case XMODEM_CANCEL:
                    ui_fill_line(13, 0);
                    ui_puts_centered(false, 13, 0, lang_keys[LK_UI_XMODEM_CANCEL]);
                    active = false;
                    break;
                case XMODEM_OK:
                    sram_incrs++;
                    if (sram_incrs < 512) {
                        uint8_t line = inportb(IO_LCD_LINE);
                        if (line >= 136 || line < 72) {
                            ui_fill_line(14, 0);
                            ui_printf_centered(false, 14, 0, lang_keys[LK_UI_XMODEM_RECEIVE_PROGRESS], sram_incrs * 128);
                        }
                        sram_ptr += 128;
                        ui_step_work_indicator();
                        xmodem_recv_ack();
                        break;
                    }
                    // fall through to XMODEM_ERROR
                case XMODEM_ERROR:
                    ui_fill_line(13, 0);
                    ui_puts_centered(false, 13, 0, lang_keys[LK_UI_XMODEM_ERROR]);
                    active = false;
                    break;
            }
        }
    }

    ui_clear_work_indicator();
    xmodem_close();
    while (!xmodem_poll_exit()) cpu_halt();
}

void ui_tools(void) {
    uint8_t menu_list[16];
    uint8_t i = 0;
    if ((_CS & 0xF000) != 0x1000) menu_list[i++] = MENU_TOOL_SRAMCODE_XM;
    menu_list[i++] = MENU_ENTRY_END;

    uint8_t result = ui_menu_select(menu_list, ui_tool_menu_draw_line);
    switch (result) {
        case MENU_TOOL_SRAMCODE_XM: ui_tool_sramcode_xm(); break;
    }
}