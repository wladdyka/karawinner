#include <interception.h>
#include <iostream>
#include <unordered_map>

enum KeyCode {
    // Modifiers
    RSHIFT = 0x36,
    LSHIFT = 0x2A,
    RCTRL = 0x1D,
    RALT = 0x38,
    RWIN = 92,   // Updated value for Right Windows
    // Alphabet keys
    J = 36,      // Updated value for J
    L = 37,      // Update these with correct values from logs
    I = 38,
    K = 39,
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

void logKeyState(const char* key, bool state) {
    std::cout << key << (state ? " pressed" : " released") << std::endl;
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

        // Log key press or release
       std::cout << "Raw Key Code: " << keystroke->code
          << ", State: " << ((keystroke->state & INTERCEPTION_KEY_DOWN) ? "DOWN" : "UP")
          << std::endl;

        // Track modifier keys (Shift, Ctrl, Right Windows)
        if (keystroke->code == RSHIFT || keystroke->code == LSHIFT ||
            keystroke->code == RCTRL || keystroke->code == RALT ||
            keystroke->code == RWIN) {
            bool isDown = (keystroke->state & INTERCEPTION_KEY_DOWN) != 0;
            modifierState[keystroke->code] = isDown;

            // Log modifier state
            const char* keyName = (keystroke->code == RSHIFT) ? "Right Shift" :
                                  (keystroke->code == LSHIFT) ? "Left Shift" :
                                  (keystroke->code == RCTRL) ? "Right Ctrl" :
                                  (keystroke->code == RALT) ? "Right Alt" :
                                  "Right Windows";
            logKeyState(keyName, isDown);
        }

        // Right Windows + J -> Left Arrow
        if (keystroke->code == J && isModifierPressed(RWIN)) {
            std::cout << "Shortcut triggered: Right Windows + J -> Left Arrow" << std::endl;
            if (isModifierPressed(RSHIFT)) {
                std::cout << "Shift is also pressed, highlighting left" << std::endl;
                sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_DOWN);
            } else {
                std::cout << "Sending Left Arrow key" << std::endl;
                sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_DOWN);
                sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_UP);
            }
            continue;
        }

        // Right Windows + L -> Right Arrow
        if (keystroke->code == L && isModifierPressed(RWIN)) {
            std::cout << "Shortcut triggered: Right Windows + L -> Right Arrow" << std::endl;
            if (isModifierPressed(RSHIFT)) {
                std::cout << "Shift is also pressed, highlighting right" << std::endl;
                sendKey(context, device, RIGHT_ARROW, INTERCEPTION_KEY_DOWN);
            } else {
                std::cout << "Sending Right Arrow key" << std::endl;
                sendKey(context, device, RIGHT_ARROW, INTERCEPTION_KEY_DOWN);
                sendKey(context, device, RIGHT_ARROW, INTERCEPTION_KEY_UP);
            }
            continue;
        }

        // Right Windows + I -> Up Arrow
        if (keystroke->code == I && isModifierPressed(RWIN)) {
            std::cout << "Shortcut triggered: Right Windows + I -> Up Arrow" << std::endl;
            sendKey(context, device, UP_ARROW, keystroke->state);
            continue;
        }

        // Right Windows + K -> Down Arrow
        if (keystroke->code == K && isModifierPressed(RWIN)) {
            std::cout << "Shortcut triggered: Right Windows + K -> Down Arrow" << std::endl;
            sendKey(context, device, DOWN_ARROW, keystroke->state);
            continue;
        }

        // Pass through other keys
        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);
    return 0;
}
