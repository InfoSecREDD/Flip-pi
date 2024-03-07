#pragma once

typedef enum {
    pi_terminalEventRefreshConsoleOutput = 0,
    pi_terminalEventSetup,
    pi_terminalEventStartConsole,
    pi_terminalEventStartKeyboardText,
    pi_terminalEventStartKeyboardHex,
    pi_terminalEventStartHelp,
} pi_terminalCustomEvent;
