#include <iostream>
#include <interception.h>

using namespace std;

namespace scancode {
    enum {
        esc  = 0x01,
        ctrl = 0x1D,
        alt  = 0x38,
        del  = 0x53,
        j    = 36,
        l    = 38,
        lwin = 91,
        rwin = 92,
        home = 71,
        end  = 79,
        ralt = 56,
    };
}

void sendKey(InterceptionContext context, InterceptionDevice device, short keyCode, int keyState) {
    InterceptionKeyStroke keystroke;
    keystroke.code = keyCode;
    keystroke.state = keyState;

    interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&keystroke), 1);
}

InterceptionKeyStroke lwinDown = {scancode::lwin, INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke raltDown = {scancode::alt , INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke jDown    = {scancode::j   , INTERCEPTION_KEY_DOWN, 0};
InterceptionKeyStroke lDown    = {scancode::l   , INTERCEPTION_KEY_DOWN, 0};

InterceptionKeyStroke lwinUp   = {scancode::lwin, INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke raltUp   = {scancode::alt , INTERCEPTION_KEY_UP  | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke jUp      = {scancode::j   , INTERCEPTION_KEY_UP , 0};
InterceptionKeyStroke lUp      = {scancode::l   , INTERCEPTION_KEY_UP , 0};

bool operator==(const InterceptionKeyStroke &first,
                const InterceptionKeyStroke &second) {
    return first.code == second.code && first.state == second.state;
}

bool homeShortcut(const InterceptionKeyStroke &kstroke) {
    static int lwin = 0, ralt = 0, j = 0;
    static bool shortcutActive = false;

    // Track individual key states
    if (kstroke == lwinDown) { lwin = 1; }
    if (kstroke == lwinUp) { lwin = 0; }
    if (kstroke == raltDown) { ralt = 1; }
    if (kstroke == raltUp) { ralt = 0; }
    if (kstroke == jDown) { j = 1; }
    if (kstroke == jUp) { j = 0; }

    // Trigger shortcut only if all keys are pressed and it's not already active
    if (lwin && ralt && j && !shortcutActive) {
        shortcutActive = true; // Prevent retriggering
        return false;          // Shortcut triggered
    }

    // Reset shortcutActive when all keys are released
    if (!lwin && !ralt && !j) {
        shortcutActive = false;
    }

    return true; // Pass through other key events
}

bool endShortcut(const InterceptionKeyStroke &kstroke) {
    static int lwin = 0, ralt = 0, l = 0;
    static bool shortcutActive = false;

    if (kstroke == lwinDown) { lwin = 1; }
    if (kstroke == lwinUp) { lwin = 0; }
    if (kstroke == raltDown) { ralt = 1; }
    if (kstroke == raltUp) { ralt = 0; }
    if (kstroke == lDown) { l = 1; }
    if (kstroke == lUp) { l = 0; }

    if (lwin && ralt && l && !shortcutActive) {
        shortcutActive = true;
        return false;
    }

    if (!lwin && !ralt && !l) {
        shortcutActive = false;
    }

    return true;
}

int main() {
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionKeyStroke kstroke;

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    while (interception_receive(context, device = interception_wait(context),
                                (InterceptionStroke *)&kstroke, 1) > 0) {
        if (!homeShortcut(kstroke)) {
            cout << "homeShortcut()" << endl;
            sendKey(context, device, scancode::home, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, scancode::home, INTERCEPTION_KEY_UP);
            continue;
        }

        if (!endShortcut(kstroke))
        {
            cout << "endShortcut()" << endl;
            sendKey(context, device, scancode::end, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, scancode::end, INTERCEPTION_KEY_UP);
            continue;
        }

        if (kstroke.code == scancode::alt || kstroke.code == scancode::ralt || kstroke.code == scancode::lwin) {
            continue;
        }

        interception_send(context, device, (InterceptionStroke *)&kstroke, 1);

        if (kstroke.code == scancode::esc)
            break;
    }

    interception_destroy_context(context);

    return 0;
}
