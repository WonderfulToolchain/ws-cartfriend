#include <stdint.h>
#include <ws.h>
#include "serial.h"

#define SERIAL_TXBUF_SIZE 128

uint8_t serial_txbuf[SERIAL_TXBUF_SIZE];
uint8_t serial_txbuf_pos = 0, serial_txbuf_len = 0;

__attribute__((interrupt))
static void serial_txbuf_int_handler(void) __far {
    if (serial_txbuf_pos != serial_txbuf_len) {
        ws_serial_putc(serial_txbuf[serial_txbuf_pos]);
        serial_txbuf_pos = (serial_txbuf_pos + 1) & (SERIAL_TXBUF_SIZE - 1);
    }
    if (serial_txbuf_pos == serial_txbuf_len) {
        ws_hwint_disable(HWINT_SERIAL_TX);
    }
}

void serial_init_buffered(void) {
    ws_hwint_set_handler(HWINT_IDX_SERIAL_TX, serial_txbuf_int_handler);
}

void serial_flush_buffered(void) {
    while (serial_txbuf_pos != serial_txbuf_len) {
        cpu_halt();
    }
}

void serial_putc_buffered(uint8_t value) {
    serial_txbuf[serial_txbuf_len] = value;
    uint8_t next_len = (serial_txbuf_len + 1) & (SERIAL_TXBUF_SIZE - 1);
    while (next_len == serial_txbuf_pos) {
        cpu_halt();
    }
    serial_txbuf_len = next_len;
    ws_hwint_enable(HWINT_SERIAL_TX);
}
