#include "X11/Xlib.h"
#include "X11/Xutil.h"

void XFree(void *data) {
    // https://tronche.com/gui/x/xlib/display/XFree.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

VisualID XVisualIDFromVisual(Visual* visual) {
    // https://tronche.com/gui/x/xlib/window/XVisualIDFromVisual.html
    return visual->visualid;
}

Status XGetGeometry(Display *display, Drawable d, Window *root_return, int *x_return, int* y_return, unsigned int *width_return, unsigned int *height_return, unsigned int *border_width_return, unsigned int *depth_return) {
    // https://tronche.com/gui/x/xlib/window-information/XGetGeometry.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XCopyGC(Display *display, GC src, GC dest, unsigned long valuemask) {
    // https://tronche.com/gui/x/xlib/GC/XCopyGC.html
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

XSizeHints* XAllocSizeHints() {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XAllocSizeHints.html
    XSizeHints* sizeHints = malloc(sizeof(XSizeHints));
    if (sizeHints != NULL) {
        memset(sizeHints, 0, sizeof(XSizeHints));
    }
    return sizeHints;
}

XClassHint* XAllocClassHint() {
    // https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/XAllocClassHint.html
    XClassHint* classHint = malloc(sizeof(XClassHint));
    if (classHint != NULL) {
        classHint->res_name  = NULL;
        classHint->res_class = NULL;
    }
    return classHint;
}
