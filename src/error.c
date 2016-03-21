#include "errors.h"
#include <stddef.h>
#include <stdio.h>

int (*error_handler)(Display*, XErrorEvent*) = default_error_handler;

int (*XSetErrorHandler(int (*handler)(Display*, XErrorEvent*)))(Display*, XErrorEvent*) {
    // https://tronche.com/gui/x/xlib/event-handling/protocol-errors/XSetErrorHandler.html
    int (*prev_error_handler)(Display *, XErrorEvent *) = error_handler;
    if (handler == NULL) {
        handler = default_error_handler;
    }
    error_handler = handler;
    return prev_error_handler;
}

int default_error_handler(Display* display, XErrorEvent* event) {
    switch (event->error_code) {
        case BadAlloc:
            fprintf(stderr, "Out of memory: Failed to allocate memory while processing request : %d\n", event->request_code);
            break;
        case BadMatch:
            fprintf(stderr, "Parameter mismatch: A parameter was not valid for request %d!\n", event->request_code);
            break;
        case BadValue:
            fprintf(stderr, "Parameter mismatch: An int parameter was out of range for request %d!\n", event->request_code);
            break;
        case BadGC:
            fprintf(stderr, "Parameter mismatch: A parameter was not a GC for request %d!\n", event->request_code);
            break;
        case BadDrawable:
            fprintf(stderr, "Parameter mismatch: A parameter was not a Drawable for request %d!\n", event->request_code);
            break;
        case BadPixmap :
            fprintf(stderr, "Parameter mismatch: A parameter was not a Pixmap for request %d!\n", event->request_code);
            break;
        case BadWindow :
            fprintf(stderr, "Parameter mismatch: A parameter was not a Window for request %d!\n", event->request_code);
            break;
        case BadName:
            fprintf(stderr, "Parameter invalid: Got an unknown name for request %d!\n", event->request_code);
            break;
        case BadAtom:
            fprintf(stderr, "Parameter invalid: Got an invalid Atom for request %d!\n", event->request_code);
            break;
        default:
            fprintf(stderr, "An unknown error occourred: %u\n", event->error_code);
            break;
    }
    fflush(stdout);
    fflush(stderr);
    // TODO: Remove NPE
    int* t = NULL;
    *t = 1;
    exit(-1);
    return 0;
}

inline void handleOutOfMemory(int type, Display* display, unsigned long serial,
                       unsigned char request_code, unsigned char minor_code) {
    handleError(type, display, NULL, serial, BadAlloc, request_code, minor_code);
}
void handleError(int type, Display* display, XID resourceId, unsigned long serial,
                 unsigned char error_code, unsigned char request_code, unsigned char minor_code) {
    XErrorEvent event;
    event.type = type;
    event.display = display;
    event.resourceid = resourceId;
    event.serial = serial;
    event.error_code = error_code;
    event.request_code = request_code;
    event.minor_code = minor_code;
    error_handler(display, &event);
}

unsigned char resourceTypeToErrorCode(XResourceType resourceType) {
    switch (resourceType) {
        case WINDOW:
            return BadWindow;
        case DRAWABLE:
            return BadDrawable;
        case PIXMAP:
            return BadPixmap;
        default:
            return BadMatch;
    }
}
