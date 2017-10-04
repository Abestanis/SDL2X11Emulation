#include "X11/Xlib.h"
#include "SDL.h"
#include "window.h"
#include "events.h"
#include "display.h"

unsigned int currentEventMask = ~0;
Bool mouseFrozen = False;
Bool keyboardFrozen = False;

int XWarpPointer(Display* display, Window src_window, Window dest_window, int src_x, int src_y,
                  unsigned int src_width, unsigned int src_height, int dest_x, int dest_y) {
    // https://tronche.com/gui/x/xlib/input/XWarpPointer.html
    SET_X_SERVER_REQUEST(display, X_WarpPointer);
    int curr_x, curr_y;
    if (dest_window == None) {
        #if SDL_VERSION_ATLEAST(2, 0, 4)
        SDL_GetGlobalMouseState(&curr_x, &curr_y);
        #else
        SDL_GetMouseState(&curr_x, &curr_y);
        #endif
    } else {
        if (src_window == None || !(curr_x < src_x || curr_x > src_x + src_width ||
                curr_y < src_y || curr_y > src_y + src_height)) {
            if (dest_window == SCREEN_WINDOW) {
                #if SDL_VERSION_ATLEAST(2, 0, 4)
                SDL_GetGlobalMouseState(&curr_x, &curr_y);
                #else
                SDL_GetMouseState(&curr_x, &curr_y);
                #endif
            } else {
                XTranslateCoordinates(display, SCREEN_WINDOW, dest_window, 0, 0, &curr_x, &curr_y, NULL);
            }
        }
    }
    #if SDL_VERSION_ATLEAST(2, 0, 4)
    if (SDL_WarpMouseGlobal(curr_x + dest_x, curr_y + dest_y) != 0) {
        LOG("Warning: SDL_WarpMouseGlobal failed: %s", SDL_GetError());
    }
    #else
    SDL_WarpMouseInWindow(SDL_GetMouseFocus(), curr_x + dest_x, curr_y + dest_y);
    #endif
    return 1;
}

Bool XQueryPointer(Display* display, Window window, Window* root_return, Window* child_return,
                   int* root_x_return, int* root_y_return, int* win_x_return, int* win_y_return,
                   unsigned int* mask_return) {
    // https://tronche.com/gui/x/xlib/window-information/XQueryPointer.html
    SET_X_SERVER_REQUEST(display, X_QueryPointer);
    *root_return = SCREEN_WINDOW;
    SDL_GetMouseState(root_x_return, root_y_return);
    XTranslateCoordinates(display, SCREEN_WINDOW, window, *root_x_return, *root_y_return,
                          win_x_return, win_y_return, child_return);
    *mask_return = convertModifierState(SDL_GetModState());
    return True;
}

int XGrabPointer(Display* display, Window grab_window, Bool owner_events, unsigned int event_mask,
                 int pointer_mode, int keyboard_mode, Window confine_to, Cursor cursor, Time time) {
    // https://tronche.com/gui/x/xlib/input/XGrabPointer.html
    SET_X_SERVER_REQUEST(display, X_GrabPointer);
    if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0) {
        LOG("SDL_SetRelativeMouseMode failed in XGrabPointer: %s", SDL_GetError());
        return GrabNotViewable;
    }
    if (owner_events) {
        currentEventMask = event_mask;
    }
    if (pointer_mode == GrabModeAsync) {
        mouseFrozen = False;
        // TODO: Process enqueued inputs
    } else {
        mouseFrozen = True;
    }
    if (keyboard_mode == GrabModeAsync) {
        keyboardFrozen = False;
        // TODO: Process enqueued inputs
    } else {
        keyboardFrozen = True;
    }
    // TODO: respect confine_to parameter
    if (cursor != None) {
        // TODO: When cursor is SDL_cursor
        //SDL_SetCursor(cursor);
    }
    // TODO: Generate EnterNotify and LeaveNotify events
    return 1;
}

int XUngrabPointer(Display* display, Time time) {
    // https://tronche.com/gui/x/xlib/input/XUngrabPointer.html
    SET_X_SERVER_REQUEST(display, X_UngrabPointer);
    if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
        if (SDL_SetRelativeMouseMode(SDL_FALSE) != 0) {
            LOG("SDL_SetRelativeMouseMode failed in XUngrabPointer: %s", SDL_GetError());
            return 0;
        }
        // TODO: Generate EnterNotify and LeaveNotify events
    }
    currentEventMask = ~0;
    mouseFrozen = False;
    keyboardFrozen = False;
    return 1;
}
