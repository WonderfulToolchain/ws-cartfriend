#include <stdint.h>
#include <wonderful.h>
#include <ws.h>
#include "error.h"
#include "lang.h"
#include "ui.h"

void error_critical(uint16_t code, uint16_t extra) __far {
    ui_reset_main_screen();
    outportw(IO_IEEP_CTRL, IEEP_PROTECT);

    ui_bg_printf_centered(2, 0, lang_keys[LK_UI_ERROR], code, extra);
    ui_puts_centered(false, 11, 0, lang_keys[LK_UI_ERROR_DESC_LINE1]);
    ui_puts_centered(false, 13, 0, lang_keys[LK_UI_ABOUT_URL_LINE1]);
    ui_puts_centered(false, 14, 0, lang_keys[LK_UI_ABOUT_URL_LINE2]);

    while (true) { cpu_halt(); }
}
