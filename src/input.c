#include "input.h"
#include "X11/Xutil.h"
#include "X11/keysym.h"
#include "keysymlist.h"
#include "errors.h"
#include "display.h"

Window keyboardFocus = None;
int revertTo = RevertToParent;
        
Window getKeyboardFocus() {
    fprintf(stderr, "keyboard focus is %lu\n", keyboardFocus);
    return keyboardFocus;
}

int XSelectInput(Display* display, Window window, long event_mask) {
    // https://tronche.com/gui/x/xlib/event-handling/XSelectInput.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    fprintf(stderr, "%s: %ld, %ld\n", __func__, event_mask & KeyPressMask, event_mask & KeyReleaseMask);
    if (event_mask & KeyPressMask || event_mask & KeyReleaseMask) {
        // TODO: Implement real system here
        if (!SDL_IsTextInputActive()) {
            SDL_StartTextInput();
        }
        keyboardFocus = window;
    }
    return 1;
}

KeyCode XKeysymToKeycode(Display *display, KeySym keysym) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XKeysymToKeycode.html
//    SET_X_SERVER_REQUEST(display, XCB_);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

KeySym XLookupKeysym(XKeyEvent *key_event, int index) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XLookupKeysym.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

KeySym XStringToKeysym(_Xconst char* string) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XStringToKeysym.html
    if (string == NULL) { return NoSymbol; }
    if (strlen(string) == 1) {
        char chr = string[0];
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
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XKeysymToString.html
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

KeySym XKeycodeToKeysym(Display *display, KeyCode keycode, int index) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XKeycodeToKeysym.html
//    SET_X_SERVER_REQUEST(display, XCB_);
    if (keycode >= SDLK_0 && keycode <= SDLK_9) { // 0 - 9
        return XK_0 + (keycode - SDLK_0);
    } else if (keycode >= SDLK_z && keycode <= SDLK_z) { // A - Z
        return XK_A + (keycode - SDLK_a);
    }
    int i;
    for (i = 0; i < SDL_KEYCODE_TO_KEYSYM_LENGTH; i++) {
        if (SDLKeycodeToKeySym[i].keycode == keycode) {
            return SDLKeycodeToKeySym[i].keysym;
        }
    }
    fprintf(stderr, "%s: Got unimplemented keycode %c\n", __func__, keycode);
    return NoSymbol;
}

int XLookupString(XKeyEvent* event_struct, char* buffer_return, int bytes_buffer,
                  KeySym* keysym_return, XComposeStatus *status_in_out) {
    // https://tronche.com/gui/x/xlib/utilities/XLookupString.html
    *buffer_return = event_struct->keycode;
    *keysym_return = XKeycodeToKeysym(event_struct->display, event_struct->keycode, 0);
    return 1;
}

XModifierKeymap* XGetModifierMapping(Display* display) {
    // https://tronche.com/gui/x/xlib/input/XGetModifierMapping.html
    SET_X_SERVER_REQUEST(display, X_GetModifierMapping);
    static KeyCode MODIFIER_KEYS[] = {
        KMOD_SHIFT,
        KMOD_CTRL,
        KMOD_CAPS, // FIXME: This is bugged! 
    };
    static int NUM_MODIFIER_KEYS = sizeof(MODIFIER_KEYS) / sizeof(MODIFIER_KEYS[0]);
    XModifierKeymap* modifierKeymap = malloc(sizeof(XModifierKeymap));
    if (modifierKeymap == NULL) {
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    modifierKeymap->max_keypermod = NUM_MODIFIER_KEYS;
    modifierKeymap->modifiermap = malloc(sizeof(KeyCode) * NUM_MODIFIER_KEYS);
    if (modifierKeymap->modifiermap == NULL) {
        free(modifierKeymap);
        handleOutOfMemory(0, display, 0, 0);
        return NULL;
    }
    memcpy(modifierKeymap->modifiermap, &MODIFIER_KEYS, sizeof(KeyCode) * NUM_MODIFIER_KEYS);
    return modifierKeymap;
}

int XFreeModifiermap(XModifierKeymap* modmap) {
    // https://tronche.com/gui/x/xlib/input/XFreeModifiermap.html
    free(modmap->modifiermap);
    free(modmap);
    return 1;
}

int XGetInputFocus(Display *display, Window *focus_return, int *revert_to_return) {
    // https://tronche.com/gui/x/xlib/input/XGetInputFocus.html
    SET_X_SERVER_REQUEST(display, X_GetInputFocus);
    *focus_return = getKeyboardFocus();
    if (*focus_return == None) *focus_return = (Window) PointerRoot;
    *revert_to_return = revertTo;
    return 1;
}

int XSetInputFocus(Display *display, Window focus, int revert_to, Time time) {
    // https://tronche.com/gui/x/xlib/input/XSetInputFocus.html
    SET_X_SERVER_REQUEST(display, X_SetInputFocus);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    revertTo = revert_to;
    return 1;
}

int XGrabKeyboard(Display *display, Window grab_window, Bool owner_events, int pointer_mode, int keyboard_mode, Time time) {
    // https://tronche.com/gui/x/xlib/input/XGrabKeyboard.html
    SET_X_SERVER_REQUEST(display, X_GrabKeyboard);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XUngrabKeyboard(Display *display, Time time) {
    // https://tronche.com/gui/x/xlib/input/XUngrabKeyboard.html
    SET_X_SERVER_REQUEST(display, X_UngrabKeyboard);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}

int XRefreshKeyboardMapping(XMappingEvent *event_map) {
    // https://tronche.com/gui/x/xlib/utilities/keyboard/XRefreshKeyboardMapping.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
    return 1;
}
