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
#include "ui.h"
#include "util.h"

volatile uint8_t vbl_ticks;

__attribute__((interrupt))
void vblank_int_handler(void) {
	vbl_ticks++;
	vblank_input_update();
	ws_hwint_ack(HWINT_VBLANK);
}

void main(void) {
	cpu_irq_disable();
	ws_hwint_set_handler(HWINT_IDX_VBLANK, vblank_int_handler);
	ws_hwint_enable(HWINT_VBLANK);
	cpu_irq_enable();

	ui_init();
	driver_init();

	settings_load();
	input_wait_clear(); // wait for input to calm down

	ui_set_current_tab(UI_TAB_BROWSE);
	ui_show();

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
