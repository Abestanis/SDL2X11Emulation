#ifndef UTIL_H
#define UTIL_H

#ifndef MAX
#  define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif /* MAX */
#ifndef MIN
#  define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

#include "X11/Xlib.h"

typedef struct {
    void** array; // The actual array.
    size_t length; // The number of entries in the array.
    size_t capacity; // The capacity of the allocated array.
} Array;

Bool initArray(Array* a, size_t initialSize);
Bool insertArray(Array* a, void* element);
void* removeArray(Array* a, size_t index, Bool preserveOrder);
ssize_t findInArray(Array *a, void* element);
void freeArray(Array* a);

#endif /* UTIL_H */
