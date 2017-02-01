#ifndef _RESOURCE_TYPES_H_
#define _RESOURCE_TYPES_H_

typedef enum {WINDOW = 1, DRAWABLE = 2, PIXMAP = 3,
    GRAPHICS_CONTEXT = 4, FONT = 5, CURSOR = 6} XResourceType;

typedef struct {
    XResourceType type;
    void* dataPointer;
} XID_Struct;

#include "X11/Xlib.h"
#include "errors.h"
#include "window.h"

#define ALLOC_XID() ((XID) malloc(sizeof(XID_Struct)))
#define FREE_XID(id) free((XID_Struct*) id)
#define SET_XID_TYPE(id, typeId) ((XID_Struct*) (id))->type = typeId
#define SET_XID_VALUE(id, value) ((XID_Struct*) (id))->dataPointer = value
#define GET_XID_TYPE(id) (((XID_Struct*) (id))->type)
#define GET_XID_VALUE(id) (((XID_Struct*) (id))->dataPointer)

#define GET_WINDOW_STRUCT(window) ((WindowStruct*) GET_XID_VALUE(window))

#define IS_TYPE(resource, typeID) (resource != None && (\
    GET_XID_TYPE(resource) == typeID || (\
        typeID == DRAWABLE && (\
            GET_XID_TYPE(resource) == PIXMAP || (\
                GET_XID_TYPE(resource) == WINDOW && !IS_INPUT_ONLY(resource)\
            )\
        )\
    )\
))


#define TYPE_CHECK(resource, typeID, display, returnCode...) \
if (!IS_TYPE(resource, typeID)) {\
    unsigned char errorCode = resourceTypeToErrorCode(typeID);\
    fprintf(stderr, "Type error: Expected '%s' to be a %s, but was %d!\n", #resource, #typeID,\
    resource != None ? GET_XID_TYPE(resource) : -1);\
    handleError(0, display, resource, 0, errorCode, 0);\
    return returnCode;\
}

#endif /* _RESOURCE_TYPES_H_ */
