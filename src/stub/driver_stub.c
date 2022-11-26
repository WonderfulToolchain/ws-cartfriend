/**
 * CartFriend platform drivers + headers
 *
 * Copyright (c) 2022 Adrian "asie" Siekierka
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <string.h>
#include "../driver.h"
#include "wonderful-asm.h"

uint8_t fm_initial_slot; // TODO: remove

void driver_init(void) {
    
}

void driver_lock(void) {

}

void driver_unlock(void) {
    
}

uint8_t driver_get_launch_slot(void) {
    return 0;
}

bool driver_read_slot(void *ptr, uint16_t slot, uint16_t bank, uint16_t offset, uint16_t len) {
    outportb(0xC3, bank);
    const void __far* src = MK_FP(0x3000, offset);
    memcpy(ptr, src, len);
    return true;
}

bool driver_write_slot(const void *data, uint16_t slot, uint16_t bank, uint16_t offset, uint16_t len) {
    return false;
}

bool driver_erase_slot_bank(uint16_t slot, uint16_t bank) {
    return false;
}

void driver_launch_slot(uint16_t unused, uint16_t slot, uint16_t bank) {
    
}