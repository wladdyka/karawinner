#include <interception.h>
#include <iostream>
#include <unordered_map>

enum KeyCode {
    BACKSPACE = 14,
    CAPSLOCK = 58,
    LCTRL = 29,
    LWIN = 91,
    RWIN = 92,
    J = 36,
    HOME = 71,
    LEFT_ARROW = 75
};

// Track key states
std::unordered_map<int, short> keyState;

void sendKey(InterceptionContext context, InterceptionDevice device, KeyCode keyCode, int keyState) {
    InterceptionKeyStroke keystroke;
    keystroke.code = keyCode;
    keystroke.state = keyState;

    interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&keystroke), 1);
}

int main() {
    InterceptionContext context = interception_create_context();
    InterceptionDevice device;
    InterceptionKeyStroke kstroke;

    // Set filter to capture all keyboard events
    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    std::cout << "Karawinner started. Press LWIN + RWIN + J to send Home key." << std::endl;

    while (interception_receive(context, device = interception_wait(context),
                                reinterpret_cast<InterceptionStroke*>(&kstroke), 1) > 0) {
        // Log raw key events
        std::cout << "Raw Key Code: " << kstroke.code
                  << ", State: " << kstroke.state << std::endl;

        // Update key state
        keyState[kstroke.code] = kstroke.state;

        // Debug: Print current states of relevant keys
        std::cout << "State: LWIN = " << keyState[LWIN]
                  << ", RWIN = " << keyState[RWIN]
                  << ", J = " << keyState[J] << std::endl << "---------------------" << std::endl;

        if (kstroke.code == LWIN) {
            continue;
        }

        if (kstroke.code == RWIN) {
            continue;
        }

        // Check for LWIN + RWIN + J shortcut
        if (keyState[LWIN] == 2 && keyState[RWIN] == 2 && keyState[J] == 0) {
            std::cout << "Shortcut triggered: LWIN + RWIN + J -> Home" << std::endl;

            // Send the Home key
            sendKey(context, device, HOME, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, HOME, INTERCEPTION_KEY_UP);
            continue;
        }

        // Pass through all other keys
        interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&kstroke), 1);
    }

    interception_destroy_context(context);
    return 0;
}
