#ifndef UTIL_H
#define UTIL_H

#ifndef MAX
#  define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif /* MAX */
#ifndef MIN
#  define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

#if defined(__cplusplus) || defined(c_plusplus)
#  define CLASS_ATTRIBUTE c_class
#else
#  define CLASS_ATTRIBUTE class
#endif

#define TO_STRING_HELPER(x) #x
#define TO_STRING(x) TO_STRING_HELPER(x)

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
