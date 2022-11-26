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

#include <wonderful-asm.h>

	.arch	i186
	.code16
	.intel_syntax noprefix
	.global driver_read_slot
	.global driver_write_slot
	.global driver_erase_slot_bank
	.global driver_launch_slot
	.global fm_initial_slot

	.section .rodata
	.align 2
driver_launch_slot:
	cli

	call _driver_switch_slot

	// initialize RAM banks
	mov al, 0xFF
	out 0xC1, al
	// initialize ROM banks
	mov al, cl
	or cl, 0xF
	out 0xC2, al
	out 0xC3, al
	shr cl, 4
	or cl, 0xF0
	out 0xC0, al

	// lock Flash Masta
	mov al, 0x08
	out 0xCD, al
	xor al, al
	out 0xCC, al
	// memory clearing will take enough time
	// call _driver_change_loop

	xor ax, ax
	mov ds, ax
	mov es, ax

	// restore a few registers (ones not used later) here already
	mov	bx, [0x42]
	mov [0x42], ax
	mov	dx, [0x46]
	mov [0x46], ax

	// move relocated driver to what i think is the safest place, and jump there
	mov si, offset driver_launch_slot_relocated
	mov di, 0x2000
	mov cx, offset driver_launch_slot_end
	sub cx, si
	rep movsb

	// clear (almost) all memory we can (rep stosw in relocated part)
	mov di, 0
	mov cx, 0x1000
	jmp 0x2000

	.align 2
driver_launch_slot_relocated:
	rep stosw

	mov di, 0x2070
	mov cx, 0x0FC8
	rep stosw

	// restore register state
	mov cx, [0x56]
	push cx
	popf
	push ax // clear stack area we just used
	mov	ax, [0x40]
	mov	cx, [0x44]
	mov	sp, [0x48]
	mov	bp, [0x4A]
	mov	si, [0x4C]
	mov	di, [0x4E]
	mov	ds, [0x50]
	mov	es, [0x52]
	mov	ss, [0x54]

	// take clearing a bit more seriously
	mov [0x2070], ax
	mov ax, 0
	// mov [0x42], ax above
	mov [0x44], ax
	// mov [0x46], ax above
	mov [0x48], ax
	mov [0x4A], ax
	mov [0x4C], ax
	mov [0x4E], ax
	mov [0x50], ax
	mov [0x52], ax
	mov [0x54], ax
	mov [0x56], ax
	mov ax, [0x2070]

	// fly me to the moon
	jmp 0xFFFF,0x0000
	.align 2
driver_launch_slot_end:

_rtc_wait_ready:
	push ax
_rtc_wait_ready_loop:
	in al, 0xCA
	test al, 0x10
	jz _rtc_wait_ready_done
	test al, 0x80
	jz _rtc_wait_ready_loop
_rtc_wait_ready_done:
	pop ax
	ret

_rtc_write_data_al:
	call _rtc_wait_ready
	out 0xCB, al
	ret

_driver_change_loop:
	push cx
	mov cx, 4600 // ~1.5ms
_dc_loop2:
	loop _dc_loop2
	pop cx
	ret

// dx = slot
_driver_switch_slot:
	push ax
_driver_switch_slot1:
	mov al, [_driver_current_slot]
	cmp al, dl
	je _driver_switch_slot_equal
	mov [_driver_current_slot], dl

	// call RTC
	mov al, 0xA0
	call _rtc_write_data_al
	mov al, 0x14
	out 0xCA, al
	mov al, dl
	call _rtc_write_data_al
	xor al, al
	call _rtc_write_data_al
	call _rtc_write_data_al
	call _rtc_write_data_al
	call _rtc_write_data_al
	call _rtc_write_data_al
	call _rtc_wait_ready
	call _driver_change_loop

_driver_switch_slot_equal:
	pop ax
	ret

// cx = bank
_driver_switch_slot_bank1:
	push ax
	mov al, cl
	out 0xC3, al
	jmp _driver_switch_slot1

driver_read_slot:
	push	si
	push	di
	push	ds
	push	es
	push	bp
	mov	bp, sp

	mov di, ax

	cli
	call _driver_switch_slot_bank1

	mov bx, 0x3000
	mov	ds, bx
	xor bx, bx
	mov es, bx

	mov si, [bp + 14]
_drs_part2:
	mov	cx, [bp + 16]
	shr	cx, 1
	cld
	rep	movsw
	jnc	_drs_no_byte
	movsb
_drs_no_byte:
	pop	bp
	pop	es
	pop	ds

	mov dl, [fm_initial_slot]
	call _driver_switch_slot
	sti
	mov al, 1

	pop	di
	pop	si

	ASM_PLATFORM_RET 0x4

driver_write_slot:
	push	si
	push	di
	push	ds
	push	es
	push	bp
	mov	bp, sp

	mov si, ax

	cli
	call _driver_switch_slot_bank1

	mov bx, 0x3000
	mov	es, bx
	xor bx, bx
	mov ds, bx

	mov di, [bp + 14]
	jmp _drs_part2

driver_erase_slot:
	// TODO
	mov al, 0
	ASM_PLATFORM_RET

	.section .bss
_driver_current_slot:
	.byte 0
fm_initial_slot:
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
