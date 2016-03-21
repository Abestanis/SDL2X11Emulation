#include "X11/Xlib.h"
#include "SDL.h"
#include "errors.h"


void XResetScreenSaver(Display *display) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XResetScreenSaver.html
    SDL_DisableScreenSaver();
    SDL_EnableScreenSaver();
}

void XForceScreenSaver(Display *display, int mode) {
    // https://tronche.com/gui/x/xlib/window-and-session-manager/XForceScreenSaver.html
    // https://www.libsdl.org/tmp/docs-1.3/_s_d_l__video_8h.html#6e5293ce67509a49c1ead749fc4547d9
    switch (mode) {
        case ScreenSaverActive:
            // Activate the screen saver now
            // TODO: This is not possible with pure sdl, maybe implement with android?
            SDL_EnableScreenSaver();
            break;
        case ScreenSaverReset:
            XResetScreenSaver(display);
            break;
        default:
            handleError(0, display, 0, 0, BadValue, XCB_FORCE_SCREEN_SAVER, 0);
    }
}
