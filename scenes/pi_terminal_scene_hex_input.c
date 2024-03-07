#include "../pi_terminal_app_i.h"

void pi_terminal_scene_hex_input_callback(void* context) {
    pi_terminalapp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, pi_terminalEventStartConsole);
}

void pi_terminal_scene_hex_input_on_enter(void* context) {
    pi_terminalapp* app = context;

    // Setup view
    UART_TextInput* text_input = app->hex_input;
    // Add help message to header
    uart_hex_input_set_header_text(text_input, "Send HEX packet to UART");
    uart_hex_input_set_result_callback(
        text_input,
        pi_terminal_scene_hex_input_callback,
        app,
        app->text_input_store,
        pi_terminal_TEXT_INPUT_STORE_SIZE,
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, pi_terminalappViewHexInput);
}

bool pi_terminal_scene_hex_input_on_event(void* context, SceneManagerEvent event) {
    pi_terminalapp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == pi_terminalEventStartConsole) {
            // Point to custom string to send
            app->selected_tx_string = app->text_input_store;
            scene_manager_next_scene(app->scene_manager, pi_terminalappViewConsoleOutput);
            consumed = true;
        }
    }

    return consumed;
}

void pi_terminal_scene_hex_input_on_exit(void* context) {
    pi_terminalapp* app = context;

    uart_hex_input_reset(app->text_input);
}
