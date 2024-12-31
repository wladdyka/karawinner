#include <interception.h>
#include <iostream>
#include <unordered_map>

// Helper to track modifier key states
std::unordered_map<int, bool> modifierState;

bool isModifierPressed(int code) {
    return modifierState.count(code) && modifierState[code];
}

void sendKey(InterceptionContext context, InterceptionDevice device, int keyCode, int keyState) {
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
        if (keystroke->code == SCANCODE_RSHIFT || keystroke->code == SCANCODE_LSHIFT ||
            keystroke->code == SCANCODE_RCTRL || keystroke->code == SCANCODE_RALT ||
            keystroke->code == SCANCODE_RWIN) {
            modifierState[keystroke->code] = (keystroke->state & INTERCEPTION_KEY_DOWN) != 0;
        }

        // Right Windows + J -> Left Arrow
        if (keystroke->code == SCANCODE_J && isModifierPressed(SCANCODE_RWIN)) {
            if (isModifierPressed(SCANCODE_RSHIFT)) {
                // Highlight to the left
                sendKey(context, device, SCANCODE_LEFT_ARROW, INTERCEPTION_KEY_DOWN);
            } else {
                // Just move to the left
                sendKey(context, device, SCANCODE_LEFT_ARROW, INTERCEPTION_KEY_DOWN);
                sendKey(context, device, SCANCODE_LEFT_ARROW, INTERCEPTION_KEY_UP);
            }
            continue;
        }

        // Right Windows + L -> Right Arrow
        if (keystroke->code == SCANCODE_L && isModifierPressed(SCANCODE_RWIN)) {
            if (isModifierPressed(SCANCODE_RSHIFT)) {
                // Highlight to the right
                sendKey(context, device, SCANCODE_RIGHT_ARROW, INTERCEPTION_KEY_DOWN);
            } else {
                // Just move to the right
                sendKey(context, device, SCANCODE_RIGHT_ARROW, INTERCEPTION_KEY_DOWN);
                sendKey(context, device, SCANCODE_RIGHT_ARROW, INTERCEPTION_KEY_UP);
            }
            continue;
        }

        // Right Windows + I -> Up Arrow
        if (keystroke->code == SCANCODE_I && isModifierPressed(SCANCODE_RWIN)) {
            sendKey(context, device, SCANCODE_UP_ARROW, keystroke->state);
            continue;
        }

        // Right Windows + K -> Down Arrow
        if (keystroke->code == SCANCODE_K && isModifierPressed(SCANCODE_RWIN)) {
        if (keystroke->code == SCANCODE_K && isModifierPressed(SCANCODE_RWIN)) {
            sendKey(context, device, SCANCODE_DOWN_ARROW, keystroke->state);
            continue;
        }

        // Pass through other keys
        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);
    return 0;
}
