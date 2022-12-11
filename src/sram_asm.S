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

#include "wonderful-asm.h"

	.arch	i186
	.code16
	.intel_syntax noprefix

	.global sram_copy_buffer_checkff
sram_copy_buffer_checkff:
	push	si
	push	di
	push	ds
	push	es

	// configure ds:si = 0x1000:dx, es:di = 0x0000:ax, dx = 0xFFFF, cx = 0x80
	mov di, ax
	mov si, dx
	xor ax, ax
	mov es, ax
	mov dx, ax
	dec dx
	mov ah, 0x10
	mov ds, ax
	mov cx, 0x80
	cld
sram_copy_buffer_checkff_loop:
	lodsw
	stosw
	cmp ax, dx
	dec cx
	jnz sram_copy_buffer_checkff_loop_found_data_postdec
	lodsw
	stosw
	cmp ax, dx
	jnz sram_copy_buffer_checkff_loop_found_data
	loop sram_copy_buffer_checkff_loop
	mov al, 0
	jmp sram_copy_buffer_checkff_loop_done

sram_copy_buffer_checkff_loop_found_data:
	dec cx
sram_copy_buffer_checkff_loop_found_data_postdec:
	mov al, 1
	rep movsw

sram_copy_buffer_checkff_loop_done:
	pop	es
	pop ds
	pop	di
	pop	si
	ASM_PLATFORM_RET
