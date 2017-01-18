#ifndef _RESOURCE_TYPES_H_
#define _RESOURCE_TYPES_H_

typedef enum {WINDOW = 1, DRAWABLE = 2, PIXMAP = 3, GRAPHICS_CONTEXT = 4} XResourceType;

struct XID_Struct {
    XResourceType type;
    void* dataPointer;
};

#include "X11/Xlib.h"
#include "errors.h"
#include "window.h"

#define GET_WINDOW_STRUCT(window) ((WindowStruct*) (window)->dataPointer)

#define IS_TYPE(resource, typeID) (resource != NULL && (\
    resource->type == typeID || (\
        typeID == DRAWABLE && (\
            resource->type == PIXMAP || (\
                resource->type == WINDOW && !IS_INPUT_ONLY(resource)\
            )\
        )\
    )\
))


#define TYPE_CHECK(resource, typeID, display, returnCode...) \
if (!IS_TYPE(resource, typeID)) {\
    unsigned char errorCode = resourceTypeToErrorCode(typeID);\
    fprintf(stderr, "Type error: Expected '%s' to be a %s, but was %d!\n", #resource, #typeID,\
    resource != NULL && resource != None ? resource->type : -1);\
    handleError(0, display, resource, 0, errorCode, 0);\
    return returnCode;\
}

#endif /* _RESOURCE_TYPES_H_ */
