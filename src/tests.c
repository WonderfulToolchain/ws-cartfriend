/**
 * Copyright (c) 2023 Adrian Siekierka
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
#include "input.h"
#include "tests.h"
#include "settings.h"
#include "sram.h"
#include "ui.h"
#include "ws/hardware.h"

const char __far msg_save_read_write_error1[] = "A %02X != %02X E";

static bool test_save_read_write_error(uint16_t bank, uint16_t offset, uint8_t expected) {
    char msg[40];
    uint8_t __far *ptr = MK_FP(0x1000, offset);
    if (*ptr != expected) {
        ui_reset_main_screen();

        ui_bg_printf_centered(2, 0, lang_keys[LK_UI_ERROR], bank, offset);
        ui_bg_printf_centered(4, 0, msg_save_read_write_error1, *ptr, expected);

        input_wait_key(KEY_A);
        return true;
    } else {
        return false;
    }
}

void test_save_read_write(uint8_t slot) {
    settings_local.active_sram_slot = slot;

    // Clear SRAM
    for (int i = 0; i < 8; i++) {
        outportb(IO_BANK_RAM, i);
        memset(MK_FP(0x1000, 0x0000), 0, 16384);
        memset(MK_FP(0x1000, 0x4000), 0, 16384);
        memset(MK_FP(0x1000, 0x8000), 0, 16384);
        memset(MK_FP(0x1000, 0xC000), 0, 16384);
    }

    // Read/Write SRAM
    sram_switch_to_slot(0xFF);
    sram_switch_to_slot(slot);

    // Compare SRAM
    for (int i = 0; i < 8; i++) {
        outportb(IO_BANK_RAM, i);
        uint16_t ofs = 0;
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++, ofs++) {
                if (test_save_read_write_error(i, ofs, 0)) return;
            }
        }
    }

    // Write test pattern to SRAM
    for (int i = 0; i < 8; i++) {
        outportb(IO_BANK_RAM, i);
        uint8_t __far *ptr = MK_FP(0x1000, 0x0000);
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++, ptr++) {
                *ptr = j + k - i;
                if (test_save_read_write_error(i | 0x1000, FP_OFF(ptr), j + k - i)) return;
            }
        }
    }

    // Read/Write SRAM
    sram_switch_to_slot(0xFF);
    sram_switch_to_slot(slot);

    // Compare SRAM
    for (int i = 0; i < 8; i++) {
        outportb(IO_BANK_RAM, i);
        uint16_t ofs = 0;
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++, ofs++) {
                if (test_save_read_write_error(i, ofs, j + k - i)) return;
            }
        }
    }

    // Write test pattern to SRAM
    for (int i = 0; i < 8; i++) {
        outportb(IO_BANK_RAM, i);
        uint8_t __far *ptr = MK_FP(0x1000, 0x0000);
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++, ptr++) {
                *ptr = k < j ? 0xFF : j;
                if (test_save_read_write_error(i | 0x1000, FP_OFF(ptr), k < j ? 0xFF : j)) return;
            }
        }
    }

    // Read/Write SRAM
    sram_switch_to_slot(0xFF);
    sram_switch_to_slot(slot);

    // Compare SRAM
    for (int i = 0; i < 8; i++) {
        outportb(IO_BANK_RAM, i);
        uint16_t ofs = 0;
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++, ofs++) {
                if (test_save_read_write_error(i, ofs, k < j ? 0xFF : j)) return;
            }
        }
    }

    settings_local.active_sram_slot = 0xFF;
    outportb(IO_BANK_RAM, 0);
}
