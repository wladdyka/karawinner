#include <iostream>

#include <interception.h>

using namespace std;

namespace scancode {
    enum {
        esc  = 0x01,
        ctrl = 0x1D,
        alt  = 0x38,
        del  = 0x53,
        j  = 36,
        l = 38,
        lwin = 91,
        rwin = 92,
        home = 71,
        end = 79,
        ralt = 56,
    };
}

void hueta(InterceptionContext context, InterceptionDevice device, short keyCode, int keyState) {
    InterceptionKeyStroke keystroke;
    keystroke.code = keyCode;
    keystroke.state = keyState;

    interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&keystroke), 1);
}

InterceptionKeyStroke lwinDown = {scancode::lwin, INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke raltDown  = {scancode::alt , INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke jDown  = {scancode::j , INTERCEPTION_KEY_DOWN, 0};
InterceptionKeyStroke lDown  = {scancode::l , INTERCEPTION_KEY_DOWN, 0};

InterceptionKeyStroke lwinUp   = {scancode::lwin, INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke raltUp    = {scancode::alt , INTERCEPTION_KEY_UP  | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke jUp    = {scancode::j , INTERCEPTION_KEY_UP , 0};
InterceptionKeyStroke lUp    = {scancode::l , INTERCEPTION_KEY_UP , 0};

bool operator==(const InterceptionKeyStroke &first,
                const InterceptionKeyStroke &second) {
    return first.code == second.code && first.state == second.state;
}

bool homeShortcut(const InterceptionKeyStroke &kstroke) {
    static int lwin = 0, ralt = 0, j = 0;

    if (lwin + ralt + j < 2) {
        if (kstroke == lwinDown) { lwin = 1; }
        if (kstroke == lwinUp) { lwin = 0; }
        if (kstroke == raltDown) { ralt = 1; }
        if (kstroke == raltUp) { ralt = 0; }
        if (kstroke == jDown) { j = 1; }
        if (kstroke == jUp) { j = 0; }
        return true;
    }

    if (lwin == 0 && (kstroke == lwinDown || kstroke == lwinUp)) {
        return false;
    }

    if (ralt == 0 && (kstroke == raltDown || kstroke == raltUp)) {
        return false;
    }

    if (j == 0 && (kstroke == jDown || kstroke == jUp)) {
        return false;
    }

    if (kstroke == lwinUp) {
        lwin = 0;
    } else if (kstroke == raltUp) {
        ralt = 0;
    } else if (kstroke == jUp) {
        j = 0;
    }

    return true;
}

bool endShortcut(const InterceptionKeyStroke &kstroke) {
    static int lwin = 0, ralt = 0, l = 0;

    if (lwin + ralt + l < 2) {
        if (kstroke == lwinDown) { lwin = 1; }
        if (kstroke == lwinUp) { lwin = 0; }
        if (kstroke == raltDown) { ralt = 1; }
        if (kstroke == raltUp) { ralt = 0; }
        if (kstroke == lDown) { l = 1; }
        if (kstroke == lUp) { l = 0; }
        return true;
    }

    if (lwin == 0 && (kstroke == lwinDown || kstroke == lwinUp)) {
        return false;
    }

    if (ralt == 0 && (kstroke == raltDown || kstroke == raltUp)) {
        return false;
    }

    if (l == 0 && (kstroke == lDown || kstroke == lUp)) {
        return false;
    }

    if (kstroke == lwinUp) {
        lwin = 0;
    } else if (kstroke == raltUp) {
        ralt = 0;
    } else if (kstroke == lUp) {
        l = 0;
    }

    return true;
}

int main() {
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionKeyStroke kstroke;

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard,
                            INTERCEPTION_FILTER_KEY_ALL);

    while (interception_receive(context, device = interception_wait(context),
                                (InterceptionStroke *)&kstroke, 1) > 0) {
        //std::cout << kstroke.code << " " << kstroke.state << std::endl;

        if (!endShortcut(kstroke)) {
            cout << "endShortcut()" << endl;
            hueta(context, device, scancode::end, INTERCEPTION_KEY_DOWN);
            hueta(context, device, scancode::end, INTERCEPTION_KEY_UP);
            continue;
        }

        if (!homeShortcut(kstroke)) {
            cout << "homeShortcut()" << endl;
            hueta(context, device, scancode::home, INTERCEPTION_KEY_DOWN);
            hueta(context, device, scancode::home, INTERCEPTION_KEY_UP);
            continue;
        }

        if (kstroke.code == scancode::alt) continue;
        if (kstroke.code == scancode::ralt) continue;
        if (kstroke.code == scancode::lwin) continue;

        interception_send(context, device, (InterceptionStroke *)&kstroke, 1);

        if (kstroke.code == scancode::esc)
            break;
    }

    interception_destroy_context(context);

    return 0;
}