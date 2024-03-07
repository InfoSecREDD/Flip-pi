#include "pi_terminal_app_i.h"
#include "pi_terminal_uart.h"

struct pi_terminalUart {
    pi_terminalApp* app;
    FuriThread* rx_thread;
    FuriStreamBuffer* rx_stream;
    uint8_t rx_buf[RX_BUF_SIZE + 1];
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context);
    FuriHalSerialHandle* serial_handle;
};

typedef enum {
    WorkerEvtStop = (1 << 0),
    WorkerEvtRxDone = (1 << 1),
} WorkerEvtFlags;

void pi_terminal_uart_set_handle_rx_data_cb(
    pi_terminalUart* uart,
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context)) {
    furi_assert(uart);
    uart->handle_rx_data_cb = handle_rx_data_cb;
}

#define WORKER_ALL_RX_EVENTS (WorkerEvtStop | WorkerEvtRxDone)

void pi_terminal_uart_on_irq_cb(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    pi_terminalUart* uart = (pi_terminalUart*)context;

    if(event == FuriHalSerialRxEventData) {
        uint8_t data = furi_hal_serial_async_rx(handle);
        furi_stream_buffer_send(uart->rx_stream, &data, 1, 0);
        furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtRxDone);
    }
}

static int32_t uart_worker(void* context) {
    pi_terminalUart* uart = (void*)context;

    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        furi_check((events & FuriFlagError) == 0);
        if(events & WorkerEvtStop) break;
        if(events & WorkerEvtRxDone) {
            size_t len = furi_stream_buffer_receive(uart->rx_stream, uart->rx_buf, RX_BUF_SIZE, 0);
            if(len > 0) {
                if(uart->handle_rx_data_cb) uart->handle_rx_data_cb(uart->rx_buf, len, uart->app);
            }
        }
    }

    furi_stream_buffer_free(uart->rx_stream);

    return 0;
}

void pi_terminal_uart_tx(pi_terminalUart* uart, uint8_t* data, size_t len) {
    furi_hal_serial_tx(uart->serial_handle, data, len);
}

pi_terminalUart* pi_terminal_uart_init(pi_terminalApp* app) {
    pi_terminalUart* uart = malloc(sizeof(pi_terminalUart));
    uart->app = app;
    // Init all rx stream and thread early to avoid crashes
    uart->rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE, 1);
    uart->rx_thread = furi_thread_alloc();
    furi_thread_set_name(uart->rx_thread, "pi_terminalUartRxThread");
    furi_thread_set_stack_size(uart->rx_thread, 1024);
    furi_thread_set_context(uart->rx_thread, uart);
    furi_thread_set_callback(uart->rx_thread, uart_worker);

    furi_thread_start(uart->rx_thread);

    if(app->BAUDRATE == 0) {
        app->BAUDRATE = 115200;
    }

    uart->serial_handle = furi_hal_serial_control_acquire(app->uart_ch);
    furi_check(uart->serial_handle);
    furi_hal_serial_init(uart->serial_handle, app->BAUDRATE);

    furi_hal_serial_async_rx_start(uart->serial_handle, pi_terminal_uart_on_irq_cb, uart, false);

    return uart;
}

void pi_terminal_uart_free(pi_terminalUart* uart) {
    furi_assert(uart);

    furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtStop);
    furi_thread_join(uart->rx_thread);
    furi_thread_free(uart->rx_thread);

    furi_hal_serial_deinit(uart->serial_handle);
    furi_hal_serial_control_release(uart->serial_handle);

    free(uart);
}

void pi_terminal_uart_send_ctrl_sequence(pi_terminalUart* uart, uint8_t sequence) {
    if(uart && uart->serial_handle) {
        pi_terminal_uart_tx(uart, &sequence, 1);
    }
}