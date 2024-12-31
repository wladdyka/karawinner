#include <interception.h>
#include <iostream>
#include <unordered_map>

// Enum for keycodes
enum KeyCode {
    RWIN = 92,    // Right Windows key
    J = 36,       // J key
    LEFT_ARROW = 75 // Arrow Left key
};

// Helper to track key states
std::unordered_map<int, bool> keyState;

bool isKeyPressed(KeyCode code) {
    return keyState.count(code) && keyState[code];
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

    bool rWinPressed = false;
    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        auto *keystroke = reinterpret_cast<InterceptionKeyStroke *>(&stroke);

        // Log every key press or release
        bool isDown = (keystroke->state & INTERCEPTION_KEY_DOWN) != 0;
        std::cout << "Raw Key Code: " << keystroke->code
                  << ", State: " << (isDown ? "DOWN" : "UP")
                  << std::endl;

        // Suppress Right Windows key
        if (keystroke->code == RWIN) {
            rWinPressed = true;
            continue;
        }

        // Check for Right Windows + J // && isKeyPressed(RWIN) && isDown
        if (keystroke->code == J && rWinPressed) {
            std::cout << "Shortcut triggered: Right Windows + J -> Left Arrow" << std::endl;
            // Send the Left Arrow key
            sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_UP);
            continue; // Prevent passing J through to the system when shortcut is triggered
        }

        rWinPressed = false;
        // Pass through all other keys
        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);
    return 0;
}
