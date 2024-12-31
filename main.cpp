#include <interception.h>
#include <iostream>
#include <unordered_map>

// Enum for keycodes
enum KeyCode {
    // Modifiers
    RWIN = 92,   // Replace with actual key code from logs
    J = 36,      // Replace with actual key code from logs
    LEFT_ARROW = 0xE04B // Replace with actual scancode if different
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

    std::cout << "Karawinner started. Press Right Windows + J to send Left Arrow key." << std::endl;

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        auto *keystroke = reinterpret_cast<InterceptionKeyStroke *>(&stroke);

        // Log every key press or release
        std::cout << "Raw Key Code: " << keystroke->code
                  << ", State: " << ((keystroke->state & INTERCEPTION_KEY_DOWN) ? "DOWN" : "UP")
                  << std::endl;

        // Track the Right Windows key
        if (keystroke->code == RWIN) {
            bool isDown = (keystroke->state & INTERCEPTION_KEY_DOWN) != 0;
            modifierState[RWIN] = isDown;
            std::cout << "Right Windows key " << (isDown ? "pressed" : "released") << std::endl;
        }

        // Check for Right Windows + J
        if (keystroke->code == J && isModifierPressed(RWIN)) {
            std::cout << "Shortcut triggered: Right Windows + J -> Left Arrow" << std::endl;

            // Send the Left Arrow key
            sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_UP);

            continue;
        }

        // Pass through all other keys
        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);
    return 0;
}