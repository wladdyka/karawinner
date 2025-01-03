#include <interception.h>
#include <iostream>
#include <unordered_map>
#include <chrono>

enum KeyCode {
    BACKSPACE = 14,
    CAPSLOCK = 58,
    LCTRL = 29,
    LWIN = 91,
    RWIN = 92,
    J = 36,
    HOME = 71
};

// Track key states with timestamps
std::unordered_map<int, std::chrono::steady_clock::time_point> keyTimestamps;

void sendKey(InterceptionContext context, InterceptionDevice device, KeyCode keyCode, int keyState) {
    InterceptionKeyStroke keystroke;
    keystroke.code = keyCode;
    keystroke.state = keyState;

    interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&keystroke), 1);
}

// Check if a key was pressed recently (within thresholdMs milliseconds)
bool isKeyRecentlyPressed(int keyCode, int thresholdMs) {
    auto now = std::chrono::steady_clock::now();
    if (keyTimestamps.count(keyCode)) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - keyTimestamps[keyCode]).count();
        return elapsed < thresholdMs;
    }
    return false;
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

        // Update key timestamps
        if (kstroke.state == 2) { // Key Down
            keyTimestamps[kstroke.code] = std::chrono::steady_clock::now();
        } else if (kstroke.state == 3) { // Key Up
            keyTimestamps.erase(kstroke.code); // Remove key from active state
        }

        // Debug: Print states of relevant keys
        std::cout << "Key Timestamps: "
                  << "LWIN = " << isKeyRecentlyPressed(LWIN, 100) << ", "
                  << "RWIN = " << isKeyRecentlyPressed(RWIN, 100) << ", "
                  << "J = " << isKeyRecentlyPressed(J, 100) << std::endl;

        // Check for LWIN + RWIN + J shortcut
        if (isKeyRecentlyPressed(LWIN, 100) &&
            isKeyRecentlyPressed(RWIN, 100) &&
            isKeyRecentlyPressed(J, 100)) {
            std::cout << "Shortcut triggered: LWIN + RWIN + J -> Home" << std::endl;

            // Send the Home key
            sendKey(context, device, HOME, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, HOME, INTERCEPTION_KEY_UP);

            // Clear timestamps to prevent repeated triggers
            keyTimestamps.clear();
            continue;
        }

        // Pass through all other keys
        interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&kstroke), 1);
    }

    interception_destroy_context(context);
    return 0;
}
