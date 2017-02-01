#include "X11/Xlib.h"
#include "SDL.h"
#include "errors.h"
#include "display.h"


int XResetScreenSaver(Display *display) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XResetScreenSaver.html
    SDL_DisableScreenSaver();
    SDL_EnableScreenSaver();
    return 1;
}

int XForceScreenSaver(Display *display, int mode) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XForceScreenSaver.html
    // https://www.libsdl.org/tmp/docs-1.3/_s_d_l__video_8h.html#6e5293ce67509a49c1ead749fc4547d9
    SET_X_SERVER_REQUEST(display, X_ForceScreenSaver);
    switch (mode) {
        case ScreenSaverActive:
            // Activate the screen saver now
            SDL_EnableScreenSaver();
            break;
        case ScreenSaverReset:
            XResetScreenSaver(display);
            break;
        default:
            handleError(0, display, None, 0, BadValue, 0);
            return 0;
    }
    return 1;
}
