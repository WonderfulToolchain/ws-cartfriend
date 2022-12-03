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
	.global driver_init
	.global driver_lock
	.global driver_unlock
	.global fm_initial_slot
	.global _fm_unlock_refcount
	.global error_critical

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

_rtc_read_data_al:
	call _rtc_wait_ready
	in al, 0xCB
	ret

_driver_change_loop:
	push cx
	mov cx, 9200 // ~15ms
_dc_loop2:
	loop _dc_loop2
	pop cx
	ret

_avr_sleep:
	push ax
	mov al, 0x08
	out 0xCD, al
	xor al, al
	out 0xCC, al
	call _driver_change_loop
	pop ax
	ret

_avr_wake:
	push ax
	mov al, 0x08
	out 0xCD, al
	out 0xCC, al
	xor al, al
	out 0xCD, al
	call _driver_change_loop
	pop ax
	ret

driver_lock:
	mov al, [_fm_unlock_refcount]
	cmp al, 0
	jz driver_lock_underflow
	dec al
	mov [_fm_unlock_refcount], al
	jnz driver_lock_no_lock
	call _avr_sleep
driver_lock_no_lock:
	ASM_PLATFORM_RET

driver_unlock:
	mov al, [_fm_unlock_refcount]
	cmp al, 0
	jnz driver_unlock_already_unlocked
	inc al
	mov [_fm_unlock_refcount], al
	jz driver_unlock_overflow
	call _avr_wake
driver_unlock_already_unlocked:
	ASM_PLATFORM_RET

driver_unlock_overflow:
	mov ax, 3
	mov dx, 0
	//jmp far error_critical
	.byte   0xEA
	.word   error_critical
	.reloc  ., R_386_SEG16, "error_critical!"
	.word   0
driver_lock_underflow:
	mov ax, 4
	mov dx, 0
	//jmp far error_critical
	.byte   0xEA
	.word   error_critical
	.reloc  ., R_386_SEG16, "error_critical!"
	.word   0


driver_init:
	mov byte ptr [_fm_unlock_refcount], 0

	/* call _avr_wake

	// figure out what slot we're on, since we're unlocking
	// FIXME: This doesn't work right.
	
	// SET_DATE_AND_TIME with GET_SLOT
	mov al, 0xA1
	call _rtc_write_data_al
	mov al, 0x14
	out 0xCA, al

	xor al, al
	call _rtc_write_data_al
	call _rtc_write_data_al
	call _rtc_write_data_al
	call _rtc_write_data_al
	call _rtc_write_data_al
	call _rtc_write_data_al

	call _driver_change_loop
	call _rtc_wait_ready

	// GET_DATE_AND_TIME
	mov al, 0x15
	out 0xCA, al
	call _rtc_read_data_al
	mov [fm_initial_slot+1], al
	call _rtc_read_data_al
	mov [fm_initial_slot+2], al
	call _rtc_read_data_al
	mov [fm_initial_slot+3], al
	call _rtc_read_data_al
	mov [fm_initial_slot+4], al
	call _rtc_read_data_al
	mov [fm_initial_slot+5], al
	call _rtc_read_data_al
	mov [fm_initial_slot+6], al
	call _rtc_read_data_al

	call _avr_sleep */

	xor al, al
	mov [fm_initial_slot], al

	ASM_PLATFORM_RET
