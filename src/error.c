#include <stdint.h>
#include <ws.h>
#include "error.h"
#include "lang.h"
#include "ui.h"
#include "wonderful-asm-common.h"

void error_critical(uint16_t code, uint16_t extra) {
    ui_reset_main_screen();

    ui_printf_centered(false, 2, 0, lang_keys[LK_UI_ERROR], code, extra);
    ui_puts_centered(false, 11, 0, lang_keys[LK_UI_ERROR_DESC_LINE1]);
    ui_puts_centered(false, 13, 0, lang_keys[LK_UI_ABOUT_URL_LINE1]);
    ui_puts_centered(false, 14, 0, lang_keys[LK_UI_ABOUT_URL_LINE2]);

    while (true) { cpu_halt(); }
}