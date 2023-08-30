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
#include <wonderful.h>
#include <ws.h>

static inline void sram_enable_fast(void) {
    outportb(IO_SYSTEM_CTRL2, inportb(IO_SYSTEM_CTRL2) & (~SYSTEM_CTRL2_SRAM_WAIT));
}
static inline void sram_disable_fast(void) {
    outportb(IO_SYSTEM_CTRL2, inportb(IO_SYSTEM_CTRL2) | SYSTEM_CTRL2_SRAM_WAIT);
}
void sram_erase(uint8_t sram_slot);
void sram_switch_to_slot(uint8_t sram_slot);
