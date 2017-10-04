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

Bool equalCmp(void* element, void* arg) {
    return element == arg;
}

ssize_t findInArray(Array *a, void* element) {
    return findInArrayNCmp(a, element, 0, &equalCmp);
}

ssize_t findInArrayN(Array *a, void* element, size_t startIndex) {
    return findInArrayNCmp(a, element, startIndex, &equalCmp);
}

ssize_t findInArrayCmp(Array *a, void *element, Bool (*cmpFunc)(void *, void *)) {
    return findInArrayNCmp(a, element, 0, cmpFunc);
}

ssize_t findInArrayNCmp(Array *a, void *element, size_t startIndex,
                        Bool (*cmpFunc)(void *, void *)) {
    ssize_t i;
    for (i = startIndex; i < a->length; i++) {
        if (cmpFunc(a->array[i], element)) return i;
    }
    return -1;
}

void swapArray(Array *a, size_t index1, size_t index2) {
    if (index1 >= a->length || index2 >= a->length) abort();
    void* tmp = a->array[index1];
    a->array[index1] = a->array[index2];
    a->array[index2] = tmp;
}

void freeArray(Array* a) {
    free(a->array);
    a->array = NULL;
    a->length = a->capacity = 0;
}

Bool matchWildcard(const char* wildcard, const char* string) {
    if (wildcard == NULL || string == NULL) return False;
    ssize_t lastStar = -1;
    size_t stringIndex = 0;
    size_t wildcardIndex = 0;
    int numCharsNeeded;
    for (; wildcard[wildcardIndex] != '\0'; wildcardIndex++) {
        switch (wildcard[wildcardIndex]) {
            case '*': // TODO: Tis fails for "*ab??" -> "bla-abab--", don't reinvent the wheel and use some code from SO.
                // Find next character to match
                lastStar = wildcardIndex;
                numCharsNeeded = 0;
                while (True) {
                    wildcardIndex++;
                    if (wildcard[wildcardIndex] == '?') {
                        numCharsNeeded++;
                    } else if (wildcard[wildcardIndex] == '\0') {
                        // We have a wildcard with a '*' at the end
                        return strlen(&string[stringIndex]) >= numCharsNeeded;
                    } else if (wildcard[wildcardIndex] == '*') {
                        continue;
                    } else {
                        break;
                    }
                }
                while (numCharsNeeded-- > 0 || wildcard[wildcardIndex] != string[stringIndex]) {
                    if (string[stringIndex] == '\0') return False;
                    stringIndex++;
                }
                break;
            case '?':
                if (string[stringIndex] == '\0') {
                    return False;
                }
                break;
            default:
                if (wildcard[wildcardIndex] != string[stringIndex]) {
                    if (string[stringIndex] != '\0' && lastStar != -1) {
                        wildcardIndex = (size_t) lastStar;
                    } else {
                        return False;
                    }
                }
        }
        stringIndex++;
    }
    return True;
}


int XFree(void *data) {
    // https://tronche.com/gui/x/xlib/display/XFree.html
    WARN_UNIMPLEMENTED;
    return 1;
}

Status XGetGeometry(Display *display, Drawable d, Window *root_return, int *x_return, int* y_return, unsigned int *width_return, unsigned int *height_return, unsigned int *border_width_return, unsigned int *depth_return) {
    // https://tronche.com/gui/x/xlib/window-information/XGetGeometry.html
    SET_X_SERVER_REQUEST(display, X_GetGeometry);
    WARN_UNIMPLEMENTED;
    return 0;
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
