#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "display.h"
#include "util.h"

Bool initArray(Array* a, size_t initialSize) {
    if (initialSize != 0) {
        a->array = malloc(initialSize * sizeof(void *));
        if (a->array == NULL) return False;
    } else {
        a->array = NULL;
    }
    a->length = 0;
    a->capacity = initialSize;
    return True;
}

Bool insertArray(Array* a, void* element) {
    if (a->length == a->capacity) {
        size_t newCapacity = (size_t) MAX(8, a->capacity * 1.5);
        void* temp = realloc(a->array, newCapacity * sizeof(void *));
        if (temp == NULL) return False;
        a->array = temp;
        a->capacity = newCapacity;
    }
    a->array[a->length++] = element;
    return True;
}

void* removeArray(Array* a, size_t index, Bool preserveOrder) {
    if (index >= a->length) abort();
    void* element = a->array[index];
    if (index - 1 != a->length) {
        if (preserveOrder) {
            memmove(&a->array[index], &a->array[index + 1], sizeof(void *) * (a->length - (index + 1)));
        } else {
            a->array[index] = a->array[a->length];
        }
    }
    a->length--;
    return element;
}

ssize_t findInArray(Array *a, void* element) {
    ssize_t i;
    for (i = 0; i < a->length; i++) {
        if (a->array[i] == element) return i;
    }
    return -1;
}

void freeArray(Array* a) {
    free(a->array);
    a->array = NULL;
    a->length = a->capacity = 0;
}


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
    SET_X_SERVER_REQUEST(display, XCB_GET_GEOMETRY);
    fprintf(stderr, "Hit unimplemented function %s.\n", __func__);
}

void XCopyGC(Display *display, GC src, GC dest, unsigned long valuemask) {
    // https://tronche.com/gui/x/xlib/GC/XCopyGC.html
    SET_X_SERVER_REQUEST(display, XCB_COPY_GC);
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
