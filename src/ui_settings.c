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
#include "lang.h"
#include "nanoprintf.h"
#include "settings.h"
#include "sram.h"
#include "ui.h"
#include "util.h"

#define MENU_OPT_SAVEMAP 0
#define MENU_OPT_SAVE 1
#define MENU_OPT_ERASE_SRAM 2
#define MENU_OPT_FORCECARTSRAM 3

static uint16_t __far ui_opt_lks[] = {
    LK_UI_SETTINGS_SAVEMAP,
    LK_UI_SETTINGS_SAVE,
    LK_UI_SETTINGS_ERASE_SRAM,
    LK_UI_SETTINGS_FORCECARTSRAM,
};
static void ui_opt_menu_build_line(uint8_t entry_id, char *buf, int buf_len, char *buf_right, int buf_right_len) {
    bool marked = false;
    if (entry_id == MENU_OPT_SAVE) {
        marked = settings_changed;
    } else if (entry_id == MENU_OPT_FORCECARTSRAM) {
        marked = settings_local.active_sram_slot == SRAM_SLOT_FIRST_BOOT;
    }
    npf_snprintf(buf, buf_len, lang_keys[marked ? LK_MENU_MARKED : LK_MENU_UNMARKED], lang_keys[ui_opt_lks[entry_id]]);
}

static void ui_opt_menu_savemap_build_line(uint8_t entry_id, char *buf, int buf_len, char *buf_right, int buf_right_len) {
    if (entry_id < SRAM_SLOTS) {
        npf_snprintf(buf, buf_len, lang_keys[entry_id == settings_local.active_sram_slot ? LK_UI_SAVEMAP_SRAM_ACTIVE : LK_UI_SAVEMAP_SRAM], entry_id + 1);
        uint8_t sram_target = settings_local.sram_slot_mapping[entry_id];
        if (sram_target < GAME_SLOTS) {
            npf_snprintf(buf_right, buf_right_len, lang_keys[LK_UI_SAVEMAP_SLOT], sram_target + 1);
        } else if (sram_target == 0xFF) {
            strncpy(buf_right, lang_keys[LK_UI_SAVEMAP_UNUSED], buf_right_len);
        }
    }
}

static void ui_opt_menu_erase_sram_build_line(uint8_t entry_id, char *buf, int buf_len, char *buf_right, int buf_right_len) {
    if (entry_id < SRAM_SLOTS) {
        npf_snprintf(buf, buf_len, lang_keys[entry_id == settings_local.active_sram_slot ? LK_UI_ERASE_SRAM_ACTIVE : LK_UI_ERASE_SRAM], entry_id + 1);
    } else if (entry_id == 0xFE) {
        strncpy(buf, lang_keys[LK_UI_ERASE_SRAM_CARTRIDGE], buf_len);
    } else if (entry_id == SRAM_SLOT_ALL) {
        strncpy(buf, lang_keys[LK_UI_ERASE_SRAM_ALL], buf_len);
    }
}


static void ui_opt_menu_settings_confirm_build_line(uint8_t entry_id, char *buf, int buf_len, char *buf_right, int buf_right_len) {
    strncpy(buf, lang_keys[entry_id ? LK_UI_CONFIRM_YES : LK_UI_CONFIRM_NO], buf_len);
}

static uint8_t ui_savemap_next(uint8_t slot) {
    // 0xFF -> 0
    for (uint8_t i = slot + 1; i < GAME_SLOTS; i++) {
        if (settings_local.slot_type[i] == SLOT_TYPE_GAME || settings_local.slot_type[i] == SLOT_TYPE_MULTI_LINEAR_GAME) {
            return i;
        }
    }
    return 0xFF;
}

static uint8_t ui_savemap_prev(uint8_t slot) {
    for (uint8_t i = slot == 0xFF ? GAME_SLOTS : (slot - 1); i >= 0; i--) {
        if (settings_local.slot_type[i] == SLOT_TYPE_GAME || settings_local.slot_type[i] == SLOT_TYPE_MULTI_LINEAR_GAME) {
            return i;
        }
    }
    return 0xFF;
}

