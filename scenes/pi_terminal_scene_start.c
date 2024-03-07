#include "../pi_terminal_app_i.h"

// Command action type
typedef enum {
    NO_ACTION = 0,
    OPEN_SETUP,
    OPEN_PORT,
    SEND_CMD,
    SEND_FAST_CMD,
    SEND_WIFITE,
    SEND_CTRL,
    OPEN_HELP
} ActionType;
// Command availability in different modes
typedef enum { OFF = 0, TEXT_MODE = 1, HEX_MODE = 2, BOTH_MODES = 3 } ModeMask;

#define MAX_OPTIONS (8)

typedef struct {
    const char* item_string;
    const char* options_menu[MAX_OPTIONS];
    int num_options_menu;
    ActionType action;
    ModeMask mode_mask;
} pi_terminalItem;

// NUM_MENU_ITEMS defined in pi_terminal_app_i.h - if you add an entry here, increment it!
static const pi_terminalItem items[START_MENU_ITEMS] = {
    {"Setup", {""}, 1, OPEN_SETUP, BOTH_MODES},
    {"Open port", {""}, 1, OPEN_PORT, BOTH_MODES},
    {"Send packet", {""}, 1, SEND_CMD, HEX_MODE},
    {"Send command", {""}, 1, SEND_CMD, TEXT_MODE},
    {"Fast cmd",
     {"kali", "sudo help", "sudo uptime", "sudo reboot", "sudo poweroff", "./log_script.sh"},
     6,
     SEND_FAST_CMD,
     TEXT_MODE},
    {"Wifite",
     {"sudo wifite"},
      1,
      SEND_WIFITE,
      TEXT_MODE},
    {"Send Ctrl+",
     {"C (Interrupt)", "Z (Suspend)", "L (Clear Screen)", "P (Previous Cmd)", "N (Next Cmd)"},
     5,
     SEND_CTRL,
     BOTH_MODES},
    {"Help", {""}, 1, OPEN_HELP, BOTH_MODES},
};

static uint8_t menu_items_num = 0;
static uint8_t item_indexes[START_MENU_ITEMS] = {0};

static void pi_terminal_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    pi_terminalApp* app = context;

    furi_assert(index < menu_items_num);
    uint8_t item_index = item_indexes[index];
    furi_assert(item_index < START_MENU_ITEMS);
    const pi_terminalItem* item = &items[item_index];

    const int selected_option_index = app->selected_option_index[index];
    furi_assert(selected_option_index < item->num_options_menu);
    app->selected_tx_string = item->options_menu[selected_option_index];
    app->is_command = false;
    app->is_custom_tx_string = false;
    app->selected_menu_index = index;

    switch(item->action) {
    case OPEN_SETUP:
        view_dispatcher_send_custom_event(app->view_dispatcher, pi_terminalEventSetup);
        return;
    case SEND_CMD:
    case SEND_WIFITE:
    case SEND_FAST_CMD:
        app->is_command = true;

        if(app->hex_mode) {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, pi_terminalEventStartKeyboardHex);
        } else {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, pi_terminalEventStartKeyboardText);
        }
        return;
    case OPEN_PORT:
        view_dispatcher_send_custom_event(app->view_dispatcher, pi_terminalEventStartConsole);
        return;
    case SEND_CTRL:
        switch(app->selected_option_index[app->selected_menu_index]) {
            case 0: // Ctrl+C
                pi_terminal_uart_send_ctrl_sequence(app->uart, 0x03);
                break;
            case 1: // Ctrl+Z
                pi_terminal_uart_send_ctrl_sequence(app->uart, 0x1A);
                break;
            case 2: // Ctrl+L
                pi_terminal_uart_send_ctrl_sequence(app->uart, 0x0C);
                break;
            case 3: // Ctrl+P
                pi_terminal_uart_send_ctrl_sequence(app->uart, 0x10);
                break;
            case 4: // Ctrl+N
                pi_terminal_uart_send_ctrl_sequence(app->uart, 0x0E);
                break;
        }
        break;
    case OPEN_HELP:
        view_dispatcher_send_custom_event(app->view_dispatcher, pi_terminalEventStartHelp);
        return;
    default:
        return;
    }
}

static void pi_terminal_scene_start_var_list_change_callback(VariableItem* item) {
    furi_assert(item);

    pi_terminalApp* app = variable_item_get_context(item);
    furi_assert(app);

    uint8_t item_index = item_indexes[app->selected_menu_index];
    const pi_terminalItem* menu_item = &items[item_index];
    uint8_t option_index = variable_item_get_current_value_index(item);
    furi_assert(option_index < menu_item->num_options_menu);
    variable_item_set_current_value_text(item, menu_item->options_menu[option_index]);
    app->selected_option_index[app->selected_menu_index] = option_index;
}

void pi_terminal_scene_start_on_enter(void* context) {
    pi_terminalApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    for(int i = 0; i < START_MENU_ITEMS; ++i) {
        app->selected_option_index[i] = 0;
    }

    variable_item_list_set_enter_callback(
        var_item_list, pi_terminal_scene_start_var_list_enter_callback, app);

    VariableItem* item;
    menu_items_num = 0;
    for(int i = 0; i < START_MENU_ITEMS; ++i) {
        bool enabled = false;
        if(app->hex_mode && (items[i].mode_mask & HEX_MODE)) {
            enabled = true;
        }
        if(!app->hex_mode && (items[i].mode_mask & TEXT_MODE)) {
            enabled = true;
        }

        if(enabled) {
            item = variable_item_list_add(
                var_item_list,
                items[i].item_string,
                items[i].num_options_menu,
                pi_terminal_scene_start_var_list_change_callback,
                app);
            variable_item_set_current_value_index(item, app->selected_option_index[i]);
            variable_item_set_current_value_text(
                item, items[i].options_menu[app->selected_option_index[i]]);

            item_indexes[menu_items_num] = i;
            menu_items_num++;
        }
    }

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, pi_terminalSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, pi_terminalAppViewVarItemList);
}

bool pi_terminal_scene_start_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    pi_terminalApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == pi_terminalEventSetup) {
            scene_manager_set_scene_state(
                app->scene_manager, pi_terminalSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, pi_terminalAppViewSetup);
        } else if(event.event == pi_terminalEventStartKeyboardText) {
            scene_manager_set_scene_state(
                app->scene_manager, pi_terminalSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, pi_terminalAppViewTextInput);
        } else if(event.event == pi_terminalEventStartKeyboardHex) {
            scene_manager_set_scene_state(
                app->scene_manager, pi_terminalSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, pi_terminalAppViewHexInput);
        } else if(event.event == pi_terminalEventStartConsole) {
            scene_manager_set_scene_state(
                app->scene_manager, pi_terminalSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, pi_terminalAppViewConsoleOutput);
        } else if(event.event == pi_terminalEventStartHelp) {
            scene_manager_set_scene_state(
                app->scene_manager, pi_terminalSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, pi_terminalAppViewHelp);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        app->selected_menu_index = variable_item_list_get_selected_item_index(app->var_item_list);
        consumed = true;
    }

    return consumed;
}

void pi_terminal_scene_start_on_exit(void* context) {
    pi_terminalApp* app = context;
    variable_item_list_reset(app->var_item_list);
}
