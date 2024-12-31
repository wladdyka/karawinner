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

    // Cast InterceptionKeyStroke to InterceptionStroke
    interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&keystroke), 1);
}


int main() {
    // Initialize Interception
    InterceptionContext context = interception_create_context();
    InterceptionDevice device;
    InterceptionKeyStroke kstroke;

    // Filter for key down and up events
    interception_set_filter(context, interception_is_keyboard,
                            INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);

    std::cout << "Karawinner started. Press Right Windows + J to send Left Arrow key." << std::endl;

    while (interception_receive(context, device = interception_wait(context),
                                (InterceptionStroke*)&kstroke, 1) > 0) {
        // Determine key state (down or up)
        bool isDown = !(kstroke.state & INTERCEPTION_KEY_UP);
        std::cout << "Raw Key Code: " << kstroke.code
                  << ", State: " << (isDown ? "DOWN" : "UP")
                  << std::endl;

        // Track key states
        keyState[kstroke.code] = isDown;

        // Suppress Right Windows key
        if (kstroke.code == RWIN) {
            std::cout << "Right Windows key " << (isDown ? "pressed" : "released") << " (suppressed)" << std::endl;
            continue; // Do not pass Right Windows key to the system
        }

        // Check for Right Windows + J
        if (kstroke.code == J && isKeyPressed(RWIN) && isDown) {
            std::cout << "Shortcut triggered: Right Windows + J -> Left Arrow" << std::endl;

            // Send the Left Arrow key
            sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_UP);
            continue; // Do not pass J key to the system when shortcut is triggered
        }

        // Pass through all other keys
        interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&kstroke), 1);
    }

    interception_destroy_context(context);
    return 0;
}