void ui_settings(void) {
    uint8_t menu_list[32];
    uint8_t i = 0;
    if (driver_supports_slots()) {
        menu_list[i++] = MENU_OPT_SAVEMAP;
    }
    menu_list[i++] = MENU_OPT_FORCECARTSRAM;
    menu_list[i++] = MENU_OPT_SAVE;
    menu_list[i++] = MENU_OPT_ERASE_SRAM;
    menu_list[i++] = MENU_ENTRY_END;

    ui_menu_state_t menu = {
        .list = menu_list,
        .build_line_func = ui_opt_menu_build_line,
        .flags = 0
    };
    ui_menu_init(&menu);
    
    uint16_t result = ui_menu_select(&menu);
    if (result == MENU_OPT_SAVEMAP) {
        i = 0;
        for (uint8_t j = 0; j < SRAM_SLOTS; j++) {
            menu_list[i++] = j;
        }
        menu_list[i] = MENU_ENTRY_END;

        menu.build_line_func = ui_opt_menu_savemap_build_line;
        menu.flags = MENU_SEND_LEFT_RIGHT | MENU_B_AS_BACK;
        ui_menu_init(&menu);
        ui_reset_main_screen();

        while (true) {
            result = ui_menu_select(&menu);
            if (result == MENU_ENTRY_END) {
                break;
            } else if (result & MENU_ACTION_LEFT) {
                settings_local.sram_slot_mapping[result & 0xFF] = ui_savemap_prev(settings_local.sram_slot_mapping[result & 0xFF]);
                settings_mark_changed();
            } else {
                settings_local.sram_slot_mapping[result & 0xFF] = ui_savemap_next(settings_local.sram_slot_mapping[result & 0xFF]);
                settings_mark_changed();
            }
        }
    } else if (result == MENU_OPT_SAVE) {
        settings_save();
    } else if (result == MENU_OPT_FORCECARTSRAM) {
        if (settings_local.active_sram_slot != 0xFE) {
            menu_list[0] = 0;
            menu_list[1] = 1;
            menu_list[2] = MENU_ENTRY_END;

            menu.build_line_func = ui_opt_menu_settings_confirm_build_line;
            menu.flags = MENU_B_AS_BACK;
            ui_menu_init(&menu);
            ui_reset_main_screen();

            result = ui_menu_select(&menu);
            if (result == 1) {
                settings_local.active_sram_slot = 0xFE;
                settings_mark_changed();
            }
        }
    } else if (result == MENU_OPT_ERASE_SRAM) {
        i = 0;
        for (uint8_t j = 0; j < SRAM_SLOTS; j++) {
            menu_list[i++] = j;
        }
        menu_list[i++] = 0xFE;
        menu_list[i++] = SRAM_SLOT_ALL;
        menu_list[i] = MENU_ENTRY_END;

        menu.build_line_func = ui_opt_menu_erase_sram_build_line;
        menu.flags = MENU_B_AS_BACK;
        ui_menu_init(&menu);
        ui_reset_main_screen();

        result = ui_menu_select(&menu);

        uint8_t slot_to_erase;

        if (result < SRAM_SLOTS) {
            slot_to_erase = result;
        } else if (result == 0xFE) {
            slot_to_erase = 0xFF;
        } else if (result == SRAM_SLOT_ALL) {
            slot_to_erase = SRAM_SLOT_ALL;
        } else {
            return;
        }

        menu_list[0] = 0;
        menu_list[1] = 1;
        menu_list[2] = MENU_ENTRY_END;

        menu.build_line_func = ui_opt_menu_settings_confirm_build_line;
        menu.flags = MENU_B_AS_BACK;
        ui_menu_init(&menu);
        ui_reset_main_screen();

        result = ui_menu_select(&menu);
        if (result == 1) {
            sram_erase(slot_to_erase);
        }
    }
}