#include <interception.h>
#include <iostream>
#include <unordered_map>

// Enum for keycodes
enum KeyCode {
    LWIN = 91,    // Left Windows key
    RWIN = 92,    // Right Windows key (optional, use if needed)
    J = 36,       // J key
    HOME = 71,    // Home key
    LEFT_ARROW = 75 // Arrow Left key
};

// Helper to track key states
std::unordered_map<int, short> keyState;

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
    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    std::cout << "Karawinner started. Press Right Windows + J to send Left Arrow key." << std::endl;

    while (interception_receive(context, device = interception_wait(context),
                                (InterceptionStroke*)&kstroke, 1) > 0) {
        // Log raw key press or release
        std::cout << "Raw Key Code: " << kstroke.code
                  << ", State: " << kstroke.state
                  << std::endl;

        // Track key states
        keyState[kstroke.code] = kstroke.state;

        // Permanent remap: LWIN (92) to HOME (75)
        if (kstroke.code == LWIN) {
            std::cout << "LWIN key remapped to HOME key" << std::endl;

            // Send the HOME key instead of LWIN
            sendKey(context, device, HOME, kstroke.state);
            continue; // Do not pass the original LWIN key event
        }

        // Suppress Right Windows key
        if (kstroke.code == RWIN) {
            std::cout << "Right Windows key " << (kstroke.state == 3 ? "pressed" : "released") << " (suppressed)" << std::endl;
            continue; // Do not pass Right Windows key to the system
        }

        // Shortcut: Right Windows + J -> Left Arrow
        if (kstroke.code == J && isKeyPressed(RWIN) && kstroke.state == 0) {
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

