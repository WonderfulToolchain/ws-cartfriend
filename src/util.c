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

// it's an uint16_t but we only want the low byte
extern volatile uint8_t vbl_ticks;

void wait_for_vblank(void) {
        uint8_t vbl_ticks_last = vbl_ticks;

        while (vbl_ticks == vbl_ticks_last) {
                cpu_halt();
        }
}

int u8_arraylist_len(uint8_t *list) {
    int i = 0;
    while (*(list++) != 0xFF) {
        i++;
    }
    return i;
}

int u16_arraylist_len(uint16_t *list) {
    int i = 0;
    while (*(list++) != 0xFFFF) {
        i++;
    }
    return i;
}

// Adapted from http://www8.cs.umu.se/~isak/snippets/crc-16.c
// Site states that all snippets are "Public Domain or free"

#define CRC16_POLY 0x8408
#define CRC16_CHECK(mask) \
    if ((crc & (mask)) ^ (v & (mask))) { \
        crc = (crc >> 1) ^ CRC16_POLY; \
    } else { \
        crc = (crc >> 1); \ 
    }
#define CRC16_CHECK_FF(mask) \
    if (!(crc & (mask))) { \
        crc = (crc >> 1) ^ CRC16_POLY; \
    } else { \
        crc = (crc >> 1); \ 
    }

uint16_t crc16(const char *data, uint16_t len, uint16_t pad_len) {
    uint16_t crc = 0xFFFF;
    uint16_t pos;

    for (pos = 0; pos < len; pos++) {
        uint8_t v = *(data++);
        CRC16_CHECK(0x01);
        CRC16_CHECK(0x02);
        CRC16_CHECK(0x04);
        CRC16_CHECK(0x08);
        CRC16_CHECK(0x10);
        CRC16_CHECK(0x20);
        CRC16_CHECK(0x40);
        CRC16_CHECK(0x80);
    }

    for (; pos < pad_len; pos++) {
        // pad is a constant - 0xFF
        CRC16_CHECK_FF(0x01);
        CRC16_CHECK_FF(0x02);
        CRC16_CHECK_FF(0x04);
        CRC16_CHECK_FF(0x08);
        CRC16_CHECK_FF(0x10);
        CRC16_CHECK_FF(0x20);
        CRC16_CHECK_FF(0x40);
        CRC16_CHECK_FF(0x80);
    }

    crc = ~crc;
    return (crc << 8) | (crc >> 8);
}