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

#include <string.h>
#include <ws.h>
#include "config.h"
#include "driver.h"
#include "input.h"
#include "lang.h"
#include "nanoprintf.h"
#include "settings.h"
#include "sram.h"
#include "ui.h"
#include "util.h"
#include "ws/hardware.h"

#define BROWSE_SUB_LAUNCH 0
#define BROWSE_SUB_INFO 1
#define BROWSE_SUB_RENAME 2

static void ui_browse_menu_build_line(uint8_t entry_id, void *userdata, char *buf, int buf_len, char *buf_right, int buf_right_len) {
    char buf_name[28];
    uint8_t *cart_metadata = (uint8_t*) userdata;

    if (entry_id < GAME_SLOTS) {
        if (settings_local.slot_name[entry_id][0] >= 0x20) {
            memcpy(buf_name, settings_local.slot_name[entry_id] + 1, 23);
            buf_name[23] = 0;
        } else {
            npf_snprintf(buf_name, sizeof(buf_name), lang_keys[LK_UI_BROWSE_SLOT_DEFAULT_NAME],
                (uint16_t) cart_metadata[(entry_id << 2)],     (uint16_t) cart_metadata[(entry_id << 2) + 1],
                (uint16_t) cart_metadata[(entry_id << 2) + 2], (uint16_t) cart_metadata[(entry_id << 2) + 3]
            );
        }
        npf_snprintf(buf, buf_len, lang_keys[LK_UI_BROWSE_SLOT], (uint16_t) (entry_id + 1), ' ', ((const char __far*) buf_name));
    }
}

static uint16_t __far browse_sub_lks[] = {
    LK_UI_BROWSE_POPUP_LAUNCH,
    LK_UI_BROWSE_POPUP_INFO,
    LK_UI_BROWSE_POPUP_RENAME
};

static void ui_browse_submenu_build_line(uint8_t entry_id, void *userdata, char *buf, int buf_len) {
    strncpy(buf, lang_keys[browse_sub_lks[entry_id]], buf_len);
}

static uint8_t iterate_carts(uint8_t *menu_list, uint8_t *cart_metadata, uint8_t i) {
    uint8_t buffer[16];

    ui_step_work_indicator();
    driver_unlock();
    for (uint8_t slot = 0; slot < GAME_SLOTS; slot++) {
        ui_step_work_indicator();
        if (settings_local.slot_type[slot] == SLOT_TYPE_UNUSED) {
            continue;
        }

        if (driver_get_launch_slot() == slot) {
            // if booted from RAM/SRAM, don't skip launch slot
            if (_CS >= 0x2000) {
                continue;
            }
        }

        memset(buffer, 0xFF, sizeof(buffer));
        if (driver_read_slot(buffer, slot, 0xFF, 0xFFF0, 16)) {
            if (buffer[0] == 0xEA || buffer[0] == 0x9A) {
		cart_metadata[(slot << 2)] = buffer[0x08];
		cart_metadata[(slot << 2)+1] = buffer[0x09];
		cart_metadata[(slot << 2)+2] = buffer[0x0E];
		cart_metadata[(slot << 2)+3] = buffer[0x0F];
                menu_list[i++] = slot;
            }
        }
    }
    driver_lock();

    return i;
}

void ui_browse(void) {
    uint8_t menu_list[256];
    uint8_t cart_metadata[512];
    uint8_t i = 0;

    i = iterate_carts(menu_list, cart_metadata, i);
    menu_list[i++] = MENU_ENTRY_END;

    ui_menu_state_t menu = {
        .list = menu_list,
        .build_line_func = ui_browse_menu_build_line,
        .build_line_data = cart_metadata,
        .flags = MENU_B_AS_ACTION
    };
    ui_menu_init(&menu);

    uint16_t result = ui_menu_select(&menu);
    uint16_t subaction = 0;
    if ((result & 0xFF) < 0xF0) {
        if ((result & 0xFF00) == MENU_ACTION_B) {
            ui_popup_menu_state_t popup_menu = {
                .list = menu_list,
                .build_line_func = ui_browse_submenu_build_line,
                .flags = 0
            };
            i = 0;
            menu_list[i++] = BROWSE_SUB_LAUNCH;
            menu_list[i++] = BROWSE_SUB_RENAME;
            menu_list[i++] = MENU_ENTRY_END;
            subaction = ui_popup_menu_run(&popup_menu);
        }
        result &= 0xFF;

        if (subaction == BROWSE_SUB_LAUNCH) {
            ui_reset_main_screen();
            driver_unlock();

            memset(menu_list, 0xFF, 16);
            driver_read_slot(menu_list, result, 0xFF, 0xFFF0, 16);

            // does the game use SRAM?
            if (menu_list[0x0B] != 0 && _CS >= 0x2000) {
                // figure out SRAM slots
                i = 0;
                for (uint8_t k = 0; k < SRAM_SLOTS; k++) {
                    if (settings_local.sram_slot_mapping[k] == result) {
                        menu_list[i++] = k;
                    }
                }
                uint8_t sram_slot = 0xFF;
                if (i > 1) {
                    menu_list[i++] = MENU_ENTRY_END;
                    menu.flags = MENU_B_AS_BACK;
                    ui_menu_init(&menu);
                    uint16_t result_sram = ui_menu_select(&menu);
                    if (result_sram == MENU_ENTRY_END) {
                        return;
                    } else {
                        sram_slot = menu_list[result_sram & 0xFF];
                    }
                } else if (i == 1) {
                    sram_slot = menu_list[0];
                }

                sram_switch_to_slot(sram_slot);
            }

            // does the game leave IEEPROM unlocked?
            if (!(menu_list[0x09] & 0x80)) {
                // lock IEEPROM
                outportw(IO_IEEP_CTRL, IEEP_PROTECT);
            }

            input_wait_clear();
            launch_slot(result, 0xFF);
        } else if (subaction == BROWSE_SUB_RENAME) {
            if (result < GAME_SLOTS) {
                if (settings_local.slot_name[result][0] < 0x20) {
                    menu_list[0] = 0;
                } else {
                    memcpy(menu_list, settings_local.slot_name[result] + 1, 23);
                }
                if (ui_osk_run(0, (char*) menu_list, 23)) {
                    memset(settings_local.slot_name[result], 0, 24);
                    if (menu_list[0] != 0) {
                        settings_local.slot_name[result][0] = 0x20;
                        strncpy((char*) (settings_local.slot_name[result] + 1), (char*) menu_list, 23);
                    }
                }
            }
        }
    }
}
