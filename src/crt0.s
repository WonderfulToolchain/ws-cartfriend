/**
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

	.arch	i186
	.code16
	.intel_syntax noprefix

	.section .ivt
	.global _ivt
_ivt:
	.fill 16, 4, 0
	.fill 12, 2, 0

	.section .start
	.global _start
_start:
	cli

	// store correct state of AX and DS in a screen LUT
	// we need to initialize DS to access RAM, but we want to preserve it
	out	0x20, ax
	mov	ax, ds
	out	0x22, ax

	// initialize DS to 0x0000
	mov	ax, 0x0000
	mov	ds, ax
	
	// preserve registers in RAM
	in	ax, 0x20
	mov	[0x40], ax
	mov	[0x42], bx
	mov	[0x44], cx
	mov	[0x46], dx
	mov	[0x48], sp
	mov	[0x4A], bp
	mov	[0x4C], si
	mov	[0x4E], di
	// CS is known
	in	ax, 0x22
	mov	[0x50], ax
	mov	[0x52], es
	mov	[0x54], ss

	// init ES/SS/SP here so we can PUSHF
	mov	ax, 0x0000
	mov	es, ax
	mov	ss, ax
	mov	sp, offset "__eheap"

	pushf
	pop ax
	mov	[0x56], ax

	// CartFriend end

	// copy rodata/data from ROM to RAM
	//mov	ax, offset "__erom!"
#ifdef __IA16_CMODEL_IS_FAR_TEXT
	// set DS to the location of rodata
	.byte	0xB8
	.reloc	., R_386_SEG16, "__erom!"
	.word	0
	mov	ds, ax
#endif
	mov	si, offset "__erom&"
	mov	di, offset "__sdata"
	mov	cx, offset "__lwdata"
#ifndef __IA16_CMODEL_IS_FAR_TEXT
	// set DS to the location of rodata
	push cs
	pop ds
#endif
	cld
	rep	movsw

	// initialize segments
	// (es/ss) initialized above
	xor	ax, ax
	mov	ds, ax

	// clear int enable
	out	0xB2, al

	// clear bss
	mov	di, offset "__edata"
	mov	cx, offset "__lwbss"
	rep	stosw

	// configure default interrupt base
	mov	al, 0x08
	out	0xB0, al

#ifdef __IA16_CMODEL_IS_FAR_TEXT
	//.reloc	.+3, R_386_SEG16, main
	//jmp 0:main
	.byte	0xEA
	.word	main
	.reloc	., R_386_SEG16, "main!"
	.word	0
#else
	jmp main
#endif
