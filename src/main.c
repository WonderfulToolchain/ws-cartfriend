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
#include <ws.h>
#include "driver.h"
#include "input.h"
#include "settings.h"
#include "sram.h"
#include "ui.h"
#include "util.h"
#include "ws/hardware.h"
#include "ws/system.h"

// Memory map:
// 0x0000 - 0x1800: Heap
// 0x1800 - 0x2000: Screen 1
// 0x2000 - 0x3800: Character set [384 glyphs]
// 0x3800 - 0x3C80: Screen 2
// 0x3C80 - 0x3D00: Reserved (About menu sprites)
// 0x3D00 - 0x3F00: Reserved

volatile uint16_t vbl_ticks;

__attribute__((interrupt))
void vblank_int_handler(void) {
	vbl_ticks++;
	vblank_input_update();
	ws_hwint_ack(HWINT_VBLANK);
}

__attribute__((interrupt))
void lowbat_nmi_handler(void) {
	outportb(IO_INT_NMI_CTRL, 0);
}

void main(void) {
	outportb(IO_INT_NMI_CTRL, 0);

	cpu_irq_disable();
	ws_hwint_set_handler(HWINT_IDX_VBLANK, vblank_int_handler);
	ws_hwint_enable(HWINT_VBLANK);
	cpu_irq_enable();

	ui_init();
	driver_init();

	settings_load();
	ui_update_theme(settings_local.color_theme);
	input_wait_clear(); // wait for input to calm down

	ui_set_current_tab(UI_TAB_BROWSE);
	wait_for_vblank();
	ui_show();
    outportb(IO_LCD_SEG, LCD_SEG_ORIENT_H);

	if (settings_local.active_sram_slot == SRAM_SLOT_FIRST_BOOT) {
		ui_reset_main_screen();
		if (ui_dialog_run(0, 0, LK_DIALOG_FIRST_BOOT_ERASE, LK_DIALOG_YES_NO) == 0) {
			sram_erase(SRAM_SLOT_ALL);
		}
	}

	ws_cpuint_set_handler(CPUINT_IDX_NMI, lowbat_nmi_handler);
	outportb(IO_INT_NMI_CTRL, NMI_ON_LOW_BATTERY);
	
	while (true) {
		ui_reset_main_screen();

		switch (ui_current_tab) {
		case UI_TAB_BROWSE:
			ui_browse();
			break;
		case UI_TAB_ABOUT:
			ui_about();
			break;
		case UI_TAB_TOOLS:
			ui_tools();
			break;
		case UI_TAB_SETTINGS:
			ui_settings();
			break;
		}
	}
	
	while(1);
}
