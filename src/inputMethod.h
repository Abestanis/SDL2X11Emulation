#ifndef INPUTMETHOD_H
#define INPUTMETHOD_H

#include "X11/Xlib.h"
#include "window.h"

typedef void* XrmDatabase; // Unused
typedef Display _XIM;
typedef struct {
    Display* display;
    XIMStyle style;
    Window focus;
    SDL_Rect* inputRect;
    unsigned long eventFilter;
} _XIC;
#define GET_XIC_STRUCT(inputConnection) ((_XIC*) inputConnection)

#define XIMUndefined 0x0000L
#define defaultLocaleModifierList "DEFAULT"
static const XIMStyle SUPPORTED_STYLES[] = {
    XIMPreeditArea | XIMStatusNothing, XIMPreeditArea | XIMStatusNone,
    XIMPreeditPosition | XIMStatusNothing, XIMPreeditPosition | XIMStatusNone
};
static const XIMStyles supportedStyles = {
    sizeof(SUPPORTED_STYLES) / sizeof(SUPPORTED_STYLES[0]),
    (XIMStyle*) &SUPPORTED_STYLES[0],
};

void inputMethodSetCurrentText(char* text);

#endif /* INPUTMETHOD_H */
