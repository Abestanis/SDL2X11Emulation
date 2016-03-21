#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/keysym.h"
#include "keysymlist.h"

KeyCode XKeysymToKeycode(Display *display, KeySym keysym) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XKeysymToKeycode.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

KeySym XLookupKeysym(XKeyEvent *key_event, int index) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XLookupKeysym.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

KeySym XStringToKeysym(char* string) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XStringToKeysym.html
    if (string == NULL) { return NoSymbol; }
    if (strlen(string) == 1) {
        char chr = *string;
        if (chr >= '0' && chr <= '9' && chr >= 'a' && chr <= 'z' && chr >= 'A' && chr <= 'Z') {
            return (KeySym) ((long) chr);
        }
    }
    int i;
    for (i = 0; i < KEY_SYM_LIST_LENGTH; i++) {
        if (strcmp(KEY_SYM_LIST[i].name, string) == 0) {
            return KEY_SYM_LIST[i].keySym;
        }
    }
    return NoSymbol;
}

char* XKeysymToString(KeySym keysym) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XKeysymToString.html<<<<<<<<<<yyyyyyyy<yyyyyyyyyyyyyyyyyyyysssyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
    if (keysym >= XK_0 && keysym <= XK_9 && keysym >= XK_a && keysym <= XK_z && keysym >= XK_A && keysym <= XK_Z) {
        // TODO: Return char of keysym
    }
    int i;
    for (i = 0; i < KEY_SYM_LIST_LENGTH; i++) {
        if (KEY_SYM_LIST[i].keySym == keysym) {
            return (char*) KEY_SYM_LIST[i].name;
        }
    }
    return NULL;
}

int XLookupString(XKeyEvent *event_struct, char *buffer_return, int bytes_buffer, KeySym *keysym_return, XComposeStatus *status_in_out) {
    // https://tronche.com/gui/x/xlib/utilities/XLookupString.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

XModifierKeymap *XGetModifierMapping(Display *display) {
    // https://tronche.com/gui/x/xlib/input/XGetModifierMapping.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XFreeModifiermap(XModifierKeymap *modmap) {
    // https://tronche.com/gui/x/xlib/input/XFreeModifiermap.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

KeySym XKeycodeToKeysym(Display *display, KeyCode keycode, int index) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XKeycodeToKeysym.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XGetInputFocus(Display *display, Window *focus_return, int *revert_to_return) {
    // https://tronche.com/gui/x/xlib/input/XGetInputFocus.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XSetInputFocus(Display *display, Window focus, int revert_to, Time time) {
    // https://tronche.com/gui/x/xlib/input/XSetInputFocus.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

int XGrabKeyboard(Display *display, Window grab_window, Bool owner_events, int pointer_mode, int keyboard_mode, Time time) {
    // https://tronche.com/gui/x/xlib/input/XGrabKeyboard.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XUngrabKeyboard(Display *display, Time time) {
    // https://tronche.com/gui/x/xlib/input/XUngrabKeyboard.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XRefreshKeyboardMapping(XMappingEvent *event_map) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XRefreshKeyboardMapping.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}
