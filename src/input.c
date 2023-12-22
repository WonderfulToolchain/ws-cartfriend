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
#include "input.h"
#include "util.h"

// it's an uint16_t but we only want the low byte
extern volatile uint8_t vbl_ticks;
extern bool is_pcv2;

uint16_t input_keys = 0;
uint16_t input_keys_repressed = 0;
uint16_t input_keys_released = 0;
uint16_t input_pressed, input_held;

void vblank_input_update(void) {
	uint16_t keys = ws_keypad_scan();
	if (is_pcv2) {
		// WS:   ....yyyyxxxxbas.
		// PCv2: ....pc1Cre1vud1l
		// remapped:
		//       ...p....urldcCe.
		keys = 
			  ((keys & 0x0800) <<  1) /*p*/
			| ((keys & 0x0400) >>  7) /*c*/
			| ((keys & 0x0100) >>  6) /*C*/
			| ((keys & 0x0080) >>  1) /*r*/
			| ((keys & 0x0040) >>  5) /*e*/
			| ((keys & 0x0008) <<  4) /*u*/
			| ((keys & 0x0004) <<  2) /*d*/
			| ((keys & 0x0001) <<  5); /*l*/
	}
	input_keys |= keys;
	input_keys_repressed |= (keys & input_keys_released);
	input_keys_released |= (input_held & (~keys));
}

void input_reset(void) {
	cpu_irq_disable();
	input_keys = 0;
	input_keys_repressed = 0;
	input_keys_released = 0;
	cpu_irq_enable();
}

#define JOY_REPEAT_DELAY 15
#define JOY_REPEAT_DELAY_NEXT 3

static uint8_t input_vbls_next[11];

void input_update(void) {
	uint16_t keys_pressed;
	uint16_t keys_repressed;
	uint16_t keys_released;

	cpu_irq_disable();
	keys_pressed = input_keys;
	keys_repressed = input_keys_repressed;
	keys_released = input_keys_released;

	input_pressed = 0;
	uint16_t input_mask = 2;
	for (uint8_t i = 0; i < 11; i++, input_mask <<= 1) {
		if (keys_pressed & input_mask) {
			if (keys_repressed & input_mask) {
				goto KeyRepressed;
			} else if (input_held & input_mask) {
				if (keys_released & input_mask) {
					input_held &= ~input_mask;
				}
				if (((uint8_t) (input_vbls_next[i] - vbl_ticks)) < 0x80) continue;
				if (!(keys_released & input_mask)) {
					input_pressed |= input_mask;
					input_vbls_next[i] = vbl_ticks + JOY_REPEAT_DELAY_NEXT;
				}
			} else {
				if (!(keys_released & input_mask)) {
KeyRepressed:
					input_pressed |= input_mask;
					input_held |= input_mask;
					input_vbls_next[i] = vbl_ticks + JOY_REPEAT_DELAY;
				}
			}
			break;
		} else {
			input_held &= ~input_mask;
		}
	}

	input_reset();
}

void input_wait_clear(void) {
	do {
		input_reset();
		wait_for_vblank();
	} while (input_keys != 0);
	input_update();
}

void input_wait_key(uint16_t key) {
	input_wait_clear();
	do {
		input_reset();
		wait_for_vblank();
	} while ((input_keys & key) != key);
	input_update();
}

void input_wait_any_key(void) {
	input_wait_clear();
	do {
		input_reset();
		wait_for_vblank();
	} while (input_keys == 0);
	input_update();
}
