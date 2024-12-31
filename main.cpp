#include <interception.h>
#include <iostream>
#include <unordered_map>

// Enum for keycodes
enum KeyCode {
    // Modifiers
    RSHIFT = 0x36,
    LSHIFT = 0x2A,
    RCTRL = 0x1D,
    RALT = 0x38,
    RWIN = 0xE05B,
    // Alphabet keys
    J = 0x24,
    L = 0x26,
    I = 0x17,
    K = 0x25,
    // Arrow keys
    LEFT_ARROW = 0xE04B,
    RIGHT_ARROW = 0xE04D,
    UP_ARROW = 0xE048,
    DOWN_ARROW = 0xE050
};

// Helper to track modifier key states
std::unordered_map<int, bool> modifierState;

bool isModifierPressed(KeyCode code) {
    return modifierState.count(code) && modifierState[code];
}

void sendKey(InterceptionContext context, InterceptionDevice device, KeyCode keyCode, int keyState) {
    InterceptionKeyStroke keystroke;
    keystroke.code = keyCode;
    keystroke.state = keyState;
    interception_send(context, device, reinterpret_cast<InterceptionStroke*>(&keystroke), 1);
}

int main() {
    // Initialize Interception
    InterceptionContext context = interception_create_context();
    InterceptionDevice device;
    InterceptionStroke stroke;

    // Filter for keyboard input
    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    std::cout << "Karawinner started. Press Right Windows + J/L/I/K with modifiers to test remapping." << std::endl;

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        auto *keystroke = reinterpret_cast<InterceptionKeyStroke *>(&stroke);

        // Track modifier keys (Shift, Ctrl, Right Windows)
        if (keystroke->code == RSHIFT || keystroke->code == LSHIFT ||
            keystroke->code == RCTRL || keystroke->code == RALT ||
            keystroke->code == RWIN) {
            modifierState[keystroke->code] = (keystroke->state & INTERCEPTION_KEY_DOWN) != 0;
        }

        // Right Windows + J -> Left Arrow
        if (keystroke->code == J && isModifierPressed(RWIN)) {
            if (isModifierPressed(RSHIFT)) {
                // Highlight to the left
                sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_DOWN);
            } else {
                // Just move to the left
                sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_DOWN);
                sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_UP);
            }
            continue;
        }

        // Right Windows + L -> Right Arrow
        if (keystroke->code == L && isModifierPressed(RWIN)) {
            if (isModifierPressed(RSHIFT)) {
                // Highlight to the right
                sendKey(context, device, RIGHT_ARROW, INTERCEPTION_KEY_DOWN);
            } else {
                // Just move to the right
                sendKey(context, device, RIGHT_ARROW, INTERCEPTION_KEY_DOWN);
                sendKey(context, device, RIGHT_ARROW, INTERCEPTION_KEY_UP);
            }
            continue;
        }

        // Right Windows + I -> Up Arrow
        if (keystroke->code == I && isModifierPressed(RWIN)) {
            sendKey(context, device, UP_ARROW, keystroke->state);
            continue;
        }

        // Right Windows + K -> Down Arrow
        if (keystroke->code == K && isModifierPressed(RWIN)) {
            sendKey(context, device, DOWN_ARROW, keystroke->state);
            continue;
        }

        // Pass through other keys
        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);
    return 0;
}
