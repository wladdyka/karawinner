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
    L = 38,
    HOME = 71,
    LEFT_ARROW = 75,
    RIGHT_ARROW = 77
};

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


    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    std::cout << "Karawinner started. Press Right Windows + J to send Left Arrow key." << std::endl;

    while (interception_receive(context, device = interception_wait(context),
                                (InterceptionStroke*)&kstroke, 1) > 0) {
        std::cout << "Raw Key Code: " << kstroke.code
                  << ", State: " << kstroke.state
                  << std::endl;

        keyState[kstroke.code] = kstroke.state;

        if (kstroke.code == CAPSLOCK) {
            sendKey(context, device, BACKSPACE, kstroke.state);
            continue;
        }

        if (kstroke.code == LWIN) {
            std::cout << "Left Windows key " << (kstroke.state == 2 ? "pressed" : "released") << " (suppressed)" << std::endl;
            sendKey(context, device, LCTRL, kstroke.state - 2);
            continue;
        }

        if (kstroke.code == RWIN) {
            std::cout << "Right Windows key " << (kstroke.state == 2 ? "pressed" : "released") << " (suppressed)" << std::endl;
            continue; // Do not pass Right Windows key to the system
        }

        bool leftArrowShortcut = kstroke.code == J && keyState[RWIN] == 2 && kstroke.state == 0;
        if (leftArrowShortcut && keyState[LCTRL] == 0) {
            std::cout << "Shortcut triggered: Right Windows + Left Windows J -> Home" << std::endl;

            sendKey(context, device, HOME, kstroke.state);
            continue;
        } else if (leftArrowShortcut) {
            std::cout << "Shortcut triggered: Right Windows + J -> Left Arrow" << std::endl;

            sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, LEFT_ARROW, INTERCEPTION_KEY_UP);
            continue;
        }

        bool rightArrowShortcut = kstroke.code == L && keyState[RWIN] == 2 && kstroke.state == 0;
        if (rightArrowShortcut) {
            std::cout << "Shortcut triggered: Right Windows + L -> Right Arrow" << std::endl;

            sendKey(context, device, RIGHT_ARROW, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, RIGHT_ARROW, INTERCEPTION_KEY_UP);
            continue;
        }

        // Pass through all other keys
        interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&kstroke), 1);
    }

    interception_destroy_context(context);
    return 0;
}

