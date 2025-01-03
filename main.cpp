#include <iostream>

#include <interception.h>

using namespace std;

namespace scancode {
    enum {
        esc  = 0x01,
        ctrl = 0x1D,
        //alt  = 0x38,
        j = 36,
        lwin = 91,
        rwin = 92,
        home = 71,
    };
}

InterceptionKeyStroke rWinDown = {scancode::rwin, INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke jDown  = {scancode::j , INTERCEPTION_KEY_DOWN, 0};
InterceptionKeyStroke lWinDown  = {scancode::lwin , INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0, 0};

InterceptionKeyStroke rWinUp   = {scancode::rwin, INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke jUp    = {scancode::j , INTERCEPTION_KEY_UP, 0};
InterceptionKeyStroke lWinUp    = {scancode::lwin , INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0, 0};

bool operator==(const InterceptionKeyStroke &first,
                const InterceptionKeyStroke &second) {
    return first.code == second.code && first.state == second.state;
}

bool shall_produce_keystroke(const InterceptionKeyStroke &kstroke) {
    static int rWinIsDown = 0, jIsDown = 0, lWinIsDown = 0;

    //cout << "sum:" << ctrl_is_down + rwin_is_down + lwin_is_down << endl;
    if (rWinIsDown + jIsDown + lWinIsDown < 2) {
        if (kstroke == rWinDown) { rWinIsDown = 1; }
        if (kstroke == rWinUp  ) { rWinIsDown = 0; }
        if (kstroke == jDown ) { jIsDown = 1;  }
        if (kstroke == jUp   ) { jIsDown = 0;  }
        if (kstroke == lWinDown ) { lWinIsDown = 1;  }
        if (kstroke == lWinUp   ) { lWinIsDown = 0;  }
        return true;
    }

    if (rWinIsDown == 0 && (kstroke == rWinDown || kstroke == rWinUp)) {
        return false;
    }

    if (jIsDown == 0 && (kstroke == jDown || kstroke == jUp)) {
        return false;
    }

    if (lWinIsDown == 0 && (kstroke == lWinDown || kstroke == lWinUp)) {
        return false;
    }

    if (kstroke == rWinUp) {
        rWinIsDown = 0;
    } else if (kstroke == jUp) {
        jIsDown = 0;
    } else if (kstroke == lWinUp) {
        lWinIsDown = 0;
    }

    return true;
}

void sendKey(InterceptionContext context, InterceptionDevice device, short keyCode, int keyState) {
    InterceptionKeyStroke keystroke;
    keystroke.code = keyCode;
    keystroke.state = keyState;

    interception_send(context, device, reinterpret_cast<const InterceptionStroke*>(&keystroke), 1);
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
        //cout << kstroke.code << " " << kstroke.state << endl;

        if (!shall_produce_keystroke(kstroke)) {
            cout << "home shorcut triggered" << endl;
            sendKey(context, device, scancode::home, INTERCEPTION_KEY_DOWN);
            sendKey(context, device, scancode::home, INTERCEPTION_KEY_UP);
            continue;
        }

        if (kstroke.code == scancode::lwin)
        {
            continue;
        }

        if (kstroke.code == scancode::rwin)
        {
            continue;
        }

        interception_send(context, device, (InterceptionStroke *)&kstroke, 1);

        if (kstroke.code == scancode::esc)
            break;
    }

    interception_destroy_context(context);

    return 0;
}