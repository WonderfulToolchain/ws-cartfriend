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

#include <ws.h>
#include "driver.h"
#include "util.h"
#include "wonderful-asm-common.h"
#include "ws/hardware.h"

static void clear_registers(void) {
    // wait for vblank, disable display, reset some registers
    wait_for_vblank();
    cpu_irq_disable();
    outportw(IO_DISPLAY_CTRL, 0);
    outportb(IO_SPR_BASE, 0);
    outportb(IO_SPR_FIRST, 0);
    outportb(IO_SPR_COUNT, 0);
    outportb(IO_SCR1_SCRL_X, 0);
    outportb(IO_SCR1_SCRL_Y, 0);
    outportb(IO_SCR2_SCRL_X, 0);
    outportb(IO_SCR2_SCRL_Y, 0);
    outportb(IO_SCR2_SCRL_Y, 0);
    outportb(IO_HWINT_VECTOR, 0);
    outportb(IO_HWINT_ENABLE, 0);
}

void launch_slot(uint16_t slot, uint16_t bank) {
    driver_unlock();
    clear_registers();
    driver_launch_slot(0, slot, bank);
}

extern void launch_sram_asm(void);

void launch_sram(void) {
    clear_registers();
    launch_sram_asm();
}