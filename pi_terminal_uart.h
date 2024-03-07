#pragma once

#include "furi_hal.h"

#define RX_BUF_SIZE (320)

typedef struct pi_terminalUart pi_terminalUart;

void pi_terminal_uart_set_handle_rx_data_cb(
    pi_terminalUart* uart,
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context));
void pi_terminal_uart_tx(pi_terminalUart* uart, uint8_t* data, size_t len);
pi_terminalUart* pi_terminal_uart_init(pi_terminalapp* app);
void pi_terminal_uart_free(pi_terminalUart* uart);
void pi_terminal_uart_send_ctrl_sequence(pi_terminalUart* uart, uint8_t sequence);