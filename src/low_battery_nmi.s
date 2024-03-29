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

#include <wonderful.h>
#include "config.h"

#ifdef USE_LOW_BATTERY_WARNING
	.arch	i186
	.code16
	.intel_syntax noprefix
	.global ui_low_battery_flag
	.global lowbat_nmi_handler

	.section data
	.align 2
lowbat_nmi_handler:
	push ax
	xor ax, ax
	out 0xB7, al
	inc ax
	mov byte ptr [ui_low_battery_flag], al
	pop ax
	iret

ui_low_battery_flag:
	.byte 0
#endif
